//
// Created by Anatol on 21/12/2022.
//

#include <stdint.h>
#include <cassert>
#include <cstring>
#include <bitset>
#include "glm/vec3.hpp"

#ifndef RUBIK_COMMON_H
#define RUBIK_COMMON_H

//DO NOT CHANGE THE ORDER OF THIS
//SO MANY THINGS DEPEND ON IT
enum Side: uint8_t {
    FRONT = 0,
    BACK = 1,
    LEFT = 2,
    RIGHT = 3,
    TOP = 4,
    BOTTOM = 5
};

const static glm::vec3 SIDE_NORMALS[6] = {
        glm::vec3(0, 0, 1),
        glm::vec3(0, 0, -1),
        glm::vec3(-1, 0, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, -1, 0)
};

enum MoveType: uint8_t {
    CLOCKWISE = 1,
    DOUBLE_TURN = 2,
    COUNTERCLOCKWISE = 3
};

struct Move {
    Side side;
    MoveType moveType;

    std::string moveCode() const {
        std::string code;
        switch (side) {
            case FRONT:
                code += "F";
                break;
            case BACK:
                code += "B";
                break;
            case LEFT:
                code += "L";
                break;
            case RIGHT:
                code += "R";
                break;
            case TOP:
                code += "U";
                break;
            case BOTTOM:
                code += "D";
                break;
        }

        switch (moveType) {
            case CLOCKWISE:
                code += "";
                break;
            case DOUBLE_TURN:
                code += "2";
                break;
            case COUNTERCLOCKWISE:
                code += "'";
                break;
        }
        return code;
    }

    static Move fromString(std::string& s) {
        Side side = Side::FRONT;
        MoveType moveType = MoveType::CLOCKWISE;

        switch (s[0]) {
            case 'F':
                side = Side::FRONT;
                break;
            case 'B':
                side = Side::BACK;
                break;
            case 'D':
                side = Side::BOTTOM;
                break;
            case 'U':
                side = Side::TOP;
                break;
            case 'L':
                side = Side::LEFT;
                break;
            case 'R':
                side = Side::RIGHT;
                break;
        }

        if (s.size() > 1) {
            if (s[1] == '\'') {
                moveType = MoveType::COUNTERCLOCKWISE;
            } else if (s[1] == '2') {
                moveType = MoveType::DOUBLE_TURN;
            }
        }

        return Move {side, moveType};
    }
};

const Move ALL_MOVES[18] = {
        {FRONT, CLOCKWISE}, {BACK, CLOCKWISE}, {LEFT, CLOCKWISE}, {RIGHT, CLOCKWISE}, {TOP, CLOCKWISE}, {BOTTOM, CLOCKWISE},
        {FRONT, DOUBLE_TURN}, {BACK, DOUBLE_TURN}, {LEFT, DOUBLE_TURN}, {RIGHT, DOUBLE_TURN}, {TOP, DOUBLE_TURN}, {BOTTOM, DOUBLE_TURN},
        {FRONT, COUNTERCLOCKWISE}, {BACK, COUNTERCLOCKWISE}, {LEFT, COUNTERCLOCKWISE}, {RIGHT, COUNTERCLOCKWISE}, {TOP, COUNTERCLOCKWISE}, {BOTTOM, COUNTERCLOCKWISE}
};

const static Side ADJACENT_CW[6][4] = {
        {TOP, RIGHT, BOTTOM, LEFT},
        {TOP, LEFT, BOTTOM, RIGHT},
        {TOP, FRONT, BOTTOM, BACK},
        {TOP, BACK, BOTTOM, FRONT},
        {FRONT, RIGHT, BACK, LEFT},
        {FRONT, LEFT, BACK, RIGHT}
};

enum Corner: uint8_t {
    TOP_LEFT_FRONT = 0,
    TOP_RIGHT_FRONT = 1,
    TOP_RIGHT_BACK = 2,
    TOP_LEFT_BACK = 3,
    BOTTOM_LEFT_FRONT = 4,
    BOTTOM_RIGHT_FRONT = 5,
    BOTTOM_RIGHT_BACK = 6,
    BOTTOM_LEFT_BACK = 7
};

const static bool CORNER_IS_IN_SIDE[8][6] = {
        {true, false, true, false, true, false},
        {true, false, false, true, true, false},
        {false, true, false, true, true, false},
        {false, true, true, false, true, false},
        {true, false, true, false, false, true},
        {true, false, false, true, false, true},
        {false, true, false, true, false, true},
        {false, true, true, false, false, true}
};

static const char * const CORNER_NAMES[8] = {
        "TOP_LEFT_FRONT",
        "TOP_RIGHT_FRONT",
        "TOP_RIGHT_BACK",
        "TOP_LEFT_BACK",
        "BOTTOM_LEFT_FRONT",
        "BOTTOM_RIGHT_FRONT",
        "BOTTOM_RIGHT_BACK",
        "BOTTOM_LEFT_BACK"
};

enum Edge: uint8_t {
    TOP_FRONT = 0,
    TOP_RIGHT = 1,
    TOP_BACK = 2,
    TOP_LEFT = 3,
    FRONT_RIGHT = 4,
    BACK_RIGHT = 5,
    BACK_LEFT = 6,
    FRONT_LEFT = 7,
    BOTTOM_FRONT = 8,
    BOTTOM_RIGHT = 9,
    BOTTOM_BACK = 10,
    BOTTOM_LEFT = 11
};

const static bool EDGE_IS_IN_SIDE[12][6] = {
//       FRONT  BACK   LEFT   RIGHT  TOP    BOTTOM
        {true,  false, false, false, true,  false}, // TOP_FRONT
        {false, false, false, true,  true, false}, // TOP_RIGHT
        {false, true,  false, false, true, false}, // TOP_BACK
        {false, false, true,  false, true, false}, // TOP_LEFT
        {true,  false, false, true,  false, false}, // FRONT_RIGHT
        {false, true,  false, true,  false, false}, // BACK_RIGHT
        {false, true,  true,  false, false, false}, // BACK_LEFT
        {true,  false, true,  false, false, false}, // FRONT_LEFT
        {true,  false, false, false, false, true}, // BOTTOM_FRONT
        {false, false, false, true,  false, true}, // BOTTOM_RIGHT
        {false, true,  false, false, false, true}, // BOTTOM_BACK
        {false, false, true,  false, false, true} // BOTTOM_LEFT
};

static const char * const EDGE_NAMES[12] = {
        "TOP_FRONT",
        "TOP_RIGHT",
        "TOP_BACK",
        "TOP_LEFT",
        "FRONT_RIGHT",
        "BACK_RIGHT",
        "BACK_LEFT",
        "FRONT_LEFT",
        "BOTTOM_FRONT",
        "BOTTOM_RIGHT",
        "BOTTOM_BACK",
        "BOTTOM_LEFT"
};

enum FaceColor: uint8_t {
    RED,
    ORANGE,
    BLUE,
    GREEN,
    YELLOW,
    WHITE
};

struct CornerData {
    Side ud;
    Side lr;
    Side fb;
};

//If the edge is in the top or bottom face, a is the top/bottom face, b is the other face
//Otherwise, a is the front/back face, b is the left/right face
//This is the same order as in edge names
struct EdgeData {
    Side a;
    Side b;
};

static Corner getCorner(Side ud, Side lr, Side fb) {
    if (ud == TOP) {
        if (lr == LEFT) {
            if (fb == FRONT) {
                return TOP_LEFT_FRONT;
            } else {
                return TOP_LEFT_BACK;
            }
        } else {
            if (fb == FRONT) {
                return TOP_RIGHT_FRONT;
            } else {
                return TOP_RIGHT_BACK;
            }
        }
    } else {
        if (lr == LEFT) {
            if (fb == FRONT) {
                return BOTTOM_LEFT_FRONT;
            } else {
                return BOTTOM_LEFT_BACK;
            }
        } else {
            if (fb == FRONT) {
                return BOTTOM_RIGHT_FRONT;
            } else {
                return BOTTOM_RIGHT_BACK;
            }
        }
    }
}

static Corner getCorner(CornerData data) {
    Side ud, lr, fb;

    if (data.ud == TOP || data.ud == BOTTOM) {
        ud = data.ud;
    } else if (data.lr == TOP || data.lr == BOTTOM) {
        ud = data.lr;
    } else {
        ud = data.fb;
    }

    if (data.ud == LEFT || data.ud == RIGHT) {
        lr = data.ud;
    } else if (data.lr == LEFT || data.lr == RIGHT) {
        lr = data.lr;
    } else {
        lr = data.fb;
    }

    if (data.ud == FRONT || data.ud == BACK) {
        fb = data.ud;
    } else if (data.lr == FRONT || data.lr == BACK) {
        fb = data.lr;
    } else {
        fb = data.fb;
    }

    return getCorner(ud, lr, fb);
}

static const Side CORNER_UD[8] = {TOP, TOP, TOP, TOP, BOTTOM, BOTTOM, BOTTOM, BOTTOM};
static const Side CORNER_LR[8] = {LEFT, RIGHT, RIGHT, LEFT, LEFT, RIGHT, RIGHT, LEFT};
static const Side CORNER_FB[8] = {FRONT, FRONT, BACK, BACK, FRONT, FRONT, BACK, BACK};

static CornerData fromCorner(Corner corner) {
    return {CORNER_UD[corner], CORNER_LR[corner], CORNER_FB[corner]};
}

static Edge getEdge(Side a, Side b) {
    if (a == TOP) {
        if (b == FRONT) {
            return TOP_FRONT;
        } else if (b == RIGHT) {
            return TOP_RIGHT;
        } else if (b == BACK) {
            return TOP_BACK;
        } else {
            return TOP_LEFT;
        }
    } else if (a == BOTTOM) {
        if (b == FRONT) {
            return BOTTOM_FRONT;
        } else if (b == RIGHT) {
            return BOTTOM_RIGHT;
        } else if (b == BACK) {
            return BOTTOM_BACK;
        } else {
            return BOTTOM_LEFT;
        }
    } else if (a == FRONT) {
        if (b == RIGHT) {
            return FRONT_RIGHT;
        } else {
            return FRONT_LEFT;
        }
    } else if (a == BACK) {
        if (b == RIGHT) {
            return BACK_RIGHT;
        } else {
            return BACK_LEFT;
        }
    } else {
        if (b == FRONT) {
            return TOP_FRONT;
        } else {
            return TOP_RIGHT;
        }
    }
}

static Edge getEdge(EdgeData data) {
    Side a, b;

    if (data.a == TOP || data.a == BOTTOM) {
        a = data.a;
        b = data.b;
    } else if (data.b == TOP || data.b == BOTTOM) {
        a = data.b;
        b = data.a;
    } else if (data.a == FRONT || data.a == BACK) {
        a = data.a;
        b = data.b;
    } else {
        assert(data.b == FRONT || data.b == BACK);
        a = data.b;
        b = data.a;
    }

    return getEdge(a, b);
}

static EdgeData fromEdge(Edge edge) {
    switch (edge) {
        case TOP_FRONT:
            return {TOP, FRONT};
        case TOP_RIGHT:
            return {TOP, RIGHT};
        case TOP_BACK:
            return {TOP, BACK};
        case TOP_LEFT:
            return {TOP, LEFT};
        case FRONT_RIGHT:
            return {FRONT, RIGHT};
        case BACK_RIGHT:
            return {BACK, RIGHT};
        case BACK_LEFT:
            return {BACK, LEFT};
        case FRONT_LEFT:
            return {FRONT, LEFT};
        case BOTTOM_FRONT:
            return {BOTTOM, FRONT};
        case BOTTOM_RIGHT:
            return {BOTTOM, RIGHT};
        case BOTTOM_BACK:
            return {BOTTOM, BACK};
        case BOTTOM_LEFT:
            return {BOTTOM, LEFT};
    }
}

#endif //RUBIK_COMMON_H
