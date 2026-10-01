# Solveur A* — feuille de route

Objectif : résoudre le cube par recherche heuristique, du plus simple (A* sans heuristique) au plus fort (IDA* avec bases de motifs, ou heuristique apprise), en mesurant à chaque étape ce que l'on gagne.

Référence de l'API : [API_PYTHON.md](API_PYTHON.md).

---

## Principe de découpage Python / C++

- **Python** : les algorithmes pendant qu'ils évoluent, les mesures, les courbes, l'entraînement éventuel d'un réseau, et le rejeu des solutions dans l'IHM.
- **C++ (`rubikcore`)** : les briques réutilisées par tous les algorithmes et appelées des millions de fois (heuristiques par lots, bases de motifs). La boucle de recherche complète n'y passe qu'une fois l'algorithme stabilisé, si l'on vise l'optimal sur des mélanges profonds.
- **Règle** : écrire d'abord en Python, mesurer, et ne passer en C++ que ce que la mesure désigne.

## Repères chiffrés

- **Coups** : les 12 quarts de tour (`rc.moves()`), tous de coût 1.
- **Distance maximale** : 26 quarts de tour suffisent pour tout état (« nombre de Dieu » en quarts de tour). Un état aléatoire est typiquement autour de 20.
- **États à exactement d coups** : 1, 12, 114, 1 068, 10 011, 93 840, 878 880, 8 221 632 (d = 7), environ 77 millions (d = 8)… Une recherche sans heuristique plafonne donc vers d = 7 ou 8.
- **Vitesse** : environ 10⁵ nœuds par seconde en Python, 10⁷ à 10⁸ en C++.
- **Élagage des coups inutiles** (facteur de branchement effectif d'environ 9,4) :
  - ne jamais jouer l'inverse du coup précédent ;
  - pour deux faces opposées, qui commutent, n'autoriser qu'un seul ordre (par exemple `L R`, jamais `R L`) ;
  - ne jamais jouer trois fois de suite le même coup (`R R R` vaut `R'`).

---

## Étape 0 — Banc d'essai

Avant tout algorithme, préparer de quoi les comparer.

- **Jeux de mélanges fixes** : par exemple 50 états par profondeur de mélange d = 1 … 20, générés avec `rc.generate(n, d, k_min=d, seed=…)` et sauvegardés pour être réutilisés à l'identique.
- **Mesures par résolution** : nœuds développés, nœuds générés, temps, mémoire maximale (taille de la file et de l'ensemble des visités), longueur de la solution, et vérification que `rc.is_solved(rc.apply(etat, solution))`.
- **Sorties** : un tableau ou une courbe par algorithme (temps et nœuds en fonction de la profondeur, en échelle log).
- **Visualisation** : rejouer une solution dans la fenêtre de script de l'IHM avec `rubik.set_state(etat)` puis `rubik.exec(solution)`.
- **Point de comparaison** : le paquet `kociemba` (`pip install kociemba`) donne vite une solution d'environ 20 coups, non optimale.

**Fini quand** : on peut lancer `bench(algo)` sur tous les jeux et obtenir les courbes.

## Étape 1 — A* sans heuristique (h = 0)

C'est en fait Dijkstra, ou un parcours en largeur puisque tous les coûts valent 1. Le but est de valider la mécanique.

- File de priorité (`heapq`), ensemble des visités (`etat.tobytes()`), développement avec `rc.children`, élagage des coups inutiles.
- Garder le chemin parcouru (parent et coup) pour reconstruire la solution.
- **Contrôles** :
  - le nombre d'états trouvés à chaque profondeur ≤ 5 doit correspondre exactement à 12, 114, 1 068, 10 011, 93 840 ;
  - les solutions sont optimales, donc jamais plus longues que le mélange.
- **Attendu** : ça fonctionne jusqu'à environ d = 6 ou 7, puis la mémoire et le temps explosent. C'est la référence « sans heuristique ».

**Fini quand** : les contrôles passent et la courbe de l'étape 0 montre où ça décroche.

## Étape 2 — Heuristique admissible : distance de Manhattan des cubies

Pour chaque coin, h compte le nombre minimal de quarts de tour qui le ramènent à sa place avec la bonne orientation. Même chose pour les arêtes. Chaque coup déplaçant 4 coins et 4 arêtes, `h = max(Σcoins / 4, Σarêtes / 4)` est admissible.

- **Ajout à l'API** : savoir quels autocollants forment chaque cubie, soit 8 coins de 3 autocollants et 12 arêtes de 2. Soit `rubikcore` l'expose, soit on code ces groupes en dur (ce sont les tables classiques du format Kociemba).
- **Tables précalculées** : pour un coin seul, il y a 8 positions × 3 orientations = 24 cas. Un petit parcours en largeur donne la distance de chaque cas. Même chose pour les arêtes : 12 × 2 = 24 cas.
- **Version Python** d'abord (vectorisée avec numpy sur un lot d'états), puis **`rc.manhattan(states)` en C++** si la mesure le justifie.
- **Contrôle d'admissibilité** : sur des états dont on connaît la distance exacte (grâce à l'étape 1, d ≤ 7), vérifier que h(s) ≤ distance. C'est à automatiser.
- **Attendu** : des solutions optimales jusqu'à environ 10 à 12 coups.

**Fini quand** : le contrôle d'admissibilité passe et la courbe montre le gain par rapport à l'étape 1.

## Étape 3 — A* pondéré

On utilise `f = g + w·h` avec w > 1 : la recherche est beaucoup plus rapide, mais la solution n'est plus garantie optimale.

- Essayer w ∈ {1 ; 1,5 ; 2 ; 3 ; 5}.
- Tracer le temps et les nœuds en fonction de la longueur de solution obtenue, comparée à la longueur optimale : c'est le compromis vitesse / optimalité.
- **Attendu** : des mélanges nettement plus profonds deviennent accessibles, avec des solutions un peu plus longues que l'optimal.

**Fini quand** : on a les courbes de compromis et une valeur de w « raisonnable ».

## Étape 4 (optionnelle) — Variantes de la recherche

- **Développement par lots** : sortir N nœuds de la file, un seul appel à `rc.children` et un seul calcul vectorisé de h. Ça réduit le surcoût de Python, et c'est indispensable avec une heuristique apprise.
- **Recherche bidirectionnelle** : on cherche depuis le mélange et depuis le cube résolu, et l'on s'arrête quand les deux fronts se rencontrent. Ça ramène la profondeur de d à d/2 de chaque côté.
- **IDA*** en Python avec l'heuristique de Manhattan : quasiment pas de mémoire, mais plus lent. C'est une bonne préparation à l'étape 5a.

---

## Étape 5 — Aller plus loin (au choix)

### 5a — IDA* et bases de motifs (Korf), côté algorithmes

Objectif : des solutions **optimales** pour n'importe quel état.

1. **Coder les cubies en entiers** :
   - permutation des coins, de rang 0 à 8! − 1 ;
   - orientation des coins, de 0 à 3⁷ − 1 ;
   - permutation et orientation de sous-ensembles d'arêtes.
2. **Construire les bases par parcours en largeur, en C++** :
   - une base de coins : 8! × 3⁷ ≈ 88 millions d'entrées, environ 44 Mo à 4 bits par entrée ;
   - deux bases de 6 ou 7 arêtes.
   - Les enregistrer dans des fichiers binaires, à ignorer dans git.
3. **Heuristique** : le maximum des valeurs lues dans les bases.
4. **IDA* en C++**, exposé dans `rubikcore`, par exemple `rc.solve_ida(state)`.
5. **Contrôles** :
   - les solutions ont la même longueur que celles de l'étape 2 sur les mélanges peu profonds ;
   - elles sont toujours de 26 coups au plus.

### 5b — Heuristique apprise (DeepCubeA), côté machine learning

Objectif : estimer h avec un réseau et résoudre des mélanges profonds avec un A* pondéré par lots.

1. Données : `rc.generate`.
2. Cible de Bellman : `J(s) = min_a [1 + J(s')]` sur `rc.children(s)`, avec J(résolu) = 0.
3. Réseau : un MLP sur l'encodage one-hot (54 × 6 entrées) qui prédit une seule valeur.
4. Recherche : A* pondéré par lots (étapes 3 et 4) avec h = J.
5. Comparer à l'étape 5a : longueur des solutions et temps.

---

## Organisation suggérée

```
solveur/
    bench.py          # étape 0 : jeux de mélanges, mesures, courbes
    recherche.py      # A*, A* pondéré, IDA*, variantes
    heuristiques.py   # h = 0, autocollants mal placés, Manhattan, bases de motifs
    tests.py          # contrôles : nombres d'états par profondeur, admissibilité, solutions valides
```

Les ajouts C++ éventuels (heuristiques par lots, bases de motifs, IDA*) vont dans `CCubeCore` et sont exposés par `rubikcore.cpp`. Ils sont compilés par `make` et documentés dans `API_PYTHON.md`.
