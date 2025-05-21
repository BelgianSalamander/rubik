//
// Created by Anatol on 29/12/2022.
//

#ifndef RUBIK_FASTRUBIKSCUBE_H
#define RUBIK_FASTRUBIKSCUBE_H


#include "RubiksCube.h"
#include <array>
#include <vector>

static std::array<Edge, 7> EDGE_GROUP_ONE = {Edge::TOP_BACK, Edge::TOP_LEFT, Edge::TOP_RIGHT, Edge::BACK_LEFT, Edge::FRONT_RIGHT, Edge::BOTTOM_FRONT, Edge::BOTTOM_RIGHT};
static std::array<Edge, 7> EDGE_GROUP_TWO = {Edge::TOP_FRONT, Edge::FRONT_LEFT, Edge::BACK_RIGHT, Edge::BOTTOM_BACK, Edge::BOTTOM_LEFT, Edge::TOP_BACK, Edge::FRONT_RIGHT};

static std::array<Edge, 4> EDGE_GROUP_CROSS_ONE = {Edge::TOP_BACK, Edge::TOP_LEFT, Edge::TOP_RIGHT, Edge::TOP_FRONT};

void initFastRubiksCubeData();

extern unsigned int FACTORIAL_U32[13];
extern unsigned long long FACTORIAL_U64[21];
extern unsigned int NPR_U32[13][13];
extern unsigned int NCR_U32[13][13];
extern uint8_t BIT_COUNT_U16[65536];

class FastRubiksCube {
public:
    uint8_t corners[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    uint8_t cornerOrientations[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    uint8_t edges[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    uint8_t edgeOrientations[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    constexpr FastRubiksCube();
    explicit FastRubiksCube(const RubiksCube& cube);

    [[nodiscard]] FastRubiksCube copyAndApplyTo(const FastRubiksCube& cube) const;
    inline FastRubiksCube operator*(const FastRubiksCube& cube) const {
        return copyAndApplyTo(cube);
    }

    [[nodiscard]] RubiksCube toRubiksCube() const;

    [[nodiscard]] uint32_t getEdgePermutationIndex() const;
    [[nodiscard]] uint32_t getEdgeOrientationIndex() const;
    [[nodiscard]] uint64_t getEdgeIndex() const;

    template<size_t Size>
    [[nodiscard]] uint32_t getPartialEdgePermutationIndex(std::array<Edge, Size>& edgeGroup) const {
        uint32_t seen = 0;
        uint32_t result = 0;

        for (int i = 0; i < Size; i++) {
            int v = edges[edgeGroup[i]];
            int count = BIT_COUNT_U16[seen >> (12 - v)];
            result += (v - count) * NPR_U32[11 - i][Size - i - 1];
            seen |= (1 << (11 - v));
        }

        return result;
    }

    template<size_t Size>
    [[nodiscard]] uint32_t getPartialEdgeOrientationIndex(std::array<Edge, Size>& edgeGroup) const {
        uint32_t result = 0;
        for (int i = 0; i < Size; i++) {
            result = result * 2 + edgeOrientations[edgeGroup[i]];
        }

        return result;
    }

    template<size_t Size>
    [[nodiscard]] uint64_t getPartialEdgeIndex(std::array<Edge, Size>& edgeGroup) const {
        return ((uint64_t) getPartialEdgePermutationIndex(edgeGroup)) * (1 << Size) + getPartialEdgeOrientationIndex(edgeGroup);
    }

    [[nodiscard]] uint32_t getPartialEdgePermutationIndex(std::vector<Edge>& edgeGroup) const;
    [[nodiscard]] uint32_t getPartialEdgeOrientationIndex(std::vector<Edge>& edgeGroup) const;
    [[nodiscard]] uint64_t getPartialEdgeIndex(std::vector<Edge>& edgeGroup) const;

    [[nodiscard]] uint32_t getCornerPermutationIndex() const;
    [[nodiscard]] uint32_t getCornerOrientationIndex() const;
    [[nodiscard]] uint32_t getCornerIndex() const;

    template<size_t Size>
    [[nodiscard]] uint32_t getPartialCornerPermutationIndex(std::array<Corner, Size>& cornerGroup) const {
        uint32_t seen = 0;
        uint32_t result = 0;

        for (int i = 0; i < Size; i++) {
            int v = corners[cornerGroup[i]];
            int count = BIT_COUNT_U16[seen >> (8 - v)];
            result += (v - count) * NPR_U32[7 - i][Size - i - 1];
            seen |= (1 << (7 - v));
        }

        return result;
    }

    template<size_t Size>
    [[nodiscard]] uint32_t getPartialCornerOrientationIndex(std::array<Corner, Size>& cornerGroup) const {
        uint32_t result = 0;
        for (int i = 0; i < Size; i++) {
            result = result * 3 + cornerOrientations[cornerGroup[i]];
        }

        return result;
    }

    template<size_t Size>
    [[nodiscard]] uint32_t getPartialCornerIndex(std::array<Corner, Size>& cornerGroup) const {
        return getPartialCornerPermutationIndex(cornerGroup) * FACTORIAL_U32[Size] + getPartialCornerOrientationIndex(cornerGroup);
    }

    [[nodiscard]] uint32_t getPartialCornerPermutationIndex(std::vector<Corner>& cornerGroup) const;
    [[nodiscard]] uint32_t getPartialCornerOrientationIndex(std::vector<Corner>& cornerGroup) const;
    [[nodiscard]] uint32_t getPartialCornerIndex(std::vector<Corner>& cornerGroup) const;

    [[nodiscard]] FastRubiksCube doMove(int idx) const;

    [[nodiscard]] bool isSolved() const;
    [[nodiscard]] bool isValid() const;

    template<size_t Size>
    [[nodiscard]] bool isPartiallyOriented(std::array<Edge, Size>& edgeGroup) const {
        for (int i = 0; i < Size; i++) {
            if (edgeOrientations[edgeGroup[i]] != 0) {
                return false;
            }
        }

        return true;
    }

    template<size_t Size>
    [[nodiscard]] bool isPartiallyOriented(std::array<Corner, Size>& cornerGroup) const {
        for (int i = 0; i < Size; i++) {
            if (cornerOrientations[cornerGroup[i]] != 0) {
                return false;
            }
        }

        return true;
    }

    template<size_t Size>
    [[nodiscard]] bool isPartiallyPlaced(std::array<Edge, Size>& edgeGroup) const {
        for (int i = 0; i < Size; i++) {
            if (edges[edgeGroup[i]] != edgeGroup[i]) {
                return false;
            }
        }

        return true;
    }

    template<size_t Size>
    [[nodiscard]] bool isPartiallyPlaced(std::array<Corner, Size>& cornerGroup) const {
        for (int i = 0; i < Size; i++) {
            if (corners[cornerGroup[i]] != cornerGroup[i]) {
                return false;
            }
        }

        return true;
    }

    template<size_t Size>
    [[nodiscard]] bool isPartiallySolved(std::array<Edge, Size>& edgeGroup) const {
        return isPartiallyOriented(edgeGroup) && isPartiallyPlaced(edgeGroup);
    }

    template<size_t Size>
    [[nodiscard]] bool isPartiallySolved(std::array<Corner, Size>& cornerGroup) const {
        return isPartiallyOriented(cornerGroup) && isPartiallyPlaced(cornerGroup);
    }

    [[nodiscard]] bool isPartiallyOriented(std::vector<Edge>& edgeGroup) const;
    [[nodiscard]] bool isPartiallyOriented(std::vector<Corner>& cornerGroup) const;
    [[nodiscard]] bool isPartiallyPlaced(std::vector<Edge>& edgeGroup) const;
    [[nodiscard]] bool isPartiallyPlaced(std::vector<Corner>& cornerGroup) const;
    [[nodiscard]] bool isPartiallySolved(std::vector<Edge>& edgeGroup) const;
    [[nodiscard]] bool isPartiallySolved(std::vector<Corner>& cornerGroup) const;

    inline bool operator==(const FastRubiksCube& other) const {
        for (int i = 0; i < 8; i++) {
            if (corners[i] != other.corners[i] || cornerOrientations[i] != other.cornerOrientations[i]) {
                return false;
            }
        }
        for (int i = 0; i < 12; i++) {
            if (edges[i] != other.edges[i] || edgeOrientations[i] != other.edgeOrientations[i]) {
                return false;
            }
        }
        return true;
    }

    inline bool operator!=(const FastRubiksCube& other) const {
        for (int i = 0; i < 8; i++) {
            if (corners[i] != other.corners[i] || cornerOrientations[i] != other.cornerOrientations[i]) {
                return true;
            }
        }

        for (int i = 0; i < 12; i++) {
            if (edges[i] != other.edges[i] || edgeOrientations[i] != other.edgeOrientations[i]) {
                return true;
            }
        }

        return false;
    }

    void print() const;

    FastRubiksCube inverse() const;
    FastRubiksCube applyBasicSymmetry(const FastRubiksCube& symmetry) const;
};

#endif //RUBIK_FASTRUBIKSCUBE_H
