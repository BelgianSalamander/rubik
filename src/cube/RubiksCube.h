//
// Created by Anatol on 21/12/2022.
//

#ifndef RUBIK_RUBIKSCUBE_H
#define RUBIK_RUBIKSCUBE_H


#include "common.h"

struct Face;
struct Color;

class RubiksCube {
public:
    Side sides[6][3][3];

    RubiksCube();

    //Rotate a face clockwise
    void rotate(Side side);
    void doMove(Side side, MoveType moveType);

    inline void doMove(Move move) {
        doMove(move.side, move.moveType);
    }

    [[nodiscard]] CornerData getCorner(Corner corner) const;
    [[nodiscard]] EdgeData getEdge(Edge edge) const;

    [[nodiscard]] uint8_t getCornerOrientation(Corner corner) const;
    [[nodiscard]] uint8_t getEdgeOrientation(Edge edge) const;

    void setCorner(Corner targetPos, Corner targetValue, uint8_t orientation);
    void setEdge(Edge targetPos, Edge targetValue, uint8_t orientation);

    void saveNetImage(const char* filename);

    inline bool operator==(const RubiksCube& other) const {
        for (int face = 0; face < 6; face++) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (sides[face][i][j] != other.sides[face][i][j]) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    inline bool operator!=(const RubiksCube& other) const {
        for (int face = 0; face < 6; face++) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (sides[face][i][j] != other.sides[face][i][j]) {
                        return true;
                    }
                }
            }
        }

        return false;
    }
private:
    void rotateCorner(Corner corner);
    void flipEdge(Edge edge);
};


#endif //RUBIK_RUBIKSCUBE_H
