//
// Created by Anatol on 21/12/2022.
//

#include "RubiksCube.h"
#include "stb_image_write.h"
#include <utility>

RubiksCube::RubiksCube() {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                sides[i][j][k] = (Side) i;
            }
        }
    }
}

template<typename T>
inline void swap(T& a, T& b, T& c, T& d) {
    T temp = d;
    d = c;
    c = b;
    b = a;
    a = temp;
}

template<typename T>
inline void swap(T& a, T& b, T& c) {
    T temp = c;
    c = b;
    b = a;
    a = temp;
}

using std::swap;

void RubiksCube::rotate(Side side) {
    //Rotate the face itself
    swap(sides[side][0][0], sides[side][0][2], sides[side][2][2], sides[side][2][0]);
    swap(sides[side][0][1], sides[side][1][2], sides[side][2][1], sides[side][1][0]);

    //Rotate the adjacent faces clockwise
    switch (side) {
        case TOP:
            swap(sides[FRONT][0][0], sides[LEFT][0][0], sides[BACK][0][0], sides[RIGHT][0][0]);
            swap(sides[FRONT][0][1], sides[LEFT][0][1], sides[BACK][0][1], sides[RIGHT][0][1]);
            swap(sides[FRONT][0][2], sides[LEFT][0][2], sides[BACK][0][2], sides[RIGHT][0][2]);
            break;
        case BOTTOM:
            swap(sides[FRONT][2][0], sides[RIGHT][2][0], sides[BACK][2][0], sides[LEFT][2][0]);
            swap(sides[FRONT][2][1], sides[RIGHT][2][1], sides[BACK][2][1], sides[LEFT][2][1]);
            swap(sides[FRONT][2][2], sides[RIGHT][2][2], sides[BACK][2][2], sides[LEFT][2][2]);
            break;
        case FRONT:
            swap(sides[TOP][2][0], sides[RIGHT][0][0], sides[BOTTOM][0][2], sides[LEFT][2][2]);
            swap(sides[TOP][2][1], sides[RIGHT][1][0], sides[BOTTOM][0][1], sides[LEFT][1][2]);
            swap(sides[TOP][2][2], sides[RIGHT][2][0], sides[BOTTOM][0][0], sides[LEFT][0][2]);
            break;
        case BACK:
            swap(sides[TOP][0][0], sides[LEFT][2][0], sides[BOTTOM][2][2], sides[RIGHT][0][2]);
            swap(sides[TOP][0][1], sides[LEFT][1][0], sides[BOTTOM][2][1], sides[RIGHT][1][2]);
            swap(sides[TOP][0][2], sides[LEFT][0][0], sides[BOTTOM][2][0], sides[RIGHT][2][2]);
            break;
        case LEFT:
            swap(sides[TOP][0][0], sides[FRONT][0][0], sides[BOTTOM][0][0], sides[BACK][2][2]);
            swap(sides[TOP][1][0], sides[FRONT][1][0], sides[BOTTOM][1][0], sides[BACK][1][2]);
            swap(sides[TOP][2][0], sides[FRONT][2][0], sides[BOTTOM][2][0], sides[BACK][0][2]);
            break;
        case RIGHT:
            swap(sides[TOP][0][2], sides[BACK][2][0], sides[BOTTOM][0][2], sides[FRONT][0][2]);
            swap(sides[TOP][1][2], sides[BACK][1][0], sides[BOTTOM][1][2], sides[FRONT][1][2]);
            swap(sides[TOP][2][2], sides[BACK][0][0], sides[BOTTOM][2][2], sides[FRONT][2][2]);
            break;
    }
}

void RubiksCube::doMove(Side side, MoveType moveType) {
    for (int i = 0; i < moveType; i++) {
        rotate(side);
    }
}

CornerData RubiksCube::getCorner(Corner corner) const {
    switch (corner) {
        case TOP_LEFT_BACK:
            return {
                sides[TOP][0][0],
                sides[LEFT][0][0],
                sides[BACK][0][2]
            };
        case TOP_LEFT_FRONT:
            return {
                sides[TOP][2][0],
                sides[LEFT][0][2],
                sides[FRONT][0][0]
            };
        case TOP_RIGHT_FRONT:
            return {
                sides[TOP][2][2],
                sides[RIGHT][0][0],
                sides[FRONT][0][2]
            };
        case TOP_RIGHT_BACK:
            return {
                sides[TOP][0][2],
                sides[RIGHT][0][2],
                sides[BACK][0][0]
            };
        case BOTTOM_LEFT_BACK:
            return {
                sides[BOTTOM][2][0],
                sides[LEFT][2][0],
                sides[BACK][2][2]
            };
        case BOTTOM_LEFT_FRONT:
            return {
                sides[BOTTOM][0][0],
                sides[LEFT][2][2],
                sides[FRONT][2][0]
            };
        case BOTTOM_RIGHT_FRONT:
            return {
                sides[BOTTOM][0][2],
                sides[RIGHT][2][0],
                sides[FRONT][2][2]
            };
        case BOTTOM_RIGHT_BACK:
            return {
                sides[BOTTOM][2][2],
                sides[RIGHT][2][2],
                sides[BACK][2][0]
            };
    }
}

EdgeData RubiksCube::getEdge(Edge edge) const {
    switch (edge) {
        case TOP_LEFT:
            return {
                sides[TOP][1][0],
                sides[LEFT][0][1]
            };
        case TOP_FRONT:
            return {
                sides[TOP][2][1],
                sides[FRONT][0][1]
            };
        case TOP_RIGHT:
            return {
                sides[TOP][1][2],
                sides[RIGHT][0][1]
            };
        case TOP_BACK:
            return {
                sides[TOP][0][1],
                sides[BACK][0][1]
            };


        case BOTTOM_LEFT:
            return {
                sides[BOTTOM][1][0],
                sides[LEFT][2][1]
            };
        case BOTTOM_FRONT:
            return {
                sides[BOTTOM][0][1],
                sides[FRONT][2][1]
            };
        case BOTTOM_RIGHT:
            return {
                sides[BOTTOM][1][2],
                sides[RIGHT][2][1]
            };
        case BOTTOM_BACK:
            return {
                sides[BOTTOM][2][1],
                sides[BACK][2][1]
            };


        case FRONT_LEFT:
            return {
                sides[FRONT][1][0],
                sides[LEFT][1][2]
            };
        case FRONT_RIGHT:
            return {
                sides[FRONT][1][2],
                sides[RIGHT][1][0]
            };
        case BACK_LEFT:
            return {
                sides[BACK][1][2],
                sides[LEFT][1][0]
            };
        case BACK_RIGHT:
            return {
                sides[BACK][1][0],
                sides[RIGHT][1][2]
            };
    }
}

uint8_t RubiksCube::getCornerOrientation(Corner corner) const {
    CornerData cornerData = getCorner(corner);

    if (cornerData.ud == TOP || cornerData.ud == BOTTOM) {
        return 0; //Oriented corner
    }

    switch (corner) {
        case TOP_LEFT_BACK:
        case TOP_RIGHT_FRONT:
        case BOTTOM_LEFT_FRONT:
        case BOTTOM_RIGHT_BACK:
            return (cornerData.lr == TOP || cornerData.lr == BOTTOM) ? 1 : 2;

        case TOP_LEFT_FRONT:
        case TOP_RIGHT_BACK:
        case BOTTOM_LEFT_BACK:
        case BOTTOM_RIGHT_FRONT:
            return (cornerData.fb == TOP || cornerData.fb == BOTTOM) ? 1 : 2;
    }
}

uint8_t RubiksCube::getEdgeOrientation(Edge edge) const {
    EdgeData edgeData = getEdge(edge);

    if (edgeData.a == LEFT || edgeData.a == RIGHT) {
        return 1; //Disoriented edge
    }

    if (edgeData.a == FRONT || edgeData.a == BACK) {
        if (edgeData.b == TOP || edgeData.b == BOTTOM) {
            return 1; //Disoriented edge
        }
    }

    return 0; //Oriented edge
}

void RubiksCube::setCorner(Corner targetPos, Corner targetValue, uint8_t orientation) {
    CornerData targetData = fromCorner(targetValue);

    bool g1 = targetPos == TOP_LEFT_FRONT || targetPos == TOP_RIGHT_BACK || targetPos == BOTTOM_LEFT_BACK || targetPos == BOTTOM_RIGHT_FRONT;
    bool g2 = targetValue == TOP_LEFT_FRONT || targetValue == TOP_RIGHT_BACK || targetValue == BOTTOM_LEFT_BACK || targetValue == BOTTOM_RIGHT_FRONT;

    if (g1 != g2) {
        swap(targetData.lr, targetData.fb);
    }

    switch (targetPos) {
        case TOP_LEFT_BACK:
            sides[TOP][0][0] = targetData.ud;
            sides[LEFT][0][0] = targetData.lr;
            sides[BACK][0][2] = targetData.fb;
            break;
        case TOP_LEFT_FRONT:
            sides[TOP][2][0] = targetData.ud;
            sides[LEFT][0][2] = targetData.lr;
            sides[FRONT][0][0] = targetData.fb;
            break;
        case TOP_RIGHT_FRONT:
            sides[TOP][2][2] = targetData.ud;
            sides[RIGHT][0][0] = targetData.lr;
            sides[FRONT][0][2] = targetData.fb;
            break;
        case TOP_RIGHT_BACK:
            sides[TOP][0][2] = targetData.ud;
            sides[RIGHT][0][2] = targetData.lr;
            sides[BACK][0][0] = targetData.fb;
            break;
        case BOTTOM_LEFT_BACK:
            sides[BOTTOM][2][0] = targetData.ud;
            sides[LEFT][2][0] = targetData.lr;
            sides[BACK][2][2] = targetData.fb;
            break;
        case BOTTOM_LEFT_FRONT:
            sides[BOTTOM][0][0] = targetData.ud;
            sides[LEFT][2][2] = targetData.lr;
            sides[FRONT][2][0] = targetData.fb;
            break;
        case BOTTOM_RIGHT_FRONT:
            sides[BOTTOM][0][2] = targetData.ud;
            sides[RIGHT][2][0] = targetData.lr;
            sides[FRONT][2][2] = targetData.fb;
            break;
        case BOTTOM_RIGHT_BACK:
            sides[BOTTOM][2][2] = targetData.ud;
            sides[RIGHT][2][2] = targetData.lr;
            sides[BACK][2][0] = targetData.fb;
            break;
    }

    for (int i = 0; i < orientation; i++) {
        rotateCorner(targetPos);
    }
}

void RubiksCube::rotateCorner(Corner corner) {
    //Should rotate clockwise
    switch (corner) {
        case TOP_LEFT_BACK:
            swap(sides[BACK][0][2], sides[TOP][0][0], sides[LEFT][0][0]);
            break;
        case TOP_LEFT_FRONT:
            swap(sides[TOP][2][0], sides[FRONT][0][0], sides[LEFT][0][2]);
            break;
        case TOP_RIGHT_FRONT:
            swap(sides[FRONT][0][2], sides[TOP][2][2], sides[RIGHT][0][0]);
            break;
        case TOP_RIGHT_BACK:
            swap(sides[RIGHT][0][2], sides[TOP][0][2], sides[BACK][0][0]);
            break;
        case BOTTOM_LEFT_BACK:
            swap(sides[LEFT][2][0], sides[BOTTOM][2][0], sides[BACK][2][2]);
            break;
        case BOTTOM_LEFT_FRONT:
            swap(sides[BOTTOM][0][0], sides[LEFT][2][2], sides[FRONT][2][0]);
            break;
        case BOTTOM_RIGHT_FRONT:
            swap(sides[BOTTOM][0][2], sides[FRONT][2][2], sides[RIGHT][2][0]);
            break;
        case BOTTOM_RIGHT_BACK:
            swap(sides[BOTTOM][2][2], sides[RIGHT][2][2], sides[BACK][2][0]);
            break;
    }
}

void RubiksCube::setEdge(Edge targetPos, Edge targetValue, uint8_t orientation) {
    EdgeData targetData = fromEdge(targetValue);

    switch (targetPos) {
        case TOP_FRONT:
            sides[TOP][2][1] = targetData.a;
            sides[FRONT][0][1] = targetData.b;
            break;
        case TOP_BACK:
            sides[TOP][0][1] = targetData.a;
            sides[BACK][0][1] = targetData.b;
            break;
        case TOP_LEFT:
            sides[TOP][1][0] = targetData.a;
            sides[LEFT][0][1] = targetData.b;
            break;
        case TOP_RIGHT:
            sides[TOP][1][2] = targetData.a;
            sides[RIGHT][0][1] = targetData.b;
            break;
        case BOTTOM_FRONT:
            sides[BOTTOM][0][1] = targetData.a;
            sides[FRONT][2][1] = targetData.b;
            break;
        case BOTTOM_BACK:
            sides[BOTTOM][2][1] = targetData.a;
            sides[BACK][2][1] = targetData.b;
            break;
        case BOTTOM_LEFT:
            sides[BOTTOM][1][0] = targetData.a;
            sides[LEFT][2][1] = targetData.b;
            break;
        case BOTTOM_RIGHT:
            sides[BOTTOM][1][2] = targetData.a;
            sides[RIGHT][2][1] = targetData.b;
            break;
        case FRONT_LEFT:
            sides[FRONT][1][0] = targetData.a;
            sides[LEFT][1][2] = targetData.b;
            break;
        case FRONT_RIGHT:
            sides[FRONT][1][2] = targetData.a;
            sides[RIGHT][1][0] = targetData.b;
            break;
        case BACK_LEFT:
            sides[BACK][1][2] = targetData.a;
            sides[LEFT][1][0] = targetData.b;
            break;
        case BACK_RIGHT:
            sides[BACK][1][0] = targetData.a;
            sides[RIGHT][1][2] = targetData.b;
            break;
    }

    if (orientation == 1) {
        flipEdge(targetPos);
    }
}

void RubiksCube::flipEdge(Edge edge) {
    switch (edge) {
        case TOP_FRONT:
            swap(sides[TOP][2][1], sides[FRONT][0][1]);
            break;
        case TOP_BACK:
            swap(sides[TOP][0][1], sides[BACK][0][1]);
            break;
        case TOP_LEFT:
            swap(sides[TOP][1][0], sides[LEFT][0][1]);
            break;
        case TOP_RIGHT:
            swap(sides[TOP][1][2], sides[RIGHT][0][1]);
            break;
        case BOTTOM_FRONT:
            swap(sides[BOTTOM][0][1], sides[FRONT][2][1]);
            break;
        case BOTTOM_BACK:
            swap(sides[BOTTOM][2][1], sides[BACK][2][1]);
            break;
        case BOTTOM_LEFT:
            swap(sides[BOTTOM][1][0], sides[LEFT][2][1]);
            break;
        case BOTTOM_RIGHT:
            swap(sides[BOTTOM][1][2], sides[RIGHT][2][1]);
            break;
        case FRONT_LEFT:
            swap(sides[FRONT][1][0], sides[LEFT][1][2]);
            break;
        case FRONT_RIGHT:
            swap(sides[FRONT][1][2], sides[RIGHT][1][0]);
            break;
        case BACK_LEFT:
            swap(sides[BACK][1][2], sides[LEFT][1][0]);
            break;
        case BACK_RIGHT:
            swap(sides[BACK][1][0], sides[RIGHT][1][2]);
            break;
    }
}

void RubiksCube::saveNetImage(const char *filename) {
    //Each square is 50x50 pixels one pixel of which is a border
    //So each face is 150x150 pixels
    //Therefore the whole image is 600x450 pixels

    //Red Orange Blue Green Yellow White
    const uint8_t COLORS_RGB[7][3] = {
            {255, 0,   0}, //RED
            {255, 165, 0}, //ORANGE
            {0,   0,   255}, //BLUE
            {0,   255, 0}, //GREEN
            {255, 255, 0}, //YELLOW
            {255, 255, 255}, //WHITE
            {0,   0,   0} //BORDER
    };

    const int SQUARE_SIZE = 50;
    const int FACE_SIZE = SQUARE_SIZE * 3;
    const int IMAGE_WIDTH = FACE_SIZE * 4;
    const int IMAGE_HEIGHT = FACE_SIZE * 3;

    uint8_t* image = new uint8_t[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

    auto drawSquare = [&](int x, int y, FaceColor color) {
        for (int i = 0; i < 50; i++) {
            for (int j = 0; j < 50; j++) {
                auto* c = COLORS_RGB[color];

                if (i == 0 || i == 49 || j == 0 || j == 49) {
                    c = COLORS_RGB[6];
                }

                int xCoord = x * SQUARE_SIZE + i;
                int yCoord = y * SQUARE_SIZE + j;

                image[(yCoord * IMAGE_WIDTH + xCoord) * 3] = c[0];
                image[(yCoord * IMAGE_WIDTH + xCoord) * 3 + 1] = c[1];
                image[(yCoord * IMAGE_WIDTH + xCoord) * 3 + 2] = c[2];
            }
        }
    };

    auto drawFace = [&](int x, int y, Side face) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                drawSquare(x * 3 + j, y * 3 + i, (FaceColor) sides[face][i][j]);
            }
        }
    };

    //Draw the faces
    drawFace(1, 0, TOP);
    drawFace(1, 2, BOTTOM);
    drawFace(0, 1, LEFT);
    drawFace(2, 1, RIGHT);
    drawFace(1, 1, FRONT);
    drawFace(3, 1, BACK);

    //Save with stb_image_write
    stbi_write_png(filename, IMAGE_WIDTH, IMAGE_HEIGHT, 3, image, IMAGE_WIDTH * 3);
}
