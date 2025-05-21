//
// Created by Anatol on 29/12/2022.
//

#include "RedundantMovePreventor.h"

constexpr std::bitset<6> ofFaces(Side side) {
    return {static_cast<unsigned long long>(1 << side)};
}

std::bitset<6> ofFaces(Side side1, Side side2) {
    return ofFaces(side1) | ofFaces(side2);
}

const std::bitset<6> BLOCK_AFTER_TURN[6] = {
        ofFaces(FRONT),
        ofFaces(FRONT, BACK),
        ofFaces(LEFT),
        ofFaces(LEFT, RIGHT),
        ofFaces(TOP),
        ofFaces(TOP, BOTTOM)
};

const std::bitset<6> DONT_RELEASE_AFTER_TURN[6] = {
        ofFaces(FRONT, BACK),
        ofFaces(FRONT, BACK),
        ofFaces(LEFT, RIGHT),
        ofFaces(LEFT, RIGHT),
        ofFaces(TOP, BOTTOM),
        ofFaces(TOP, BOTTOM)
};

RedundantMovePreventor::RedundantMovePreventor()
    :dontTurn(0)
{

}

bool RedundantMovePreventor::isRedundant(Side side) {
    return dontTurn[side];
}

void RedundantMovePreventor::turnFace(Side side) {
    dontTurn |= BLOCK_AFTER_TURN[side];
    dontTurn &= DONT_RELEASE_AFTER_TURN[side];
}