#ifndef CCUBECORE_H
#define CCUBECORE_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

// Cœur logique du cube, sans Qt : partagé par l'IHM (CRubik) et le module Python autonome (rubikcore).
//
// Un état est un tableau de 54 autocollants (couleur 0..5), dans l'ordre de rubik.map() :
// 6 faces de 9 autocollants (-x, +x, -z, +z, -y, +y), la face i du cube résolu portant la couleur i.
// Chaque coup est une permutation de ces 54 autocollants.
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
    // Coup correspondant à un groupe de rotation de CRubik (cf. groupeMap de CMouvement)
    static int moveFromGroupe(int groupe, bool inverse);

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
