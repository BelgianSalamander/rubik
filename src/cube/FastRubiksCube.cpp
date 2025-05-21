//
// Created by Anatol on 29/12/2022.
//

#include "FastRubiksCube.h"
#include <iostream>
#include <random>
#include <chrono>
#include <array>
#include <stdint.h>
#include "solve/solver.h"

FastRubiksCube FAST_MOVES[18];
unsigned int FACTORIAL_U32[13];
unsigned long long FACTORIAL_U64[21];
unsigned int NPR_U32[13][13];
unsigned int NCR_U32[13][13];

uint8_t BIT_COUNT_U16[65536];

void testMoves() {
    RubiksCube cube;
    FastRubiksCube fastCube(cube);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 17);

    const int MOVES = 1000000;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < MOVES; i++) {
        int move = dis(gen);
        cube.doMove(ALL_MOVES[move]);
        fastCube = FAST_MOVES[move].copyAndApplyTo(fastCube);
    }
    auto end = std::chrono::high_resolution_clock::now();

    if (cube != fastCube.toRubiksCube()) {
        FastRubiksCube fastCube2(cube);
        std::cerr << "ERROR: Fast moves don't work!" << std::endl;

        std::cerr << "Expected: " << std::endl;
        fastCube2.print();
        std::cerr << "Actual: " << std::endl;
        fastCube.print();
    }

    for (int i = 0; i < 1000; i++) {
        RubiksCube cube;
        for (int j = 0; j < 20; j++) {
            int move = dis(gen);
            cube.doMove(ALL_MOVES[move]);
        }

        FastRubiksCube fastCube(cube);

        if (cube != fastCube.toRubiksCube()) {
            std::cerr << "ERROR: Cube translation doesn't work!" << std::endl;
        }
    }

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Time: " << milliseconds << "ms" << std::endl;
    std::cout << "Time per move: " << milliseconds / (double) MOVES * 1000.0 << "us" << std::endl;
    std::cout << "Moves per second: " << MOVES / (milliseconds / 1000.0) << std::endl;

    for (int i = 0; i < 10000; i++) {
        FastRubiksCube cube;
        for (int j = 0; j < 100; j++) {
            int move = dis(gen);
            cube = FAST_MOVES[move].copyAndApplyTo(cube);
        }

        FastRubiksCube inverse = cube.inverse();

        FastRubiksCube identity = cube.copyAndApplyTo(inverse);
        FastRubiksCube identity2 = inverse.copyAndApplyTo(cube);

        if (identity != identity2) {
            std::cerr << "ERROR: Inverse doesn't work (a * a' != a' * a)!" << std::endl;
            identity.print();
            identity2.print();
            break;
        }

        if (!identity.isSolved()) {
            std::cerr << "ERROR: Inverse doesn't work! (a * a! != 0)" << std::endl;
            identity.print();
            break;
        }
    }
}

void initFastRubiksCubeData() {
    for (int i = 0; i < 18; i++) {
        RubiksCube cube;
        cube.doMove(ALL_MOVES[i]);
        FAST_MOVES[i] = FastRubiksCube(cube);
    }

    FACTORIAL_U32[0] = 1;
    for (int i = 1; i < 13; i++) {
        FACTORIAL_U32[i] = FACTORIAL_U32[i - 1] * i;
    }

    FACTORIAL_U64[0] = 1;
    for (int i = 1; i < 21; i++) {
        FACTORIAL_U64[i] = FACTORIAL_U64[i - 1] * i;
    }

    for (int n = 0; n < 13; n++) {
        for (int r = 0; r <= n; r++) {
            NPR_U32[n][r] = FACTORIAL_U32[n] / FACTORIAL_U32[n - r];
            NCR_U32[n][r] = FACTORIAL_U32[n] / (FACTORIAL_U32[r] * FACTORIAL_U32[n - r]);
        }
    }

    for (int i = 0; i < 65536; i++) {
        uint16_t n = i;
        uint8_t count = 0;
        while (n) {
            count += n & 1;
            n >>= 1;
        }
        BIT_COUNT_U16[i] = count;
    }

    //testMoves();

    std::cout << "FastRubiksCube initialized" <<std::endl;

    initSolver();
}

constexpr FastRubiksCube::FastRubiksCube()
{}

FastRubiksCube::FastRubiksCube(const RubiksCube &cube) {
    for (int i = 0; i < 8; i++) {
        CornerData cd = cube.getCorner((Corner) i);
        uint8_t orientation = cube.getCornerOrientation((Corner) i);

        corners[getCorner(cd)] = (Corner) i;
        cornerOrientations[getCorner(cd)] = orientation;
    }
    for (int i = 0; i < 12; i++) {
        EdgeData ed = cube.getEdge((Edge) i);
        uint8_t orientation = cube.getEdgeOrientation((Edge) i);

        edges[getEdge(ed)] = (Edge) i;
        edgeOrientations[getEdge(ed)] = orientation;
    }

    //Check orientation parity
    int cornerParity = 0;
    for (int i = 0; i < 8; i++) {
        cornerParity += cornerOrientations[i];
    }

    int edgeParity = 0;
    for (int i = 0; i < 12; i++) {
        edgeParity += edgeOrientations[i];
    }

    if (cornerParity % 3 != 0) {
        std::cerr << "INVALID CORNER PARITY: " << cornerParity << std::endl;
    }

    if (edgeParity % 2 != 0) {
        std::cerr << "INVALID EDGE PARITY: " << edgeParity << std::endl;
    }
}

//this.corners[x] stores the position of edge x
//this.cornerOrientation[x] stores the orientation of the corner at position x
FastRubiksCube FastRubiksCube::copyAndApplyTo(const FastRubiksCube& cube) const {
    FastRubiksCube result = cube;

    for (int i = 0; i < 8; i++) {
        result.corners[i] = this->corners[cube.corners[i]];
        result.cornerOrientations[i] =
                (cube.cornerOrientations[i] + this->cornerOrientations[cube.corners[i]]) % 3;
    }

    for (int i = 0; i < 12; i++) {
        result.edges[i] = this->edges[cube.edges[i]];
        result.edgeOrientations[i] =
                (cube.edgeOrientations[i] ^ this->edgeOrientations[cube.edges[i]]);
    }

    return result;
}

FastRubiksCube FastRubiksCube::inverse() const {
    FastRubiksCube result;

    /*
     For all i
     (this->cornerOrientations[cube.edges[i]] ^ result.edgeOrientations[this->edges[cube.edges[i]]]) == 0
     This means
        this->cornerOrientations[cube.edges[i]] == result.edgeOrientations[this->edges[cube.edges[i]]]
     */

    /*
     For all i
        (this.cornerOrientations[cube.corners[i]] + result.cornerOrientations[result.corners[i]]) % 3 == 0
        This means
            result.cornerOrientations[result.corners[i]] = (3 - this.cornerOrientations[cube.corners[i]]) % 3
     */

    for (int i = 0; i < 8; i++) {
        result.corners[corners[i]] = (Corner) i;
        result.cornerOrientations[corners[i]] = (3 - cornerOrientations[i]) % 3;
    }

    for (int i = 0; i < 12; i++) {
        result.edges[edges[i]] = (Edge) i;
        result.edgeOrientations[edges[i]] = edgeOrientations[i];
    }

    return result;
}

RubiksCube FastRubiksCube::toRubiksCube() const {
    RubiksCube result;

    for (int i = 0; i < 8; i++) {
        result.setCorner((Corner) corners[i], (Corner) i, cornerOrientations[i]);
    }

    for (int i = 0; i < 12; i++) {
        result.setEdge((Edge) edges[i], (Edge) i, edgeOrientations[i]);
    }

    return result;
}

uint32_t FastRubiksCube::getCornerPermutationIndex() const {
    uint32_t seen = 0;
    uint32_t result = 0;

    for (int i = 0; i < 8; i++) {
        int v = corners[i];
        int count = BIT_COUNT_U16[seen >> (8 - v)];
        result += (v - count) * FACTORIAL_U32[7 - i];
        seen |= (1 << (7 - v));
    }

    return result;
}

uint32_t FastRubiksCube::getCornerOrientationIndex() const {
    uint32_t result = 0;
    //Storing last corner is redundant
    for (int i = 0; i < 7; i++) {
        result = result * 3 + cornerOrientations[i];
    }

    return result;
}

uint32_t FastRubiksCube::getCornerIndex() const {
    return getCornerPermutationIndex() * 2187 + getCornerOrientationIndex();
}

uint32_t FastRubiksCube::getEdgePermutationIndex() const {
    uint32_t seen = 0;
    uint32_t result = 0;

    for (int i = 0; i < 12; i++) {
        int v = edges[i];
        int count = BIT_COUNT_U16[seen >> (12 - v)];
        result += (v - count) * FACTORIAL_U32[11 - i];
        seen |= (1 << (11 - v));
    }

    return result;
}

uint32_t FastRubiksCube::getEdgeOrientationIndex() const {
    uint32_t result = 0;
    for (int i = 0; i < 11; i++) {
        result = result * 2 + edgeOrientations[i];
    }

    return result;
}

uint64_t FastRubiksCube::getEdgeIndex() const {
    return ((uint64_t) getEdgePermutationIndex()) * 2048ull + getEdgeOrientationIndex();
}

FastRubiksCube FastRubiksCube::doMove(int idx) const {
    return FAST_MOVES[idx].copyAndApplyTo(*this);
}

void FastRubiksCube::print() const {
    std::cout << "  Corner Positions: " << std::endl << "    ";
    for (int i = 0; i < 8; i++) {
        std::cout << (int) corners[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "  Corner Orientations: " << std::endl << "    ";
    for (int i = 0; i < 8; i++) {
        std::cout << (int) cornerOrientations[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "  Edge Positions: " << std::endl << "    ";
    for (int i = 0; i < 12; i++) {
        std::cout << (int) edges[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "  Edge Orientations: " << std::endl << "    ";
    for (int i = 0; i < 12; i++) {
        std::cout << (int) edgeOrientations[i] << " ";
    }
    std::cout << std::endl;
}

bool FastRubiksCube::isSolved() const {
    for (int i = 0; i < 8; i++) {
        if (corners[i] != i || cornerOrientations[i] != 0) {
            return false;
        }
    }

    for (int i = 0; i < 12; i++) {
        if (edges[i] != i || edgeOrientations[i] != 0) {
            return false;
        }
    }

    return true;
}


uint32_t FastRubiksCube::getPartialEdgePermutationIndex(std::vector<Edge>& edgeGroup) const {
    uint32_t seen = 0;
    uint32_t result = 0;

    for (int i = 0; i < edgeGroup.size(); i++) {
        int v = edges[edgeGroup[i]];
        int count = BIT_COUNT_U16[seen >> (12 - v)];
        result += (v - count) * NPR_U32[11 - i][edgeGroup.size() - i - 1];
        seen |= (1 << (11 - v));
    }

    return result;
}

uint32_t FastRubiksCube::getPartialEdgeOrientationIndex(std::vector<Edge>& edgeGroup) const {
    uint32_t result = 0;
    for (int i = 0; i < edgeGroup.size(); i++) {
        result = result * 2 + edgeOrientations[edgeGroup[i]];
    }

    return result;
}

uint64_t FastRubiksCube::getPartialEdgeIndex(std::vector<Edge>& edgeGroup) const {
    return ((uint64_t) getPartialEdgePermutationIndex(edgeGroup)) * (1 << edgeGroup.size()) + getPartialEdgeOrientationIndex(edgeGroup);
}

uint32_t FastRubiksCube::getPartialCornerPermutationIndex(std::vector<Corner>& cornerGroup) const {
    uint32_t seen = 0;
    uint32_t result = 0;

    for (int i = 0; i < cornerGroup.size(); i++) {
        int v = corners[cornerGroup[i]];
        int count = BIT_COUNT_U16[seen >> (8 - v)];
        result += (v - count) * NPR_U32[7 - i][cornerGroup.size() - i - 1];
        seen |= (1 << (7 - v));
    }

    return result;
}

uint32_t FastRubiksCube::getPartialCornerOrientationIndex(std::vector<Corner>& cornerGroup) const {
    uint32_t result = 0;
    for (int i = 0; i < cornerGroup.size(); i++) {
        result = result * 3 + cornerOrientations[cornerGroup[i]];
    }

    return result;
}

uint32_t FastRubiksCube::getPartialCornerIndex(std::vector<Corner>& cornerGroup) const {
    return getPartialCornerPermutationIndex(cornerGroup) * FACTORIAL_U32[cornerGroup.size()] + getPartialCornerOrientationIndex(cornerGroup);
}

bool FastRubiksCube::isPartiallyOriented(std::vector<Edge>& edgeGroup) const {
    for (int i = 0; i < edgeGroup.size(); i++) {
        if (edgeOrientations[edgeGroup[i]] != 0) {
            return false;
        }
    }

    return true;
}

bool FastRubiksCube::isPartiallyOriented(std::vector<Corner>& cornerGroup) const {
    for (int i = 0; i < cornerGroup.size(); i++) {
        if (cornerOrientations[cornerGroup[i]] != 0) {
            return false;
        }
    }

    return true;
}

bool FastRubiksCube::isPartiallyPlaced(std::vector<Edge>& edgeGroup) const {
    for (int i = 0; i < edgeGroup.size(); i++) {
        if (edges[edgeGroup[i]] != edgeGroup[i]) {
            return false;
        }
    }

    return true;
}

bool FastRubiksCube::isPartiallyPlaced(std::vector<Corner>& cornerGroup) const {
    for (int i = 0; i < cornerGroup.size(); i++) {
        if (corners[cornerGroup[i]] != cornerGroup[i]) {
            return false;
        }
    }

    return true;
}

bool FastRubiksCube::isPartiallySolved(std::vector<Edge>& edgeGroup) const {
    return isPartiallyOriented(edgeGroup) && isPartiallyPlaced(edgeGroup);
}

bool FastRubiksCube::isPartiallySolved(std::vector<Corner>& cornerGroup) const {
    return isPartiallyOriented(cornerGroup) && isPartiallyPlaced(cornerGroup);
}

bool FastRubiksCube::isValid() const {
    int cornerParity = 0;
    for (int i = 0; i < 8; i++) {
        cornerParity += cornerOrientations[i];
    }

    int edgeParity = 0;
    for (int i = 0; i < 12; i++) {
        edgeParity += edgeOrientations[i];
    }

    return cornerParity % 3 == 0 && edgeParity % 2 == 0;
}

FastRubiksCube FastRubiksCube::applyBasicSymmetry(const FastRubiksCube &symmetry) const {
    return symmetry.inverse() * (*this) * symmetry;
    //return symmetry * (*this);
}
