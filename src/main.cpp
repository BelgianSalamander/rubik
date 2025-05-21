#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#include "cube/RubiksCube.h"
#include "common.h"
#include "render/CubeRenderer.h"
#include "util/RedundantMovePreventor.h"
#include <random>
#include <iostream>
#include "cube/FastRubiksCube.h"
#include "render/CubeScanner.h"
#include "optional"

int main() {
    std::cout << "Initializing fast cube data!" << std::endl;
    initFastRubiksCubeData();

    CubeRenderer renderer;
    renderer.mainLoop();
}
