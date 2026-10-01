#ifndef CCUBECORE_H
#define CCUBECORE_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

// Cœur logique du cube, sans Qt : partagé par l'IHM (CRubik) et le module Python autonome (rubikcore).
//
// Notation standard : U haut (+y), D bas, F avant (+z, vers l'observateur), B arrière, R droite (+x), L gauche ;
// chaque coup tourne dans le sens horaire vu depuis sa face, M comme L, E comme D, S comme F.
// Couleurs standard : U blanc, R rouge, F vert, D jaune, L orange, B bleu.
//
// Un état est un tableau de 54 autocollants dans l'ordre des « facelets » de Kociemba (U1..U9 R1..R9 F1..F9
// D1..D9 L1..L9 B1..B9) ; chaque valeur est la face d'origine de la couleur : 0 = U, 1 = R, 2 = F, 3 = D,
// 4 = L, 5 = B (la face i du cube résolu porte donc la valeur i). Chaque coup est une permutation des 54 autocollants.
class CCubeCore {
public:
    static const int NBSTICKERS = 54;
    // Ordre des coups : U U' D D' R R' L L' F F' B B' M M' E E' S S' (un coup puis son inverse)
    static const int NBMOVES = 18;
    // Les 12 premiers coups, sans les tranches M, E et S
    static const int NBFACEMOVES = 12;

    typedef std::array<uint8_t, NBSTICKERS> State;

    static const State& solved(void);
    static bool isSolved(const uint8_t *state);
    static void apply(const uint8_t *in, uint8_t *out, int move);
    static void apply(State& state, int move);

    static inline int inverse(int move) { return move ^ 1; }
    static inline int nbMoves(bool slices) { return slices ? NBMOVES : NBFACEMOVES; }
    static const char *moveName(int move);
    // Grammaire simple : coups séparés ou non par des espaces, chacun suivi d'un ' et/ou d'un chiffre de répétition (ex. "R U'2 F")
    static bool parse(const std::string& str, std::vector<int>& moves);
    // Groupe de rotation de CRubik (axe géométrique * 3 + couche 0..2) correspondant à un coup, et inversement
    static int groupe(int move);
    static int moveFromGroupe(int groupe, bool inverse);
    // Sens du quart de tour (+1/-1) pour les formules de rotation de CRubik::rotate()
    static int coefficient(int move);

    // Autocollant situé en (x, y, z) (0..2) de normale signe * axe (axe géométrique 0 = x, 1 = y, 2 = z), -1 s'il n'y en a pas
    static int stickerIndex(int x, int y, int z, int axe, int signe);
    // Position (-1..1) du cubie portant l'autocollant et normale de l'autocollant
    static void stickerGeometry(int sticker, int position[3], int normale[3]);

    // Traitements par lots (états contigus de NBSTICKERS octets)
    // k tiré uniformément dans [kMin, kMax] ; aucun coup n'annule le précédent ; last = dernier coup joué (-1 si k = 0)
    static void generate(int64_t n, int kMin, int kMax, bool slices, uint64_t seed, uint8_t *states, int32_t *ks, int32_t *last);
    // out : n * nbMoves(slices) états, les voisins de chaque état dans l'ordre des coups
    static void children(const uint8_t *states, int64_t n, bool slices, uint8_t *out);
};

#endif // CCUBECORE_H
