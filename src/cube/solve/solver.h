#include "../FastRubiksCube.h"
#include <vector>
#include <optional>

void initSolver();

std::optional<std::vector<Move>> solve(FastRubiksCube cube, bool& halt, std::function<void (std::string)> statusUpdateCallback);