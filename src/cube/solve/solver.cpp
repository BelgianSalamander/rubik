#include "solver.h"
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <chrono>
#include <utility>
#include <vector>
#include <functional>
#include <algorithm>
#include "util/RedundantMovePreventor.h"
#include "database.h"
#include "kociemba.h"
#include "solver_util.h"

constexpr uint64_t fact(uint64_t n) noexcept {
    return n == 0 ? 1 : n * fact(n - 1);
}

constexpr uint64_t bpow(uint64_t base, uint64_t exp) noexcept {
    if (exp == 0) {
        return 1;
    }

    uint64_t result = bpow(base, exp / 2);

    result *= result;

    if (exp % 2 == 1) {
        result *= base;
    }

    return result;
}




template<typename KeyGetter>
void genDataDisk(KeyGetter keyGetter, const std::function<void(uint64_t, uint8_t)>& setter, uint64_t size) {
    auto startTime = std::chrono::high_resolution_clock::now();

    struct CachedCube {
        FastRubiksCube cube;
        uint64_t idx;

        CachedCube() {}
        CachedCube(FastRubiksCube cube, uint64_t idx) : cube(cube), idx(idx) {}
    };

    bool* queued = new bool[size];
    memset(queued, 0, size);

    int depth = 0;
    std::string frontierPath = "data/tmp/frontier-0.bin";

    std::filesystem::path frontierPathObject = frontierPath;
    std::filesystem::create_directories(frontierPathObject.parent_path());

    std::ofstream frontierOut(frontierPath, std::ios::binary);
    FastRubiksCube startCube;
    CachedCube start(startCube, keyGetter(startCube));
    frontierOut.write((char*) &start, sizeof(CachedCube));
    frontierOut.close();
    queued[start.idx] = true;
    int frontierSize = 1;

    const int BUFFER_SIZE = 10000000;
    char* outputBuffer = new char[BUFFER_SIZE * sizeof(CachedCube)];
    char* inputBuffer = new char[BUFFER_SIZE * sizeof(CachedCube)];

    int processed = 0;

    while (frontierSize) {
        std::string nextFrontierPath = "data/tmp/frontier-" + std::to_string(depth + 1) + ".bin";
        std::ofstream nextFrontierOut(nextFrontierPath, std::ios::binary);
        int nextFrontierSize = 0;

        std::ifstream frontierIn(frontierPath, std::ios::binary);

        int outputBufferPos = 0;

        int inputBufferPos = 0;
        int inputBufferCount = 0;

        auto currDepthStart = std::chrono::high_resolution_clock::now();
        std::cout << "Processing depth " << depth << " with " << frontierSize << " cubes" << std::endl;
        for (int i = 0; i < frontierSize; i++) {
            if (inputBufferPos >= inputBufferCount) {
                frontierIn.read(inputBuffer, BUFFER_SIZE * sizeof(CachedCube));
                inputBufferCount = frontierIn.gcount() / sizeof(CachedCube);
                inputBufferPos = 0;
            }

            CachedCube cube;
            memcpy(&cube, inputBuffer + inputBufferPos * sizeof(CachedCube), sizeof(CachedCube));

            inputBufferPos++;

            uint64_t index = cube.idx;
            setter(index, depth);

            processed++;

            if (processed % 1000000 == 0) {
                std::cout << "Global progress:" << std::endl;
                progressBar(processed, size, startTime);
                std::cout << "Depth progress (depth " << depth << "):" << std::endl;
                progressBar(i, frontierSize, currDepthStart);
            }

            for (int j = 0; j < 18; j++) {
                FastRubiksCube nextCube = cube.cube.doMove(j);
                CachedCube next(nextCube, keyGetter(nextCube));

                if (!queued[next.idx]) {
                    queued[next.idx] = true;

                    if (outputBufferPos >= BUFFER_SIZE) {
                        nextFrontierOut.write(outputBuffer, BUFFER_SIZE * sizeof(CachedCube));
                        outputBufferPos = 0;
                    }

                    memcpy(outputBuffer + outputBufferPos * sizeof(CachedCube), &next, sizeof(CachedCube));
                    outputBufferPos++;
                    nextFrontierSize++;
                }
            }
        }

        nextFrontierOut.write(outputBuffer, outputBufferPos * sizeof(CachedCube));
        nextFrontierOut.close();
        frontierIn.close();

        frontierPath = nextFrontierPath;
        frontierSize = nextFrontierSize;
        depth++;

        std::cout << "Finished processing depth " << depth - 1 << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - currDepthStart).count() << " ms" << std::endl;
    }

    delete[] queued;
    delete[] outputBuffer;
    delete[] inputBuffer;

    std::cout << "Final depth: " << depth - 1 << std::endl;

    //Delete all tmp files
    for (int i = 0; i <= depth; i++) {
        std::string path = "data/tmp/frontier-" + std::to_string(i) + ".bin";
        remove(path.c_str());
    }
}

std::function<void(uint64_t, uint8_t)> basicSetter(uint8_t* dst) {
    return [dst](uint64_t index, uint8_t depth) {
        dst[index] = depth;
    };
}

std::function<void(uint64_t, uint8_t)> interspersedSetter(uint8_t* dst) {
    return [dst](uint64_t index, uint8_t depth) {
        if (index & 1) {
            dst[index / 2] = (depth << 4) | (dst[index / 2] & 0x0F);
        } else {
            dst[index / 2] = (dst[index / 2] & 0xF0) | depth;
        }
    };
}

uint8_t getInterspersedValue(uint8_t* dst, uint64_t index) {
    if (index & 1) {
        return dst[index / 2] >> 4;
    } else {
        return dst[index / 2] & 0xF;
    }
}

template<size_t GroupSize>
struct PartialEdgeKeyGetter {
    std::array<Edge, GroupSize> group;

    PartialEdgeKeyGetter(std::array<Edge, GroupSize> group) : group(group) {}

    inline uint64_t operator()(FastRubiksCube& cube) {
        return cube.getPartialEdgeIndex(group);
    }
};

const uint64_t NUM_CORNER_INDICES = fact(8) * bpow(3, 7);
std::string FULL_CORNERS_PATH = "data/corners.bin";

const uint64_t NUM_PARTIAL_EDGE_INDICES = fact(12) / fact(5) * bpow(2, 7);
std::string PARTIAL_EDGES_GROUP_1_PATH = "data/partial_edges_group_1.bin";
std::string PARTIAL_EDGES_GROUP_2_PATH = "data/partial_edges_group_2.bin";

const uint64_t NUM_EDGE_PERM_INDICES = fact(12);
std::string EDGE_PERMS_PATH = "data/edge_perms.bin";

const uint64_t NUM_EDGE_CROSS_INDICES = fact(12) / fact(8) * bpow(2, 4);
std::string EDGE_CROSS_ONE_PATH = "data/edge_cross_one.bin";

Database<uint8_t> LOWER_BOUND_CORNERS(NUM_CORNER_INDICES, "data/corners.bin", [](uint8_t* out) {
    struct KeyGetter {
        inline uint32_t operator()(FastRubiksCube& cube) {
            return cube.getCornerIndex();
        }
    } keyGetter;

    genDataDisk(keyGetter, basicSetter(out), NUM_CORNER_INDICES);
});

Database<uint8_t> LOWER_BOUND_PARTIAL_EDGES_GROUP_1(NUM_PARTIAL_EDGE_INDICES, PARTIAL_EDGES_GROUP_1_PATH, [](uint8_t* out) {
    genDataDisk(PartialEdgeKeyGetter(EDGE_GROUP_ONE), interspersedSetter(out), NUM_PARTIAL_EDGE_INDICES);
});
Database<uint8_t> LOWER_BOUND_PARTIAL_EDGES_GROUP_2(NUM_PARTIAL_EDGE_INDICES, PARTIAL_EDGES_GROUP_2_PATH, [](uint8_t* out) {
    genDataDisk(PartialEdgeKeyGetter(EDGE_GROUP_TWO), interspersedSetter(out), NUM_PARTIAL_EDGE_INDICES);
});

Database<uint8_t> LOWER_BOUND_EDGE_PERMS(NUM_EDGE_PERM_INDICES, EDGE_PERMS_PATH, [](uint8_t* out) {
    struct KeyGetter {
        inline uint32_t operator()(FastRubiksCube& cube) {
            return cube.getEdgePermutationIndex();
        }
    } keyGetter;

    genDataDisk(keyGetter, basicSetter(out), NUM_EDGE_PERM_INDICES);
});

Database<uint8_t> LOWER_BOUND_EDGE_CROSS_ONE(NUM_EDGE_CROSS_INDICES, EDGE_CROSS_ONE_PATH, [](uint8_t* out) {
    genDataDisk(PartialEdgeKeyGetter(EDGE_GROUP_CROSS_ONE), interspersedSetter(out), NUM_PARTIAL_EDGE_INDICES / 2);
});

void initSolver() {
    kociembaInit();
    std::cout << "Kociemba initialized" << std::endl;

    std::cout << "Preloading Korf Tables!" << std::endl;
    LOWER_BOUND_CORNERS.ensureLoaded();
    LOWER_BOUND_PARTIAL_EDGES_GROUP_1.ensureLoaded();
    LOWER_BOUND_PARTIAL_EDGES_GROUP_2.ensureLoaded();
    LOWER_BOUND_EDGE_PERMS.ensureLoaded();
}

template<typename IsSolvedFunc, typename HeuristicFunc, int MoveCount>
bool solveAtDepth(FastRubiksCube& cube, RedundantMovePreventor rmp, int depth, std::vector<int>& out, IsSolvedFunc& isSolvedFunc, HeuristicFunc heuristicFunc, std::array<int, MoveCount>& moves, bool& halt) {
    uint8_t dist = heuristicFunc(cube);

    if (halt) return true;

    if (dist == 0) {
        if (isSolvedFunc(cube)) {
            return true;
        }
    }

    if (dist > depth) {
        return false;
    }

    for (int i = 0; i < MoveCount; i++) {
        if (rmp.isRedundant(ALL_MOVES[moves[i]])) {
            continue;
        }

        FastRubiksCube next = cube.doMove(moves[i]);
        RedundantMovePreventor nextRMP = rmp;
        nextRMP.turnFace(ALL_MOVES[moves[i]].side);

        if (solveAtDepth<IsSolvedFunc, HeuristicFunc, MoveCount>(next, nextRMP, depth - 1, out, isSolvedFunc, heuristicFunc, moves, halt)) {
            out.push_back(moves[i]);
            return true;
        }
    }

    return false;
}

template<typename IsSolvedFunc, typename HeuristicFunc, int MoveCount>
std::optional<std::vector<int>> solveIDAStar(FastRubiksCube cube, IsSolvedFunc isSolvedFunc, HeuristicFunc heuristicFunc, std::array<int, MoveCount> moves, bool& halt, int maxDepth = 20) {
    std::vector<int> out;

    for (int i = 0; i <= maxDepth && !halt; i++) {
        std::cout << "Trying to solve cube in " << i << " moves!" << std::endl;
        if (solveAtDepth<IsSolvedFunc, HeuristicFunc, MoveCount>(cube, RedundantMovePreventor(), i, out, isSolvedFunc, heuristicFunc, moves, halt)) {
            std::reverse(out.begin(), out.end());

            return out;
        }
    }

    return std::nullopt;
}

const std::array<int, 18> ALL_MOVES_ARR = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
template<typename IsSolvedFunc, typename HeuristicFunc>
std::optional<std::vector<int>> solveIDAStarAllMoves(FastRubiksCube cube, IsSolvedFunc isSolvedFunc, HeuristicFunc heuristicFunc, bool& halt, int maxMoves = 20) {
    return solveIDAStar<IsSolvedFunc, HeuristicFunc, 18>(cube, isSolvedFunc, heuristicFunc, ALL_MOVES_ARR, halt, maxMoves);
}

std::optional<std::vector<int>> solveKorf(FastRubiksCube cube, bool& halt, int maxMoves = 20) {
    LOWER_BOUND_CORNERS.ensureLoaded();
    LOWER_BOUND_PARTIAL_EDGES_GROUP_1.ensureLoaded();
    LOWER_BOUND_PARTIAL_EDGES_GROUP_2.ensureLoaded();
    LOWER_BOUND_EDGE_PERMS.ensureLoaded();

    auto isSolvedFunc = [](FastRubiksCube& cube) {
        return cube.isSolved();
    };

    auto heuristicFunc = [](FastRubiksCube& cube) {
        uint8_t res = LOWER_BOUND_CORNERS.ptr[cube.getCornerIndex()];
        res = std::max(
                res,
                getInterspersedValue(LOWER_BOUND_PARTIAL_EDGES_GROUP_1.ptr, cube.getPartialEdgeIndex(EDGE_GROUP_ONE))
        );
        res = std::max(
                res,
                getInterspersedValue(LOWER_BOUND_PARTIAL_EDGES_GROUP_2.ptr, cube.getPartialEdgeIndex(EDGE_GROUP_TWO))
        );
        res = std::max(
                res,
                getInterspersedValue(LOWER_BOUND_EDGE_PERMS.ptr, cube.getEdgePermutationIndex())
        );

        return res;
    };

    return solveIDAStarAllMoves(cube, isSolvedFunc, heuristicFunc, halt, maxMoves);
}

std::optional<std::vector<int>> solveCFOP(FastRubiksCube cube, bool& halt) {
    std::vector<int> res;

    LOWER_BOUND_EDGE_CROSS_ONE.ensureLoaded();

    auto isCrossSolvedFunc = [](FastRubiksCube& cube) {
        return cube.isPartiallySolved(EDGE_GROUP_CROSS_ONE);
    };

    auto crossHeuristicFunc = [](FastRubiksCube& cube) {
        return getInterspersedValue(LOWER_BOUND_EDGE_CROSS_ONE.ptr, cube.getPartialEdgeIndex(EDGE_GROUP_CROSS_ONE));
    };

    auto crossMoves = solveIDAStarAllMoves(cube, isCrossSolvedFunc, crossHeuristicFunc, halt);

    if (!crossMoves) {
        return std::nullopt;
    }

    for (int move : *crossMoves) {
        cube = cube.doMove(move);
        res.push_back(move);
    }

    std::vector<std::pair<Edge, Corner>> cornerOrder = {
            {Edge::FRONT_LEFT, Corner::TOP_LEFT_FRONT},
            {Edge::FRONT_RIGHT, Corner::TOP_RIGHT_FRONT},
            {Edge::BACK_RIGHT, Corner::TOP_RIGHT_BACK},
            {Edge::BACK_LEFT, Corner::TOP_LEFT_BACK},
    };

    std::vector<std::vector<int>> allPerms;
    std::vector<int> p = {0, 1, 2, 3};
    do {
        std::vector<Edge> toSolveEdges(EDGE_GROUP_CROSS_ONE.begin(), EDGE_GROUP_CROSS_ONE.end());
        std::vector<Corner> toSolveCorners;

        std::vector<int> currMoves;
        FastRubiksCube currCube = cube;

        for (int i = 0; i < 4; i++) {
            if (halt) return std::nullopt;
            Corner corner = cornerOrder[p[i]].second;
            Edge edge = cornerOrder[p[i]].first;

            toSolveCorners.push_back(corner);
            toSolveEdges.push_back(edge);

            auto f = [&toSolveEdges, &toSolveCorners](FastRubiksCube& cube) {
                return cube.isPartiallySolved(toSolveEdges) && cube.isPartiallySolved(toSolveCorners);
            };

            auto moves = solveIDAStarAllMoves(currCube, f, crossHeuristicFunc, halt);


            if (!moves) {
                return std::nullopt;
            }

            for (int move : *moves) {
                currMoves.push_back(move);
                currCube = currCube.doMove(move);
            }
        }

        allPerms.push_back(currMoves);
    } while (std::next_permutation(p.begin(), p.end()));

    for (int i = 0; i <= 14; i++) {
        for (int j = 0; j < allPerms.size(); j++) {
            if (halt) return std::nullopt;

            FastRubiksCube currCube = cube;

            for (int move : allPerms[j]) {
                currCube = currCube.doMove(move);
            }

            auto moves = solveKorf(currCube, halt, i);

            if (moves) {
                for (int move: allPerms[j]) {
                    res.push_back(move);
                }

                for (int move : *moves) {
                    res.push_back(move);
                }

                return res;
            }
        }
    }

    return std::nullopt;
}

std::optional<std::vector<Move>> solve(FastRubiksCube cube, bool& halt) {
    //return kociembaSolve(cube, halt);

    auto res = solveCFOP(cube, halt);

    if (!res) {
        return std::nullopt;
    }

    std::vector<Move> out;
    for (int move : *res) {
        out.push_back(ALL_MOVES[move]);
    }

    return out;
}