//
// Created by Anatol on 13/02/2023.
//
#include "kociemba.h"
#include "database.h"
#include "solver_util.h"
#include "util/RedundantMovePreventor.h"

#include <bitset>
#include <vector>
#include <utility>
#include <iostream>
#include <functional>
#include <queue>
#include <map>
#include <set>
#include <random>
#include <chrono>
#include <optional>
#include <thread>

int PHASE_TWO_MOVES[10] = {
        4, 5,
        6, 7, 8, 9, 10, 11,
        16, 17
};

template<size_t Size>
constexpr std::bitset<12> cornerMask(std::array<Edge, Size> edges) {
    std::bitset<12> mask;
    for (auto edge : edges) {
        mask.set((int) edge);
    }
    return mask;
}

constexpr
FastRubiksCube makeCube(std::array<std::pair<Corner, std::pair<Corner, int>>, 8> cornerPerm, std::array<std::pair<Edge, std::pair<Edge, int>>, 12> edgePerm) {
    FastRubiksCube res = {};

    for (int i = 0; i < 8; i++) {
        res.corners[cornerPerm[i].first] = cornerPerm[i].second.first;
        res.cornerOrientations[cornerPerm[i].second.first] = cornerPerm[i].second.second;
    }

    for (int i = 0; i < 12; i++) {
        res.edges[edgePerm[i].first] = edgePerm[i].second.first;
        res.edgeOrientations[edgePerm[i].second.first] = edgePerm[i].second.second;
    }

    return res;
}

const std::bitset<12> UD_MASK = cornerMask<4>({Edge::FRONT_LEFT, Edge::FRONT_RIGHT, Edge::BACK_LEFT, Edge::BACK_RIGHT});

const FastRubiksCube S_F2 = makeCube(
        std::array<std::pair<Corner, std::pair<Corner, int>>, 8>{
                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_RIGHT_FRONT, {Corner::BOTTOM_LEFT_FRONT, 0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_LEFT_FRONT,  {Corner::TOP_RIGHT_FRONT,  0}},

                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_LEFT_FRONT, {Corner::BOTTOM_RIGHT_FRONT, 0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_RIGHT_FRONT,  {Corner::TOP_LEFT_FRONT,  0}},

                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_RIGHT_BACK, {Corner::BOTTOM_LEFT_BACK, 0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_LEFT_BACK,  {Corner::TOP_RIGHT_BACK,  0}},

                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_LEFT_BACK, {Corner::BOTTOM_RIGHT_BACK, 0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_RIGHT_BACK,  {Corner::TOP_LEFT_BACK,  0}},
        },

        {
                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_FRONT, {Edge::BOTTOM_FRONT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_FRONT, {Edge::TOP_FRONT, 0}},

                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_RIGHT, {Edge::BOTTOM_LEFT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_LEFT, {Edge::TOP_RIGHT, 0}},

                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_LEFT, {Edge::BOTTOM_RIGHT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_RIGHT, {Edge::TOP_LEFT, 0}},

                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_BACK, {Edge::BOTTOM_BACK, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_BACK, {Edge::TOP_BACK, 0}},

                std::pair<Edge, std::pair<Edge, int>>{Edge::FRONT_LEFT, {Edge::FRONT_RIGHT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::FRONT_RIGHT, {Edge::FRONT_LEFT, 0}},

                std::pair<Edge, std::pair<Edge, int>>{Edge::BACK_LEFT, {Edge::BACK_RIGHT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BACK_RIGHT, {Edge::BACK_LEFT, 0}},
        }
);

//Clockwise turn of cube around U axis
const FastRubiksCube S_U4 = makeCube(
        std::array<std::pair<Corner, std::pair<Corner, int>>, 8>{
                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_RIGHT_FRONT, {Corner::TOP_LEFT_FRONT, 0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_LEFT_FRONT,  {Corner::TOP_LEFT_BACK,  0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_LEFT_BACK,   {Corner::TOP_RIGHT_BACK, 0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_RIGHT_BACK,  {Corner::TOP_RIGHT_FRONT, 0}},

                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_RIGHT_FRONT, {Corner::BOTTOM_LEFT_FRONT, 0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_LEFT_FRONT,  {Corner::BOTTOM_LEFT_BACK,  0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_LEFT_BACK,   {Corner::BOTTOM_RIGHT_BACK, 0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_RIGHT_BACK,  {Corner::BOTTOM_RIGHT_FRONT, 0}}
        },

        std::array<std::pair<Edge, std::pair<Edge, int>>, 12> {
                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_FRONT, {Edge::TOP_LEFT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_LEFT,  {Edge::TOP_BACK, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_BACK,  {Edge::TOP_RIGHT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_RIGHT, {Edge::TOP_FRONT, 0}},

                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_FRONT, {Edge::BOTTOM_LEFT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_LEFT,  {Edge::BOTTOM_BACK, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_BACK,  {Edge::BOTTOM_RIGHT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_RIGHT, {Edge::BOTTOM_FRONT, 0}},

                std::pair<Edge, std::pair<Edge, int>>{Edge::FRONT_LEFT,  {Edge::BACK_LEFT, 1}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BACK_LEFT,   {Edge::BACK_RIGHT, 1}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BACK_RIGHT,  {Edge::FRONT_RIGHT, 1}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::FRONT_RIGHT, {Edge::FRONT_LEFT, 1}}
        }
);

const FastRubiksCube S_LR2_BASE = makeCube(
        std::array<std::pair<Corner, std::pair<Corner, int>>, 8>{
                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_RIGHT_FRONT, {Corner::TOP_LEFT_FRONT, 0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_LEFT_FRONT,  {Corner::TOP_RIGHT_FRONT, 0}},

                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_RIGHT_BACK,  {Corner::TOP_LEFT_BACK,  0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::TOP_LEFT_BACK,   {Corner::TOP_RIGHT_BACK,  0}},

                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_RIGHT_FRONT, {Corner::BOTTOM_LEFT_FRONT, 0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_LEFT_FRONT,  {Corner::BOTTOM_RIGHT_FRONT, 0}},

                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_RIGHT_BACK,  {Corner::BOTTOM_LEFT_BACK,  0}},
                std::pair<Corner, std::pair<Corner, int>>{Corner::BOTTOM_LEFT_BACK,   {Corner::BOTTOM_RIGHT_BACK,  0}}
        },

        std::array<std::pair<Edge, std::pair<Edge, int>>, 12> {
                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_FRONT, {Edge::TOP_FRONT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_BACK, {Edge::TOP_BACK, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_FRONT, {Edge::BOTTOM_FRONT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_BACK, {Edge::BOTTOM_BACK, 0}},

                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_LEFT,  {Edge::TOP_RIGHT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::TOP_RIGHT, {Edge::TOP_LEFT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_LEFT,  {Edge::BOTTOM_RIGHT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BOTTOM_RIGHT, {Edge::BOTTOM_LEFT, 0}},

                std::pair<Edge, std::pair<Edge, int>>{Edge::FRONT_LEFT,  {Edge::FRONT_RIGHT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::FRONT_RIGHT, {Edge::FRONT_LEFT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BACK_LEFT,  {Edge::BACK_RIGHT, 0}},
                std::pair<Edge, std::pair<Edge, int>>{Edge::BACK_RIGHT, {Edge::BACK_LEFT, 0}}
        }
);

FastRubiksCube apply_S_F2(const FastRubiksCube& cube) {
    return cube.applyBasicSymmetry(S_F2);
}

FastRubiksCube apply_S_U4(const FastRubiksCube& cube) {
    return cube.applyBasicSymmetry(S_U4);
}

FastRubiksCube apply_S_LR2(const FastRubiksCube& cube) {
    FastRubiksCube res = cube.applyBasicSymmetry(S_LR2_BASE);

    for (int i = 0; i < 8; i++) {
        res.cornerOrientations[i] = (3 - res.cornerOrientations[i]) % 3;
    }

    return res;
}

FastRubiksCube applySymmetry(const FastRubiksCube& cube, int idx) {
    FastRubiksCube res = cube;

    if (idx >= 8) {
        res = apply_S_LR2(res);
        idx -= 8;
    }

    while (idx >= 2) {
        res = apply_S_U4(res);
        idx -= 2;
    }

    if (idx == 1) {
        res = apply_S_F2(res);
    }

    return res;
}
int REVERSE_SYMMETRIES[16];
int SYM_MULT[16][16];

FastRubiksCube reduceTest(const FastRubiksCube& cube) {
    FastRubiksCube best = cube;

    uint32_t bestCoord = flipUDSliceCoordinate(best);

    for (int i = 1; i < 16; i++) {
        FastRubiksCube cur = applySymmetry(cube, i);

        uint32_t curCoord = flipUDSliceCoordinate(cur);

        if (curCoord < bestCoord) {
            best = cur;
            bestCoord = curCoord;
        }
    }

    return best;
}

FastRubiksCube applyTestSymmetry(const FastRubiksCube& cube) {
    return apply_S_F2(cube);
}

uint32_t cornerOrientationCoordinate(const FastRubiksCube& cube) {
    return cube.getCornerOrientationIndex();
}

uint32_t POWERS_OF_THREE[8] = {1, 3, 9, 27, 81, 243, 729, 0};
uint32_t positionalCornerOrientationCoordinate(const FastRubiksCube& cube) {
    uint32_t res = 0;
    for (int i = 0; i < 8; i++) {
        res += POWERS_OF_THREE[cube.corners[i]] * cube.cornerOrientations[i];
    }

    return res;
}

uint32_t edgeOrientationCoordinate(const FastRubiksCube& cube) {
    return cube.getEdgeOrientationIndex();
}

uint32_t POWERS_OF_TWO[12] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 0};
uint32_t positionalEdgeOrientationCoordinate(const FastRubiksCube& cube) {
    uint32_t res = 0;
    for (int i = 0; i < 12; i++) {
        res += POWERS_OF_TWO[cube.edges[i]] * cube.edgeOrientations[i];
    }

    return res;
}

uint8_t UD_SLICE_PERM[12] = {0, 1, 2, 3, 8, 9, 10, 11, 4, 5, 6, 7};
uint32_t UDSliceCoordinate(const FastRubiksCube& cube) {
    bool occupied[12] = {false};

    occupied[UD_SLICE_PERM[cube.edges[Edge::FRONT_LEFT]]] = true;
    occupied[UD_SLICE_PERM[cube.edges[Edge::FRONT_RIGHT]]] = true;
    occupied[UD_SLICE_PERM[cube.edges[Edge::BACK_LEFT]]] = true;
    occupied[UD_SLICE_PERM[cube.edges[Edge::BACK_RIGHT]]] = true;

    uint32_t res = 0;
    int k = 3;
    int n = 11;

    while (k >= 0) {
        if (occupied[n]) {
            k--;
        } else {
            res += NCR_U32[n][k];
        }

        n--;
    }

    return res;
}

uint32_t flipUDSliceCoordinate(const FastRubiksCube& cube) {
    uint32_t udSlice = UDSliceCoordinate(cube);
    uint32_t orientation = positionalEdgeOrientationCoordinate(cube);

    return udSlice * 2048 + orientation;
}

uint32_t cornerPermutationCoordinate(const FastRubiksCube &cube) {
    return cube.getCornerPermutationIndex();
}

const uint8_t phase2EdgePermIndices[12] = {0, 1, 2, 3, 255, 255, 255, 255, 4, 5, 6, 7};
const uint8_t phase2EdgePermTargetEdges[8] = {0, 1, 2, 3, 8, 9, 10, 11};
uint32_t phase2EdgePermutationCoordinate(const FastRubiksCube &cube) {
    uint32_t seen = 0;
    uint32_t res = 0;

    for (int i = 0; i < 8; i++) {
        int v = phase2EdgePermIndices[cube.edges[phase2EdgePermTargetEdges[i]]];

#ifdef _DEBUG
        if (v == 255) {
            std::cout << "Invalid edge permutation" << std::endl;
        }
#endif

        int count = BIT_COUNT_U16[seen >> (8 - v)];
        res += (v - count) * FACTORIAL_U32[7 - i];
        seen |= (1 << (7 - v));
    }

    return res;
}

const uint8_t phase2UDSliceEdgeIndices[12] = {
        255, 255, 255, 255,
        0, 1, 2, 3,
        255, 255, 255, 255
};
const uint8_t phase2UDSliceEdgeTargetEdges[4] = {4, 5, 6, 7};
uint32_t phase2UDSliceCoordinate(const FastRubiksCube &cube) {
    uint32_t seen = 0;
    uint32_t res = 0;

    for (int i = 0; i < 4; i++) {
        int v = phase2UDSliceEdgeIndices[cube.edges[phase2UDSliceEdgeTargetEdges[i]]];

#ifdef _DEBUG
        if (v == 255) {
            std::cout << "Invalid UDSlice permutation" << std::endl;
        }
#endif

        int count = BIT_COUNT_U16[seen >> (4 - v)];
        res += (v - count) * FACTORIAL_U32[3 - i];
        seen |= (1 << (3 - v));
    }

    return res;
}


struct SymCoordLookup {
    std::vector<uint32_t> rawCoordToSymCoord;
    std::vector<uint32_t> classIndexToRepresentant;
};

struct SymCoordLookupSerializer {
    static void serialize(SymCoordLookup* ptr, uint64_t size, std::ostream& out) {
        size_t coordToClassLen = ptr->rawCoordToSymCoord.size();
        size_t classIndexToRepresentantLen = ptr->classIndexToRepresentant.size();

        out.write((char*) &coordToClassLen, sizeof(size_t));
        out.write((char*) ptr->rawCoordToSymCoord.data(), coordToClassLen * sizeof(uint32_t));

        out.write((char*) &classIndexToRepresentantLen, sizeof(size_t));
        out.write((char*) ptr->classIndexToRepresentant.data(), classIndexToRepresentantLen * sizeof(uint32_t));
    }

    static void deserialize(SymCoordLookup* ptr, uint64_t size, std::istream& in) {
        new (&ptr->rawCoordToSymCoord) std::vector<uint32_t>();
        new (&ptr->classIndexToRepresentant) std::vector<uint32_t>();

        size_t coordToClassLen;
        in.read((char*) &coordToClassLen, sizeof(size_t));
        ptr->rawCoordToSymCoord.resize(coordToClassLen);
        in.read((char*) ptr->rawCoordToSymCoord.data(), coordToClassLen * sizeof(uint32_t));

        size_t classIndexToRepresentantLen;
        in.read((char*) &classIndexToRepresentantLen, sizeof(size_t));
        ptr->classIndexToRepresentant.resize(classIndexToRepresentantLen);
        in.read((char*) ptr->classIndexToRepresentant.data(), classIndexToRepresentantLen * sizeof(uint32_t));
    }
};

void constructSymCoordLookup(SymCoordLookup* out, std::function<uint32_t (const FastRubiksCube&)> coordFunc, std::vector<int> moves) {
    std::cout << "Generating lookup table..." << std::endl;
    std::set<uint32_t> visited;

    new (&out->rawCoordToSymCoord) std::vector<uint32_t>();
    new (&out->classIndexToRepresentant) std::vector<uint32_t>();

    std::queue<FastRubiksCube> q;
    q.push(FastRubiksCube());

    std::map<uint32_t, uint32_t> representatives;
    uint32_t progress = 0;

    while (!q.empty()) {
        FastRubiksCube cur = q.front();
        q.pop();

        uint32_t coord = coordFunc(cur);

        if (visited.find(coord) != visited.end()) {
            continue;
        }

        visited.insert(coord);
        progress++;

        int bestIdx = -1;
        uint32_t bestCoord = UINT32_MAX;

        for (int i = 0; i < 16; i++) {
            FastRubiksCube sym = applySymmetry(cur, i);
            uint32_t symCoord = coordFunc(sym);

            if (symCoord < bestCoord) {
                bestIdx = i;
                bestCoord = symCoord;
            }
        }

        uint32_t representativeIdx;
        if (representatives.find(bestCoord) == representatives.end()) {
            representativeIdx = representatives.size();
            representatives[bestCoord] = representativeIdx;

            out->classIndexToRepresentant.resize(std::max(out->classIndexToRepresentant.size(), (size_t) (representativeIdx + 1)));
            out->classIndexToRepresentant[representativeIdx] = bestCoord;
        } else {
            representativeIdx = representatives[bestCoord];
        }

        out->rawCoordToSymCoord.resize(std::max(out->rawCoordToSymCoord.size(), (size_t) (coord + 1)));
        out->rawCoordToSymCoord[coord] = representativeIdx * 16 + REVERSE_SYMMETRIES[bestIdx];

        for (int i: moves) {
            q.push(cur.doMove(i));
        }

        if (progress % 100000 == 0) {
            std::cout << "Processed " << progress << " states" << std::endl;
        }
    }

    std::cout << "Found " << representatives.size() << " equivalence classes" << std::endl;
    std::cout << "Processed " << progress << " states" << std::endl;
}

struct MoveTable {
    uint32_t moves[18];
};

void constructMoveTable(MoveTable* out, std::function<uint32_t (const FastRubiksCube&)> coordFunc, std::vector<int> moves) {
    std::cout << "Generating move table..." << std::endl;
    std::set<uint32_t> visited;

    std::queue<FastRubiksCube> q;
    q.push(FastRubiksCube());

    uint32_t progress = 0;

    while (!q.empty()) {
        FastRubiksCube cur = q.front();
        q.pop();

        uint32_t coord = coordFunc(cur);

        if (visited.find(coord) != visited.end()) {
            continue;
        }

        visited.insert(coord);
        progress++;

        for (int i = 0; i < moves.size(); i++) {
            int actualMove = moves[i];
            FastRubiksCube res = cur.doMove(actualMove);

            uint32_t resCoord = coordFunc(res);
            out[coord].moves[actualMove] = resCoord;

            q.push(res);
        }

        if (progress % 100000 == 0) {
            std::cout << "Processed " << progress << " states" << std::endl;
        }
    }

    std::cout << "Processed " << progress << " states" << std::endl;
}

struct SymMoveTable {
    std::vector<MoveTable> tables;
    uint8_t symMoves[16][18];

    uint32_t doMove(uint32_t symCoord, int move) {
        /*
         Given a sym coord where j is the equivalence class and i is the index of the symmetry to get from R(j) to the position.
         Therefore the actual position is:
         S_i' * R(j) * S_i

         And we want to apply a move M to it so we want to find the sym coords of
         M * (S_i' * R(j) * S_i)

         We can reformulate:
         (S_i' * S_i) * M * (S_i' * R(j) * S_i)
         = S_i' * (S_i * M * S_i') * (R(j) * S_i)

         (S_i * M * S_i') is simply the move under a given symmetry. Let's denote it M_i

         S_i' * M_i * (R(j) * S_i)
         = S_i' * (M_i * R(j)) * S_i

         M_i * R(j) can also have a sym coord i1 and j1, so M_i * R(j) = S'_{i1} * R(j1) * S_{i1} and we get
         S_i' * (S'_{i1} * R(j1) * S_{i1}) * S_i

         Let S_i2 denote the composition of S_i and S_i1, so S_i2 = S_i * S_{i1}
         S_i2' * R(j1) * S_i2
         */

        int j = symCoord / 16;
        int i = symCoord % 16;

        int m1 = symMoves[REVERSE_SYMMETRIES[i]][move];

        int m1_times_rj = tables[j].moves[m1];
        int i1 = m1_times_rj % 16;
        int j1 = m1_times_rj / 16;

        int i2 = SYM_MULT[i1][i];

        return j1 * 16 + i2;
    }
};

struct SymMoveTableSerializer {
    static void serialize(const SymMoveTable* ptr, uint64_t size, std::ostream& out) {
        size_t len = ptr->tables.size();
        out.write((char*) &len, sizeof(size_t));
        out.write((char*) ptr->tables.data(), len * sizeof(MoveTable));

        out.write((char*) ptr->symMoves, 16 * 18 * sizeof(uint8_t));
    }

    static void deserialize(SymMoveTable* ptr, uint64_t size, std::istream& in) {
        new (&ptr->tables) std::vector<MoveTable>();

        size_t len;
        in.read((char*) &len, sizeof(size_t));
        ptr->tables.resize(len);
        in.read((char*) ptr->tables.data(), len * sizeof(MoveTable));

        in.read((char*) ptr->symMoves, 16 * 18 * sizeof(uint8_t));
    }
};

void constructSymMoveTable(SymMoveTable* out, std::function<uint32_t (const FastRubiksCube&)> coordFunc, const SymCoordLookup* lookup, std::vector<int> availableMoves) {
    std::cout << "Generating sym table..." << std::endl;
    new (&out->tables) std::vector<MoveTable>(lookup->classIndexToRepresentant.size());

    std::vector<bool> visited(lookup->classIndexToRepresentant.size() * 16, false);

    std::queue<FastRubiksCube> q;

    q.push(FastRubiksCube());
    int progress = 0;

    while (!q.empty()) {
        FastRubiksCube cur = q.front();
        q.pop();

        uint32_t coord = coordFunc(cur);
        uint32_t symCoord = lookup->rawCoordToSymCoord[coord];

        uint32_t classIdx = symCoord / 16;
        uint32_t symIdx = symCoord % 16;

        if (visited[symCoord]) {
            continue;
        }

        visited[symCoord] = true;

        for (int i = 0; i < availableMoves.size(); i++) {
            int actualMove = availableMoves[i];
            FastRubiksCube res = cur.doMove(actualMove);

            if (symIdx == 0) {
                uint32_t rawCoord = coordFunc(res);
                uint32_t symCoord = lookup->rawCoordToSymCoord[rawCoord];

                out->tables[classIdx].moves[actualMove] = symCoord;
            }

            q.push(res);
        }

        progress++;

        if (progress % 10000 == 0) {
            std::cout << "Processed " << progress << " states" << std::endl;
        }
    }

    FastRubiksCube moves[18];
    for (int i = 0; i < availableMoves.size(); i++) {
        moves[availableMoves[i]] = FastRubiksCube().doMove(availableMoves[i]);
    }

    for (int i: availableMoves) {
        for (int j = 0; j < 16; j++) {
            FastRubiksCube sym = applySymmetry(moves[i], j);

            uint8_t moveIdx = -1;
            for (int k: availableMoves) {
                if (sym == moves[k]) {
                    moveIdx = k;
                    break;
                }
            }

            if (moveIdx == -1) {
                std::cout << "Error: move not found" << std::endl;
                exit(1);
            }

            out->symMoves[j][i] = moveIdx;
        }
    }
}

Database<SymCoordLookup, SymCoordLookupSerializer> FLIP_UD_SLICE_SYM_COORDS(
        sizeof(SymCoordLookup),
        "data/flip_ud_slice_sym_coords.bin",
        [](SymCoordLookup* out) {
            constructSymCoordLookup(out, flipUDSliceCoordinate, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17});
        }
);

Database<SymMoveTable, SymMoveTableSerializer> FLIP_UD_SLICE_SYM_MOVE_TABLE(
        sizeof(SymMoveTable),
        "data/flip_ud_sym_moves.bin",
        [](SymMoveTable* out) {
            FLIP_UD_SLICE_SYM_COORDS.ensureLoaded();
            constructSymMoveTable(out, flipUDSliceCoordinate, FLIP_UD_SLICE_SYM_COORDS.ptr, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17});
        }
);

Database<SymCoordLookup, SymCoordLookupSerializer> CORNER_PERM_SYM_COORDS(
        sizeof(SymCoordLookup),
        "data/corner_perm_sym_coords.bin",
        [](SymCoordLookup* out) {
            constructSymCoordLookup(out, cornerPermutationCoordinate, std::vector<int>(PHASE_TWO_MOVES, PHASE_TWO_MOVES + 10));
        }
);

Database<SymMoveTable, SymMoveTableSerializer> CORNER_PERM_SYM_MOVE_TABLE(
        sizeof(SymMoveTable),
        "data/corner_perm_sym_moves.bin",
        [](SymMoveTable* out) {
            CORNER_PERM_SYM_COORDS.ensureLoaded();
            constructSymMoveTable(out, cornerPermutationCoordinate, CORNER_PERM_SYM_COORDS.ptr, std::vector<int>(PHASE_TWO_MOVES, PHASE_TWO_MOVES + 10));
        }
);

struct SymmetryTable {
    uint32_t lookup[16];
};

void constructSymmetryTable(SymmetryTable* out, std::function<uint32_t (const FastRubiksCube&)> coordFunc, std::vector<int> moves) {
    std::cout << "Generating symmetry table..." << std::endl;
    std::set<uint32_t> visited;

    std::queue<FastRubiksCube> q;
    q.push(FastRubiksCube());

    uint32_t progress = 0;

    while (!q.empty()) {
        FastRubiksCube cur = q.front();
        q.pop();

        uint32_t coord = coordFunc(cur);

        if (visited.find(coord) != visited.end()) {
            continue;
        }

        visited.insert(coord);
        progress++;

        for (int i: moves) {
            FastRubiksCube res = cur.doMove(i);
            q.push(res);
        }

        for (int i = 0; i < 16; i++) {
            FastRubiksCube sym = applySymmetry(cur, i);
            uint32_t symCoord = coordFunc(sym);

            out[coord].lookup[i] = symCoord;
        }

        if (progress % 100 == 0) {
            std::cout << "Processed " << progress << " states" << std::endl;
        }
    }

    std::cout << "Processed " << progress << " states" << std::endl;
}

Database<MoveTable> CORNER_TWIST_MOVE_TABLE(
        sizeof(MoveTable) * 2187,
        "data/corner_twist_moves.bin",
        [](MoveTable* out) {
            constructMoveTable(out, positionalCornerOrientationCoordinate, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17});
        }
);

Database<SymmetryTable> CORNER_TWIST_SYMMETRY_TABLE(
        sizeof(SymmetryTable) * 2187,
        "data/corner_twist_symmetry.bin",
        [](SymmetryTable* out) {
            constructSymmetryTable(out, positionalCornerOrientationCoordinate, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17});
        }
);

Database<MoveTable> CORNER_PERM_MOVE_TABLE(
        sizeof(MoveTable) * 40320,
        "data/corner_perm_moves.bin",
        [](MoveTable* out) {
            constructMoveTable(out, cornerPermutationCoordinate, std::vector<int>(PHASE_TWO_MOVES, PHASE_TWO_MOVES + 10));
        }
);

Database<SymmetryTable> CORNER_PERM_SYMMETRY_TABLE(
        sizeof(SymmetryTable) * 40320,
        "data/corner_perm_symmetry.bin",
        [](SymmetryTable* out) {
            constructSymmetryTable(out, cornerPermutationCoordinate, std::vector<int>(PHASE_TWO_MOVES, PHASE_TWO_MOVES + 10));
        }
);

Database<MoveTable> PHASE_2_EDGE_PERM_MOVE_TABLE(
        sizeof(MoveTable) * 40320,
        "data/edge_perm_moves.bin",
        [](MoveTable* out) {
            constructMoveTable(out, phase2EdgePermutationCoordinate, std::vector<int>(PHASE_TWO_MOVES, PHASE_TWO_MOVES + 10));
        }
);

Database<SymmetryTable> PHASE_2_EDGE_PERM_SYMMETRY_TABLE(
        sizeof(SymmetryTable) * 40320,
        "data/edge_perm_symmetry.bin",
        [](SymmetryTable* out) {
            constructSymmetryTable(out, phase2EdgePermutationCoordinate, std::vector<int>(PHASE_TWO_MOVES, PHASE_TWO_MOVES + 10));
        }
);

Database<MoveTable> PHASE_2_UD_SLICE_MOVE_TABLE(
        sizeof(MoveTable) * 24,
        "data/ud_slice_moves.bin",
        [](MoveTable* out) {
            constructMoveTable(out, phase2UDSliceCoordinate, std::vector<int>(PHASE_TWO_MOVES, PHASE_TWO_MOVES + 10));
        }
);

Database<SymmetryTable> PHASE_2_UD_SLICE_SYMMETRY_TABLE(
        sizeof(SymmetryTable) * 24,
        "data/ud_slice_symmetry.bin",
        [](SymmetryTable* out) {
            constructSymmetryTable(out, phase2UDSliceCoordinate, std::vector<int>(PHASE_TWO_MOVES, PHASE_TWO_MOVES + 10));
        }
);

struct PhaseOnePruningTable {
    uint8_t lookup[140908410];
};

struct SuperFastPhaseOneCube {
    uint32_t flipUDSlice;
    uint32_t cornerTwist;

    SuperFastPhaseOneCube() {}

    SuperFastPhaseOneCube(const FastRubiksCube& cube) {
        flipUDSlice = FLIP_UD_SLICE_SYM_COORDS.ptr->rawCoordToSymCoord[flipUDSliceCoordinate(cube)];
        cornerTwist = positionalCornerOrientationCoordinate(cube);
    }

    SuperFastPhaseOneCube(uint32_t flipUDSlice, uint32_t cornerTwist) : flipUDSlice(flipUDSlice), cornerTwist(cornerTwist) {}

    SuperFastPhaseOneCube doMove(int move) const {
        uint32_t newFlipUDSlice = FLIP_UD_SLICE_SYM_MOVE_TABLE.ptr->doMove(flipUDSlice, move);
        uint32_t newCornerTwist = CORNER_TWIST_MOVE_TABLE.ptr[cornerTwist].moves[move];

        return SuperFastPhaseOneCube(newFlipUDSlice, newCornerTwist);
    }

    uint32_t getPruningCoord() const {
        uint8_t sym = flipUDSlice & 0xF;
        uint32_t flipUDSliceCoord = flipUDSlice >> 4;

        return CORNER_TWIST_SYMMETRY_TABLE.ptr[cornerTwist].lookup[REVERSE_SYMMETRIES[sym]] + flipUDSliceCoord * 2187;
    }
};

struct PhaseTwoPruningTable {
    uint8_t lookup[111605760];
};

struct SuperFastPhaseTwoCube {
    uint32_t cornerPerm;
    uint32_t edgePerm;
    uint32_t udSlice;

    SuperFastPhaseTwoCube() {}

    SuperFastPhaseTwoCube(const FastRubiksCube& cube) {
        cornerPerm = CORNER_PERM_SYM_COORDS.ptr->rawCoordToSymCoord[cornerPermutationCoordinate(cube)];
        edgePerm = phase2EdgePermutationCoordinate(cube);
        udSlice = phase2UDSliceCoordinate(cube);
    }

    SuperFastPhaseTwoCube(uint32_t cornerPerm, uint32_t edgePerm, uint32_t udSlice) : cornerPerm(cornerPerm), edgePerm(edgePerm), udSlice(udSlice) {}

    SuperFastPhaseTwoCube doMove(int move) const {
        uint32_t newCornerPerm = CORNER_PERM_SYM_MOVE_TABLE.ptr->doMove(cornerPerm, move);
        uint32_t newEdgePerm = PHASE_2_EDGE_PERM_MOVE_TABLE.ptr[edgePerm].moves[move];
        uint32_t newUDSlice = PHASE_2_UD_SLICE_MOVE_TABLE.ptr[udSlice].moves[move];

        return SuperFastPhaseTwoCube(newCornerPerm, newEdgePerm, newUDSlice);
    }

    uint32_t getPruningCoord() const {
        uint8_t sym = cornerPerm & 0xF;
        uint32_t cornerPermCoord = cornerPerm >> 4;

        return PHASE_2_EDGE_PERM_SYMMETRY_TABLE.ptr[edgePerm].lookup[REVERSE_SYMMETRIES[sym]] + cornerPermCoord * 40320;
    }
};

void constructPhaseOnePruningTable(PhaseOnePruningTable* out) {
    performBFSDisk<SuperFastPhaseOneCube>(
            SuperFastPhaseOneCube(FastRubiksCube()),
            sizeof(PhaseOnePruningTable) / sizeof(uint8_t),
            [](const SuperFastPhaseOneCube& state) {
                return state.getPruningCoord();
            },
            [](const SuperFastPhaseOneCube& state) {
                std::vector<SuperFastPhaseOneCube> res;
                for (int i = 0; i < 18; i++) {
                    res.push_back(state.doMove(i));
                }
                return res;
            },
            [&](uint8_t depth, uint64_t idx, const SuperFastPhaseOneCube& state) {
                out->lookup[idx] = depth;
            }
    );
}

void constructPhaseTwoPruningTable(PhaseTwoPruningTable* out) {
    performBFSDisk<SuperFastPhaseTwoCube>(
            SuperFastPhaseTwoCube(FastRubiksCube()),
            sizeof(PhaseTwoPruningTable) / sizeof(uint8_t),
            [](const SuperFastPhaseTwoCube& state) {
                return state.getPruningCoord();
            },
            [](const SuperFastPhaseTwoCube& state) {
                std::vector<SuperFastPhaseTwoCube> res;
                for (int i : PHASE_TWO_MOVES) {
                    res.push_back(state.doMove(i));
                }
                return res;
            },
            [&](uint8_t depth, uint64_t idx, const SuperFastPhaseTwoCube& state) {
                out->lookup[idx] = depth;
            }
    );
}

Database<PhaseOnePruningTable> PHASE_ONE_PRUNING_TABLE(
        sizeof(PhaseOnePruningTable),
        "data/phase_one_pruning.bin",
        constructPhaseOnePruningTable
);

Database<PhaseTwoPruningTable> PHASE_TWO_PRUNING_TABLE(
        sizeof(PhaseTwoPruningTable),
        "data/phase_two_pruning.bin",
        constructPhaseTwoPruningTable
);

FastRubiksCube genRandomCube() {
    FastRubiksCube cube;

    for (int i = 0; i < 100; i++) {
        cube = cube.doMove(rand() % 18);
    }

    return cube;
}

void kociembaInit() {
    for (int i = 0; i < 16; i++) {
        std::set<int> possibleInverses;
        for (int j = 0; j < 16; j++) possibleInverses.insert(j);

        while (possibleInverses.size() > 1) {
            FastRubiksCube orig = genRandomCube();
            FastRubiksCube res = applySymmetry(orig, i);

            std::set<int> newFrontier;
            for (int j : possibleInverses) {
                if (applySymmetry(res, j) == orig) {
                    newFrontier.insert(j);
                }
            }

            possibleInverses = newFrontier;
        }

        if (possibleInverses.size() != 1) {
            std::cout << "Error: no inverse for symmetry " << i << std::endl;
            exit(1);
        }

        REVERSE_SYMMETRIES[i] = *possibleInverses.begin();
    }

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            std::set<int> possibleMult;

            for (int k = 0; k < 16; k++) {
                possibleMult.insert(k);
            }

            while (possibleMult.size() > 1) {
                FastRubiksCube orig = genRandomCube();
                FastRubiksCube res = applySymmetry(orig, i);
                res = applySymmetry(res, j);

                std::set<int> newFrontier;
                for (int k : possibleMult) {
                    if (applySymmetry(orig, k) == res) {
                        newFrontier.insert(k);
                    }
                }

                possibleMult = newFrontier;
            }

            if (possibleMult.size() != 1) {
                std::cout << "Error: no mult for symmetries " << i << " and " << j << std::endl;
                exit(1);
            }

            SYM_MULT[i][j] = *possibleMult.begin();
        }
    }

    FLIP_UD_SLICE_SYM_COORDS.ensureLoaded();
    CORNER_TWIST_MOVE_TABLE.ensureLoaded();
    FLIP_UD_SLICE_SYM_MOVE_TABLE.ensureLoaded();
    CORNER_TWIST_SYMMETRY_TABLE.ensureLoaded();
    PHASE_ONE_PRUNING_TABLE.ensureLoaded();

    CORNER_PERM_MOVE_TABLE.ensureLoaded();
    CORNER_PERM_SYMMETRY_TABLE.ensureLoaded();
    PHASE_2_EDGE_PERM_MOVE_TABLE.ensureLoaded();
    PHASE_2_EDGE_PERM_SYMMETRY_TABLE.ensureLoaded();
    PHASE_2_UD_SLICE_MOVE_TABLE.ensureLoaded();
    PHASE_2_UD_SLICE_SYMMETRY_TABLE.ensureLoaded();

    CORNER_PERM_SYM_COORDS.ensureLoaded();
    CORNER_PERM_SYM_MOVE_TABLE.ensureLoaded();

    PHASE_TWO_PRUNING_TABLE.ensureLoaded();

    return; // We don't need the tests

    //Test SuperFastPhaseOneCube

    FastRubiksCube ref;
    SuperFastPhaseOneCube cube(ref);

    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < 100000; i++) {
        int move = gen() % 18;
        cube = cube.doMove(move);
        ref = ref.doMove(move);

        SuperFastPhaseOneCube cube2(ref);

        if (cube.flipUDSlice / 16 != cube2.flipUDSlice / 16 || cube.cornerTwist != cube2.cornerTwist) {
            std::cout << "Error: SuperFastPhaseOneCube is broken" << std::endl;
            std::cout << "Executed " << i + 1 << " moves" << std::endl;
            exit(1);
        }
    }

    /*bool halt = false;
    auto intoPhaseTwo = kociembaSolve(ref, halt);
    for (Move move : *intoPhaseTwo) {
        int idx = -1;
        for (int i = 0; i < 18; i++) {
            if (move.side == ALL_MOVES[i].side && move.moveType == ALL_MOVES[i].moveType) {
                idx = i;
                break;
            }
        }

        if (idx == -1) {
            std::cout << "Error: invalid move in solution" << std::endl;
            exit(1);
        }

        ref = ref.doMove(idx);
    }

    SuperFastPhaseTwoCube cube2(ref);

    for (int i = 0; i < 100000; i++) {
        int move = gen() % 10;
        move = PHASE_TWO_MOVES[move];
        cube2 = cube2.doMove(move);
        ref = ref.doMove(move);

        SuperFastPhaseTwoCube cube3(ref);

        if (cube2.cornerPerm / 16 != cube3.cornerPerm / 16 || cube2.edgePerm != cube3.edgePerm || cube2.udSlice != cube3.udSlice) {
            std::cout << "Error: SuperFastPhaseTwoCube is broken" << std::endl;
            std::cout << "Executed " << i + 1 << " moves" << std::endl;
            exit(1);
        }
    }*/

    //Test the speed!!
    std::cout << "Testing speed..." << std::endl;
    const int MOVE_COUNT = 10000000;

    //Prepare move sequence
    std::vector<int> moves(MOVE_COUNT);
    for (int i = 0; i < MOVE_COUNT; i++) {
        moves[i] = gen() % 18;
    }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    for (int i = 0; i < MOVE_COUNT; i++) {
        cube = cube.doMove(moves[i]);
    }

    end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "Performed " << MOVE_COUNT << " moves in " << elapsed_seconds.count() << " seconds" << std::endl;
    std::cout << "That's " << (MOVE_COUNT / elapsed_seconds.count()) << " moves per second" << std::endl;

    double nanosPerMove = (elapsed_seconds.count() * 1000000000) / MOVE_COUNT;
    std::cout << "That's " << nanosPerMove << " nanoseconds per move" << std::endl;

    std::cout << cube.flipUDSlice << " " << cube.cornerTwist << std::endl;

    //Test validity of SuperFastPhaseOneCube
    std::cout << "Testing validity of SuperFastPhaseOneCube..." << std::endl;

    {
        FastRubiksCube ref;
        SuperFastPhaseOneCube cube(ref);

        for (int i = 0; i < 100000; i++) {
            int move = gen() % 18;
            cube = cube.doMove(move);
            ref = ref.doMove(move);

            int sym = cube.flipUDSlice % 16;
            FastRubiksCube ref2 = applySymmetry(ref, REVERSE_SYMMETRIES[sym]);

            uint32_t symCoord = FLIP_UD_SLICE_SYM_COORDS.ptr->rawCoordToSymCoord[flipUDSliceCoordinate(ref2)];
            if (symCoord % 16 != 0) {
                std::cout << "Error: Symmetry reduction doesn't result in representant" << std::endl;
                std::cout << "Executed " << i + 1 << " moves" << std::endl;
                exit(1);
            }

            uint32_t lookupCoord = (symCoord / 16) * 2187 + positionalCornerOrientationCoordinate(ref2);

            if (cube.getPruningCoord() != lookupCoord) {
                std::cout << "Error: Symmetry reduction doesn't result in correct coordinate" << std::endl;
                std::cout << "Executed " << i + 1 << " moves" << std::endl;
                exit(1);
            }
        }
    }

    //collectData();
}

void solvePhaseOneAtDepth(SuperFastPhaseOneCube& cube, RedundantMovePreventor rmp, int depth, std::vector<int>& out, bool& halt, const std::function<void(std::vector<int>&)>& callback) {
    uint8_t dist = PHASE_ONE_PRUNING_TABLE.ptr->lookup[cube.getPruningCoord()];

    if (halt) return;

    if (dist == 0) {
        if (cube.flipUDSlice / 16 == 0 && cube.cornerTwist == 0) {
            callback(out);
            return;
        }
    }

    if (dist > depth) {
        return;
    }

    for (int i = 0; i < 18; i++) {
        if (rmp.isRedundant(ALL_MOVES[i])) {
            continue;
        }

        SuperFastPhaseOneCube next = cube.doMove(i);
        RedundantMovePreventor nextRMP = rmp;
        nextRMP.turnFace(ALL_MOVES[i].side);

        out.push_back(i);
        solvePhaseOneAtDepth(next, nextRMP, depth - 1, out, halt, callback);
        out.pop_back();
    }
}

bool solvePhaseTwoAtDepth(SuperFastPhaseTwoCube& cube, RedundantMovePreventor rmp, int depth, std::vector<int>& out, bool& halt) {
    uint8_t dist = PHASE_TWO_PRUNING_TABLE.ptr->lookup[cube.getPruningCoord()];

    if (halt) return true;

    if (dist == 0) {
        if (cube.cornerPerm / 16 == 0 && cube.edgePerm == 0 && cube.udSlice == 0) {
            return true;
        }
    }

    if (dist > depth) {
        return false;
    }

    for (int i: PHASE_TWO_MOVES) {
        if (rmp.isRedundant(ALL_MOVES[i])) {
            continue;
        }

        SuperFastPhaseTwoCube next = cube.doMove(i);
        RedundantMovePreventor nextRMP = rmp;
        nextRMP.turnFace(ALL_MOVES[i].side);

        if (solvePhaseTwoAtDepth(next, nextRMP, depth - 1, out, halt)) {
            out.push_back(i);
            return true;
        }
    }

    return false;
}

std::optional<std::vector<int>> solvePhaseTwo(SuperFastPhaseTwoCube& cube, bool& halt, int maxMoves = 18) {
    int lowerBound = PHASE_TWO_PRUNING_TABLE.ptr->lookup[cube.getPruningCoord()];
    std::vector<int> out;

    for (int i = lowerBound; i <= maxMoves; i++) {
        //std::cout << "Trying to solve cube in " << i << " moves!" << std::endl;
        if (solvePhaseTwoAtDepth(cube, RedundantMovePreventor(), i, out, halt)) {
            std::reverse(out.begin(), out.end());

            return out;
        }
    }

    return std::nullopt;
}

std::optional<std::vector<Move>> kociembaSolve(FastRubiksCube cube, bool& halt) {
    FLIP_UD_SLICE_SYM_COORDS.ensureLoaded();
    CORNER_TWIST_MOVE_TABLE.ensureLoaded();
    FLIP_UD_SLICE_SYM_MOVE_TABLE.ensureLoaded();
    CORNER_TWIST_SYMMETRY_TABLE.ensureLoaded();
    PHASE_ONE_PRUNING_TABLE.ensureLoaded();

    CORNER_PERM_MOVE_TABLE.ensureLoaded();
    CORNER_PERM_SYMMETRY_TABLE.ensureLoaded();
    PHASE_2_EDGE_PERM_MOVE_TABLE.ensureLoaded();
    PHASE_2_EDGE_PERM_SYMMETRY_TABLE.ensureLoaded();
    PHASE_2_UD_SLICE_MOVE_TABLE.ensureLoaded();
    PHASE_2_UD_SLICE_SYMMETRY_TABLE.ensureLoaded();

    CORNER_PERM_SYM_COORDS.ensureLoaded();
    CORNER_PERM_SYM_MOVE_TABLE.ensureLoaded();

    PHASE_TWO_PRUNING_TABLE.ensureLoaded();

    SuperFastPhaseOneCube phaseOneCube(cube);
    int lowerBound = PHASE_ONE_PRUNING_TABLE.ptr->lookup[phaseOneCube.getPruningCoord()];

    int bestTotalLength = 1000;
    std::vector<int> bestMoves;

    for (int numPhaseOneMoves = lowerBound; numPhaseOneMoves < bestTotalLength && !halt; numPhaseOneMoves++) {
        //std::cout << "Trying to solve cube in " << numPhaseOneMoves << " moves!" << std::endl;
        std::vector<int> phaseOneMoves;
        solvePhaseOneAtDepth(phaseOneCube, RedundantMovePreventor(), numPhaseOneMoves, phaseOneMoves, halt, [&](std::vector<int>& moves) {

            FastRubiksCube copy = cube;
            for (int move: moves) {
                copy = copy.doMove(move);
            }

            SuperFastPhaseTwoCube phaseTwoCube(copy);

            std::optional<std::vector<int>> phaseTwoMoves = solvePhaseTwo(phaseTwoCube, halt, bestTotalLength - numPhaseOneMoves - 1);

            if (phaseTwoMoves) {
                bestMoves = moves;
                //std::cout << "Found phase two solution with " << phaseTwoMoves->size() << " moves!" << std::endl;
                bestMoves.insert(bestMoves.end(), phaseTwoMoves->begin(), phaseTwoMoves->end());
                bestTotalLength = numPhaseOneMoves + phaseTwoMoves->size();
                //std::cout << "Best total length is now " << bestTotalLength << std::endl;
            }
        });
    }

    if (bestMoves.empty()) {
        return std::nullopt;
    }

    std::vector<Move> out;
    for (int move: bestMoves) {
        out.push_back(ALL_MOVES[move]);
    }

    std::cout << "Final solution has " << out.size() << " moves!" << std::endl;

    return out;
}

void collectData() {
    /*std::ifstream in("kociemba.csv");

    if (!in) {
        std::ofstream out("kociemba.csv");
        out << "Num Moves,Solve" << std::endl;

        out.close();
    }

    in.close();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 17);

    //Open kociemba.csv to append to it
    while (true) {
        FastRubiksCube cube;

        for (int i = 0; i < 10000; i++) {
            int move = dis(gen);
            cube = cube.doMove(move);
        }

        //Launch kociembaSolve in a separate thread
        bool halt = false;
        std::thread t([&]() {
            auto solution = kociembaSolve(cube, halt);

            //Open kociemba.csv to append to it
            std::ofstream out("kociemba.csv", std::ios::app);

            if (solution) {
                out << solution->size() << ",";

                for (Move move: *solution) {
                    out << move.moveCode() << " ";
                }

                out << std::endl;
            } else {
                out << "," << std::endl;
            }

            out.close();

            std::cout << "Solved random cube in " << solution->size() << " moves!" << std::endl;
        });

        //Wait for 15 seconds
        std::this_thread::sleep_for(std::chrono::seconds(15));

        halt = true;

        t.join();
    }*/

    const int NUM_SOLVES = 100;
    auto allowed_duration = std::chrono::milliseconds(10);

    std::vector<FastRubiksCube> cubes;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 17);

    for (int i = 0; i < NUM_SOLVES; i++) {
        FastRubiksCube cube;

        for (int j = 0; j < 100000; j++) {
            int move = dis(gen);
            cube = cube.doMove(move);
        }

        cubes.push_back(cube);
    }

    std::cout << "Finished generating " << NUM_SOLVES << " random cubes!" << std::endl;
    std::vector<int> numMoves;

    for (int i = 0; i < NUM_SOLVES; i++) {
        bool halt = false;
        auto start = std::chrono::high_resolution_clock::now();
        std::thread t([&]() {
            auto sol = kociembaSolve(cubes[i], halt);
            if (sol) {
                numMoves.push_back(sol->size());
            } else {
                numMoves.push_back(-1);
            }
        });

        std::this_thread::sleep_for(allowed_duration);

        halt = true;
        t.join();

        std::cout << "Solved cube " << i << " in " << numMoves.back() << " moves!" << std::endl;
    }

    std::vector<int> histogram(50);

    for (int numMove: numMoves) {
        if (numMove != -1) {
            histogram[numMove]++;
        } else {
            std::cout << "Failed to solve cube!" << std::endl;
        }
    }

    for (int i = 0; i < histogram.size(); i++) {
        if (histogram[i])
            std::cout << i << ": " << histogram[i] << std::endl;
    }
}