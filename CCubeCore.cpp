#include <cstring>
#include <random>
#include "CCubeCore.h"

namespace {
    const char *const MOVENAMES[CCubeCore::NBMOVES] = { "U", "U'", "D", "D'", "R", "R'", "L", "L'", "F", "F'", "B", "B'", "M", "M'", "E", "E'", "S", "S'" };
    const char MOVETYPES[] = "UDRLFBMES";
    // Même table que groupeMap (CMouvement.cpp) : groupe = axe de rotation * 3 + couche
    const char GROUPEMAP[] = "FSBDEULMR";

    // Rotation d'un quart de tour autour d'un axe géométrique, mêmes formules que CRubik::rotate()
    void tourne(int axe, int coef, const int v[3], int out[3]) {
        switch(axe) {
        case 0:
            out[0] = v[0];
            out[1] = v[2] * coef;
            out[2] = -v[1] * coef;
            break;
        case 1:
            out[0] = v[2] * coef;
            out[1] = v[1];
            out[2] = -v[0] * coef;
            break;
        default:
            out[0] = -v[1] * coef;
            out[1] = v[0] * coef;
            out[2] = v[2];
            break;
        }
    }

    struct STables {
        CCubeCore::State solved;
        int position[CCubeCore::NBSTICKERS][3];
        int normale[CCubeCore::NBSTICKERS][3];
        // [x][y][z][axe][signe (0 = -, 1 = +)] -> autocollant, -1 s'il n'existe pas
        int index[3][3][3][3][2];
        // Après le coup m : nouveau[i] = ancien[perm[m][i]]
        uint8_t perm[CCubeCore::NBMOVES][CCubeCore::NBSTICKERS];

        STables(void) {
            int n = 0;
            int x, y, z, m, s;

            memset(index, -1, sizeof(index));

            auto ajoute = [&](int px, int py, int pz, int axe, int signe) {
                position[n][0] = px - 1;
                position[n][1] = py - 1;
                position[n][2] = pz - 1;
                normale[n][0] = normale[n][1] = normale[n][2] = 0;
                normale[n][axe] = signe;
                index[px][py][pz][axe][signe > 0] = n;
                solved[n] = static_cast<uint8_t>(n / 9);
                n++;
            };

            // Même ordre que rubik.map()
            for(z=0;z<3;z++) for(y=0;y<3;y++) ajoute(0, y, z, 0, -1);
            for(z=0;z<3;z++) for(y=0;y<3;y++) ajoute(2, y, z, 0, 1);
            for(x=2;x>=0;x--) for(y=0;y<3;y++) ajoute(x, y, 0, 2, -1);
            for(x=2;x>=0;x--) for(y=0;y<3;y++) ajoute(x, y, 2, 2, 1);
            for(x=2;x>=0;x--) for(z=0;z<3;z++) ajoute(x, 0, z, 1, -1);
            for(x=2;x>=0;x--) for(z=0;z<3;z++) ajoute(x, 2, z, 1, 1);

            for(m=0;m<CCubeCore::NBMOVES;m++) {
                int groupe = static_cast<int>(strchr(GROUPEMAP, MOVETYPES[m / 2]) - GROUPEMAP);
                int axe = groupe / 3;
                int couche = groupe % 3 - 1;
                int coef = (m & 1) ? -1 : 1;

                for(s=0;s<CCubeCore::NBSTICKERS;s++) {
                    int p[3], d[3], a;

                    if(position[s][axe] != couche) {
                        perm[m][s] = static_cast<uint8_t>(s);
                        continue;
                    }

                    tourne(axe, coef, position[s], p);
                    tourne(axe, coef, normale[s], d);

                    for(a=0;d[a]==0;a++);

                    perm[m][index[p[0] + 1][p[1] + 1][p[2] + 1][a][d[a] > 0]] = static_cast<uint8_t>(s);
                }
            }
        }
    };

    const STables& tables(void) {
        // Construit au premier appel (initialisation thread-safe en C++11)
        static const STables t;

        return t;
    }
}

const CCubeCore::State& CCubeCore::solved(void) {
    return tables().solved;
}

bool CCubeCore::isSolved(const uint8_t *state) {
    // Chaque face d'une seule couleur (les tranches pouvant déplacer les centres)
    for(int f=0;f<6;f++) {
        for(int i=0;i<9;i++) {
            if(state[f * 9 + i] != state[f * 9 + 4]) {
                return false;
            }
        }
    }

    return true;
}

void CCubeCore::apply(const uint8_t *in, uint8_t *out, int move) {
    const uint8_t *perm = tables().perm[move];

    for(int i=0;i<NBSTICKERS;i++) {
        out[i] = in[perm[i]];
    }
}

void CCubeCore::apply(State& state, int move) {
    State tmp = state;

    apply(tmp.data(), state.data(), move);
}

const char *CCubeCore::moveName(int move) {
    return MOVENAMES[move];
}

bool CCubeCore::parse(const std::string& str, std::vector<int>& moves) {
    size_t i = 0;

    moves.clear();

    while(i < str.size()) {
        const char *type;
        int move, nb = 1;

        if(str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n') {
            i++;
            continue;
        }

        type = (str[i] != '\0' ? strchr(MOVETYPES, str[i]) : nullptr);
        if(type == nullptr) {
            return false;
        }

        move = static_cast<int>(type - MOVETYPES) * 2;
        i++;

        if(i < str.size() && str[i] == '\'') {
            move++;
            i++;
        }

        if(i < str.size() && str[i] >= '0' && str[i] <= '9') {
            nb = str[i] - '0';
            i++;
        }

        moves.insert(moves.end(), nb, move);
    }

    return true;
}

int CCubeCore::moveFromGroupe(int groupe, bool inverse) {
    return static_cast<int>(strchr(MOVETYPES, GROUPEMAP[groupe]) - MOVETYPES) * 2 + (inverse ? 1 : 0);
}

int CCubeCore::stickerIndex(int x, int y, int z, int axe, int signe) {
    if(x < 0 || x > 2 || y < 0 || y > 2 || z < 0 || z > 2 || axe < 0 || axe > 2 || signe == 0) {
        return -1;
    }

    return tables().index[x][y][z][axe][signe > 0];
}

void CCubeCore::stickerGeometry(int sticker, int position[3], int normale[3]) {
    const STables& t = tables();

    memcpy(position, t.position[sticker], sizeof(t.position[sticker]));
    memcpy(normale, t.normale[sticker], sizeof(t.normale[sticker]));
}

void CCubeCore::generate(int64_t n, int kMin, int kMax, bool slices, uint64_t seed, uint8_t *states, int32_t *ks, int32_t *last) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> distK(kMin, kMax);
    std::uniform_int_distribution<int> distPremier(0, nbMoves(slices) - 1);
    std::uniform_int_distribution<int> distSuivant(0, nbMoves(slices) - 2);

    for(int64_t i=0;i<n;i++) {
        State state = solved();
        int k = distK(rng);
        int precedent = -1;

        for(int j=0;j<k;j++) {
            int move;

            if(precedent < 0) {
                move = distPremier(rng);
            } else {
                // Tirage parmi les coups restants une fois l'inverse du précédent exclu
                move = distSuivant(rng);
                if(move >= inverse(precedent)) {
                    move++;
                }
            }

            apply(state, move);
            precedent = move;
        }

        memcpy(states + i * NBSTICKERS, state.data(), NBSTICKERS);
        ks[i] = k;
        last[i] = precedent;
    }
}

void CCubeCore::children(const uint8_t *states, int64_t n, bool slices, uint8_t *out) {
    const int nb = nbMoves(slices);

    for(int64_t i=0;i<n;i++) {
        for(int m=0;m<nb;m++) {
            apply(states + i * NBSTICKERS, out + (i * nb + m) * NBSTICKERS, m);
        }
    }
}
