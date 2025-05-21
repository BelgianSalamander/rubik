//
// Created by Anatol on 12/02/2023.
//

#ifndef RUBIK_CUBESCANNER_H
#define RUBIK_CUBESCANNER_H

#include "cube/RubiksCube.h"
#include <optional>


#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

struct Color {
    int r;
    int g;
    int b;
};

struct Face {
    Color colors[3][3];
};

class CubeScanner {
public:
    CubeScanner();
    ~CubeScanner();

    std::optional<RubiksCube> tick();
    Color getSideColor(Side side, RubiksCube& cube);
private:
    cv::VideoCapture camera;

    std::optional<Face> currFace = std::nullopt;
    int currFaceIdx = 0;
    Face faces[6];
};


#endif //RUBIK_CUBESCANNER_H
