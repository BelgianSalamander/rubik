//
// Created by Anatol on 29/12/2022.
//

#ifndef RUBIK_REDUNDANTMOVEPREVENTOR_H
#define RUBIK_REDUNDANTMOVEPREVENTOR_H


#include "common.h"

#include <bitset>

class RedundantMovePreventor {
public:
    RedundantMovePreventor();

    bool isRedundant(Side side);
    void turnFace(Side side);

    inline bool isRedundant(const Move& move) {
        return isRedundant(move.side);
    }
private:
    std::bitset<6> dontTurn;
};


#endif //RUBIK_REDUNDANTMOVEPREVENTOR_H
