# API Python de clbrub

Deux modules Python partagent le même cœur logique (`CCubeCore`, C++ sans Qt) et le même format d'état :

| Module | Où l'utiliser | Rôle |
|---|---|---|
| `rubik` | Uniquement dans la fenêtre de script de l'IHM (interpréteur embarqué) | Piloter **le** cube affiché : mélanger, jouer des coups animés, visualiser un état |
| `rubikcore` | N'importe quel Python : terminal, notebook, entraînement PyTorch… | Simuler des **lots** de cubes avec numpy : génération de données, voisins, recherche |

Un état produit par l'un peut être donné à l'autre (`rubik.set_state(etat_rubikcore)`).

---

## Compilation

```bash
qmake && make          # IHM + module rubikcore (reconstruit seulement si ses sources changent)
make rubikcore         # module seul
qmake PYTHON=/chemin/vers/python3 && make   # autre interpréteur (ex. venv avec PyTorch)
```

Sans make : `python3 setup.py build_ext --inplace`.

Le module est produit à la racine du projet (`rubikcore.cpython-*.so`). Pour l'importer ailleurs, ajouter ce dossier au `PYTHONPATH` (y compris pour l'importer depuis l'IHM si elle n'est pas lancée depuis le projet).

---

## Conventions communes

Le projet suit la **notation standard** (Singmaster) et le **schéma de couleurs standard**. Un état se convertit directement au format « facelets » de Kociemba, utilisé par la plupart des outils (par exemple le paquet Python `kociemba`).

### Faces et couleurs

| Face | Position (repère 3D de l'IHM) | Couleur | Valeur dans un état |
|---|---|---|---|
| U | haut (+y) | blanc | 0 |
| R | droite (+x) | rouge | 1 |
| F | avant (+z, vers l'observateur) | vert | 2 |
| D | bas (−y) | jaune | 3 |
| L | gauche (−x) | orange | 4 |
| B | arrière (−z) | bleu | 5 |

Par défaut, la vue 3D montre U en haut, F devant à gauche et R devant à droite.

### Format d'un état

Un état est une suite de **54 octets**, un par autocollant. Chaque valeur désigne la face d'origine de la couleur de l'autocollant (0 = U … 5 = B, voir le tableau ci-dessus). Sur le cube résolu, la face `f` porte donc la valeur `f` partout.

Les autocollants suivent l'ordre de Kociemba : `U1…U9 R1…R9 F1…F9 D1…D9 L1…L9 B1…B9`. Chaque face est lue ligne par ligne, telle qu'elle apparaît sur le patron (c'est aussi la disposition de la carte 2D de l'IHM) :

```
          U1 U2 U3
          U4 U5 U6
          U7 U8 U9
L1 L2 L3  F1 F2 F3  R1 R2 R3  B1 B2 B3
L4 L5 L6  F4 F5 F6  R4 R5 R6  B4 B5 B6
L7 L8 L9  F7 F8 F9  R7 R8 R9  B7 B8 B9
          D1 D2 D3
          D4 D5 D6
          D7 D8 D9
```

L'autocollant `Xn` (n de 1 à 9) a pour indice `face(X) * 9 + n − 1`, et le centre de la face `f` est l'autocollant `f * 9 + 4`. Conversion vers une chaîne Kociemba :

```python
"".join("URFDLB"[v] for v in etat)    # "UUUUUUUUURRR…" pour le cube résolu
```

Pour un 3×3, ces 54 valeurs suffisent à décrire l'état sans ambiguïté, donc un état peut servir de clé (`bytes`) dans un dictionnaire ou un ensemble.

« Résolu » signifie que chaque face est d'une seule couleur. Après des coups de tranche, les centres ont pu bouger : un cube résolu mais tourné dans son ensemble compte donc comme résolu.

### Coups

Chaque coup tourne d'un quart de tour **dans le sens horaire, vu depuis sa face** ; `'` désigne le sens inverse. Les tranches du milieu suivent la convention standard : M tourne comme L, E comme D, S comme F.

Les coups ont un **indice** fixe : chaque coup est suivi de son inverse, donc `inverse(m) == m ^ 1`.

| Indice | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Coup | U | U' | D | D' | R | R' | L | L' | F | F' | B | B' | M | M' | E | E' | S | S' |

Les 12 premiers sont les coups de face. M, E et S sont les tranches du milieu.

Les solutions du paquet `kociemba` (par exemple `"R U2 F' …"`) se jouent telles quelles avec `rubik.exec`, `rubikcore.apply` ou `rubikcore.parse`.

---

## Module `rubik` (IHM)

Il pilote l'unique cube affiché. **Les arguments sont uniquement positionnels** : écrire `rubik.melange(20, False, False)`, pas `slices=False`.

| Fonction | Retour | Description |
|---|---|---|
| `init()` | `None` | Remet le cube à l'état résolu. |
| `melange(nb, anim, slices=True)` | `str` | Joue `nb` coups aléatoires. `anim` : animation visible. `slices=False` : coups de face uniquement, centres fixes. Retourne la séquence jouée, coups séparés par des espaces (`"R U' F"`), rejouable avec `exec`. |
| `exec(cmd, anim=True)` | `str` | Joue une série de mouvements (voir la syntaxe plus bas). Retourne le dernier coup joué, ou `""` si la série est vide ou invalide. Avec `anim=True`, chaque coup est animé (environ 200 ms par coup). |
| `inverse(cmd)` | `str` | Série inverse : `inverse("R2(UF')2 M")` → `"M' F U' F U' R' R'"`. Lève `ValueError` si la série est invalide. |
| `moves(slices=True)` | `list[str]` | Noms des coups dans l'ordre des indices : 18 coups, ou 12 avec `False`. |
| `win()` | `bool` | Vrai si le cube est résolu. |
| `get_state()` | `bytes` (54) | État courant, au format commun. |
| `set_state(etat)` | `None` | Place le cube dans un état. Accepte `bytes`, `bytearray` ou un tableau numpy `uint8` contigu de 54 valeurs. Lève `ValueError` si l'état est invalide. |
| `map()` | `list[int]` (324) | État encodé en one-hot : 54 autocollants × 6 valeurs, dans l'ordre du format commun. |
| `display(actif)` | `None` | `False` : plus aucun rafraîchissement de l'affichage, et les animations deviennent instantanées. À utiliser pour les longs calculs. `True` : rétablit l'affichage et le met à jour. |
| `seed(n)` | `None` | Graine du générateur aléatoire de `melange`, pour obtenir des mélanges reproductibles. |
| `debug(x, y, z)` | `None` | Affiche dans la console les informations du cubie en `(x, y, z)` (0..2). |

### Syntaxe des séries de mouvements (`exec`, `inverse`)

- Coups : `U D R L F B M E S`, suivis d'un `'` pour l'inverse et/ou d'un chiffre de répétition : `R'`, `U2`, `F'3`.
- Groupes répétés, éventuellement imbriqués : `(R U)2`, `((U F)2 R)2`. Un groupe sans chiffre est joué une fois.
- Les espaces, tabulations et retours à la ligne sont ignorés.
- Une série syntaxiquement invalide (`R(U`, `RU)`) n'exécute **rien**.
- Un caractère inconnu est ignoré, avec le message `Unkown lexem` (`"RX"` joue `R`).
- Un chiffre de répétition ne fait qu'un caractère : `R10` est invalide.

### Validité de `set_state`

`set_state` reconstruit les cubies à partir des couleurs. Il refuse :
- une taille différente de 54 ;
- une valeur en dehors de 0..5 ;
- un ensemble de couleurs qui ne correspond à aucun cubie, ou à un cubie déjà placé ;
- un coin ou une arête « miroir » (couleurs dans le mauvais ordre de rotation).

Il **accepte** des états qu'aucune suite de coups ne peut produire, par exemple un seul coin tourné.

### Exemple

```python
import rubik

rubik.init()
seq = rubik.melange(20, False, False)      # mélange instantané, coups de face
print(seq)
rubik.exec(rubik.inverse(seq))             # résolution animée
print(rubik.win())                         # True

# Calcul long sans affichage
rubik.display(False)
s = rubik.get_state()
for m in rubik.moves(False):
    rubik.set_state(s)
    rubik.exec(m, False)
    # ...
rubik.set_state(s)
rubik.display(True)
```

---

## Module `rubikcore` (autonome)

Il simule des cubes sous forme de tableaux numpy `uint8`, de forme `(54,)` pour un état seul ou `(n, 54)` pour un lot. Les calculs sont faits en C++ et **relâchent le GIL** : plusieurs threads Python peuvent donc générer des données en parallèle.

Pour les solveurs, les fonctions **excluent les tranches par défaut** (`slices=False`, 12 coups), contrairement au module `rubik`.

### Constantes

`NB_STICKERS = 54`, `NB_MOVES = 18`, `NB_FACE_MOVES = 12`.

### Fonctions

| Fonction | Retour | Description |
|---|---|---|
| `solved()` | `ndarray (54,) uint8` | L'état résolu. |
| `moves(slices=False)` | `list[str]` | Noms des coups. L'indice d'un coup est sa position dans la liste. |
| `parse(seq)` | `list[int]` | Indices des coups d'une série : `parse("R U'2 F")` → `[4, 1, 1, 8]`. Syntaxe simple : coups, `'`, un chiffre de répétition, espaces. **Pas de parenthèses.** Lève `ValueError` si la série est invalide. |
| `inverse(m)` | `int` | Indice du coup inverse (`m ^ 1`). |
| `apply(states, moves)` | `ndarray`, même forme que `states` | Applique des coups, sans modifier `states`. `moves` peut être : un **indice** (le même coup pour tous) ; une **chaîne** (la même série pour tous) ; une **liste d'indices**, jouée comme une série si `states` est un état seul ; un **tableau `(n,)`**, soit un coup par état si `states` est un lot. |
| `children(states, slices=False)` | `ndarray (n, 12, 54)` ou `(12, 54)` | Les voisins de chaque état. `children(s)[:, m]` est l'état obtenu en jouant le coup `m`. Avec `slices=True` : 18 voisins. |
| `is_solved(states)` | `bool` ou `ndarray (n,) bool` | Vrai si l'état est résolu. |
| `generate(n, k_max, k_min=1, slices=False, seed=None)` | `(states, ks, last)` | `n` états mélangés. Pour chacun, `k` est tiré uniformément dans `[k_min, k_max]` et `k` coups aléatoires sont joués depuis l'état résolu. Aucun coup n'annule directement le précédent. Résultats : `states` `(n, 54) uint8`, `ks` `(n,) int32` (nombre de coups joués), `last` `(n,) int32` (dernier coup joué, `-1` si `k = 0`). `inverse(last)` est donc le coup qui défait le dernier pas. `seed` rend le résultat reproductible ; `None` donne une graine aléatoire. |

Toutes les entrées sont vérifiées. Une forme différente de `(…, 54)`, une valeur hors de 0..5, un indice hors de 0..17, une série invalide ou des coups non entiers lèvent `ValueError` ou `TypeError`.

### Points à connaître

- `k` est une **borne haute** de la distance au cube résolu, pas sa valeur exacte. Certaines redondances subsistent (`R R R` = `R'`, ou `R L R'` = `L`).
- Pour l'entrée d'un réseau, encoder en one-hot côté numpy ou PyTorch : `np.eye(6, dtype=np.float32)[states]` → `(n, 54, 6)`, ou `torch.nn.functional.one_hot`.
- Pour l'ensemble des états déjà visités d'une recherche, `state.tobytes()` sert de clé.
- Ordres de grandeur mesurés : `generate(1_000_000, 30)` ≈ 0,5 s ; `children` sur 1 M d'états (soit 12 M de voisins) ≈ 0,4 s ; 4 threads ≈ ×3,8.

### Exemple

```python
import numpy as np
import rubikcore as rc

# Données d'entraînement : prédire le coup qui défait le dernier pas
states, ks, last = rc.generate(100_000, 25, seed=0)
x = np.eye(6, dtype=np.float32)[states]           # (n, 54, 6)
y = np.array([rc.inverse(m) for m in last])         # étiquettes 0..11

# Développer un nœud de recherche
s = rc.apply(rc.solved(), "R U F' D2")
voisins = rc.children(s)                            # (12, 54)
print(rc.is_solved(rc.apply(s, rc.parse("D'2 F U' R'"))))   # True

# Résoudre avec le paquet kociemba (pip install kociemba) : même format et même notation
#   import kociemba
#   solution = kociemba.solve("".join("URFDLB"[v] for v in s))
#   assert rc.is_solved(rc.apply(s, solution))

# Visualiser une solution trouvée, dans la fenêtre de script de l'IHM
#   import rubik
#   rubik.set_state(s)
#   rubik.exec(" ".join(rc.moves()[m] for m in solution))
```
