//
// Created by Anatol on 13/02/2023.
//

#ifndef RUBIK_KOCIEMBA_H
#define RUBIK_KOCIEMBA_H

#include <cstdint>
#include <optional>

#include "cube/FastRubiksCube.h"

void kociembaInit();

uint32_t cornerOrientationCoordinate(const FastRubiksCube& cube);
uint32_t edgeOrientationCoordinate(const FastRubiksCube& cube);
uint32_t UDSliceCoordinate(const FastRubiksCube& cube);
uint32_t flipUDSliceCoordinate(const FastRubiksCube& cube);
uint32_t cornerPermutationCoordinate(const FastRubiksCube& cube);
uint32_t phase2EdgePermutationCoordinate(const FastRubiksCube& cube);

FastRubiksCube reduceTest(const FastRubiksCube& cube);

std::optional<std::vector<Move>> kociembaSolve(FastRubiksCube cube, bool& halt, std::function<void (std::string)> statusUpdateCallback);

void collectData();

#endif //RUBIK_KOCIEMBA_H
