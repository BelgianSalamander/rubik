//
// Created by Anatol on 12/02/2023.
//

#include "CubeScanner.h"
#include <iostream>

std::string SIDE_NAMES[6] = {"front", "back", "left", "right", "top", "bottom"};
Side SIDE_ORDER[6] = {Side::TOP, Side::FRONT, Side::RIGHT, Side::BACK, Side::LEFT, Side::BOTTOM};

CubeScanner::CubeScanner()
    :camera()
{
    int deviceID = 0;
    int apiID = cv::CAP_ANY;

    camera.open(deviceID, apiID);

    if (!camera.isOpened()) {
        std::cerr << "ERROR! Unable to open camera\n";
    }
}

CubeScanner::~CubeScanner() {
    camera.release();
    cv::destroyAllWindows();
}

cv::Point getContourCenter(std::vector<cv::Point>& contour) {
    cv::Moments moments = cv::moments(contour);

    if (moments.m00 == 0) {
        return {-1, -1};
    }

    return cv::Point(moments.m10 / moments.m00, moments.m01 / moments.m00);
}

Color getAverageColor(std::vector<cv::Point>& contour, cv::Mat& frame) {
    cv::Mat mask = cv::Mat::zeros(frame.size(), CV_8UC1);
    cv::drawContours(mask, std::vector<std::vector<cv::Point>>{contour}, -1, cv::Scalar(1), -1);

    cv::Scalar averageColor = cv::mean(frame, mask);

    return {(int) averageColor[2], (int) averageColor[1], (int) averageColor[0]};
}

std::optional<Face> tryScanFace(cv::Mat& frame) {
    cv::Mat grayFrame;
    cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

    cv::Mat blurredFrame;
    cv::GaussianBlur(grayFrame, blurredFrame, cv::Size(3, 3), 0);

    cv::Mat cannyFrame;
    cv::Canny(blurredFrame, cannyFrame, 30, 60, 3);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));
    cv::Mat dilatedFrame;
    cv::dilate(cannyFrame, dilatedFrame, kernel);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(dilatedFrame, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> goodContours;

    for (int i = 0; i < contours.size(); i++) {
        auto& contour = contours[i];
        double area = cv::contourArea(contour);

        if (area < 300 || area > 3000) continue;

        std::vector<cv::Point> convexHull;
        cv::convexHull(contour, convexHull);
        double convexHullArea = cv::contourArea(convexHull);
        double solidity = area / convexHullArea;

        if (solidity < 0.9) continue;

        double perimeter = cv::arcLength(contour, true);
        double epsilon = 0.1 * perimeter;
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, epsilon, true);

        if (approx.size() != 4) continue;

        //Check that it is square-like
        std::vector<double> distances;
        for (int i = 0; i < 4; i++) {
            cv::Point p1 = approx[i];
            cv::Point p2 = approx[(i + 1) % 4];
            distances.push_back(cv::norm(p1 - p2));
        }

        std::sort(distances.begin(), distances.end());

        double minDistance = distances[0];
        double maxDistance = distances[3];

        if (maxDistance / minDistance > 1.4) continue;

        goodContours.push_back(contour);
    }

    cv::Mat drawing = cv::Mat::zeros(dilatedFrame.size(), CV_8UC3);
    for (int i = 0; i < goodContours.size(); i++) {
        cv::Scalar color = cv::Scalar(0, 255, 0);
        cv::drawContours(drawing, goodContours, i, color, 2, cv::LINE_8, hierarchy, 0);
    }

    cv::imshow("canny", cannyFrame);


    if (goodContours.size() < 9) {
        cv::imshow("drawing", drawing);
        return std::nullopt;
    }


    std::vector<std::vector<int>> neighbors(goodContours.size());
    double maxDistance = (frame.cols / 3.0) * sqrt(2) * 1.1;
    double minDistance = maxDistance * 0.5 / sqrt(2);

    for (int i = 0; i < goodContours.size(); i++) {
        auto& contour1 = goodContours[i];
        cv::Point center1 = getContourCenter(contour1);

        for (int j = 0; j < goodContours.size(); j++) {
            if (i == j) continue;

            auto& contour2 = goodContours[j];
            cv::Point center2 = getContourCenter(contour2);

            double distance = cv::norm(center1 - center2);

            if (distance < maxDistance && distance > minDistance) {
                neighbors[i].push_back(j);
            }
        }

        //Draw number of neighbors
        cv::putText(drawing, std::to_string(neighbors[i].size()), center1, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
    }

    //Find the center, which has 8 neighbors
    int centerIndex = -1;
    for (int i = 0; i < goodContours.size(); i++) {
        if (neighbors[i].size() == 8) {
            if (centerIndex != -1) {
                cv::imshow("drawing", drawing);
                return std::nullopt;
            }

            centerIndex = i;
        }
    }

    if (centerIndex == -1) {
        cv::imshow("drawing", drawing);
        return std::nullopt;
    }

    cv::Point faceCenter = getContourCenter(goodContours[centerIndex]);

    //Draw center
    cv::drawContours(drawing, goodContours, centerIndex, cv::Scalar(255, 0, 0), 2, cv::LINE_8, hierarchy, 0);

    //The edges will be the four closest neighbors
    std::vector<int> edgeIndices;
    std::vector<cv::Point> edgeCenters;
    std::vector<cv::Point> edgeDirections;

    std::sort(neighbors[centerIndex].begin(), neighbors[centerIndex].end(), [&](int a, int b) {
        cv::Point aCenter = getContourCenter(goodContours[a]);
        cv::Point bCenter = getContourCenter(goodContours[b]);

        double aDistance = cv::norm(faceCenter - aCenter);
        double bDistance = cv::norm(faceCenter - bCenter);

        return aDistance < bDistance;
    });

    for (int i = 0; i < 4; i++) {
        edgeIndices.push_back(neighbors[centerIndex][i]);

        cv::Point center = getContourCenter(goodContours[centerIndex]);
        cv::Point edgeCenter = getContourCenter(goodContours[edgeIndices[i]]);

        cv::Point direction = edgeCenter - center;
        double length = cv::norm(direction);

        //If the edge direction is too far off the cardinal directions, return nothing
        if (abs(direction.x) > length * 0.2 && abs(direction.y) > length * 0.2) {
            cv::imshow("drawing", drawing);
            return std::nullopt;
        }

        edgeDirections.push_back(direction);
        edgeCenters.push_back(edgeCenter);

        cv::drawContours(drawing, goodContours, edgeIndices[i], cv::Scalar(255, 0, 0), 2, cv::LINE_8, hierarchy, 0);
    }

    std::vector<int> cornerIndices;
    std::vector<cv::Point> cornerCenters;

    //Find the corners
    for (int i = 0; i < 4; i++) {
        int edgeIndex = edgeIndices[i];
        cv::Point edgeDirection = edgeDirections[i];

        cv::Point expectedCornerPos = {edgeCenters[i].x + edgeDirection.y, edgeCenters[i].y - edgeDirection.x};
        double expectedDistance = cv::norm(edgeDirection);

        double minDistance = std::numeric_limits<double>::max();
        int minIndex = -1;

        for (int j = 0; j < goodContours.size(); j++) {
            if (j == centerIndex || j == edgeIndex) continue;

            cv::Point center = getContourCenter(goodContours[j]);

            double distance = cv::norm(expectedCornerPos - center);

            if (distance < minDistance) {
                minDistance = distance;
                minIndex = j;
            }
        }

        double distanceDeviation = abs(minDistance) / expectedDistance;

        if (distanceDeviation > 0.2) {
            cv::imshow("drawing", drawing);
            return std::nullopt;
        }

        cornerIndices.push_back(minIndex);
        cornerCenters.push_back(getContourCenter(goodContours[minIndex]));
    }

    for (int i = 0; i < 4; i++) {
        cv::drawContours(drawing, goodContours, cornerIndices[i], cv::Scalar(255, 0, 0), 2, cv::LINE_8, hierarchy, 0);
    }

    Face face;

    Color centerColor = getAverageColor(goodContours[centerIndex], frame);
    face.colors[1][1] = centerColor;
    //Fill contour with color
    cv::drawContours(drawing, goodContours, centerIndex, cv::Scalar(centerColor.b, centerColor.g, centerColor.r), cv::FILLED, cv::LINE_8, hierarchy, 0);

    std::vector<int> edgeOrder = {0, 1, 2, 3};
    std::vector<int> cornerOrder = {0, 1, 2, 3};

    std::sort(edgeOrder.begin(), edgeOrder.end(), [&](int a, int b) {
        return edgeCenters[a].y < edgeCenters[b].y;
    });
    std::sort(edgeOrder.begin() + 1, edgeOrder.begin() + 3, [&](int a, int b) {
        return edgeCenters[a].x < edgeCenters[b].x;
    });

    face.colors[0][1] = getAverageColor(goodContours[edgeIndices[edgeOrder[0]]], frame);
    face.colors[1][0] = getAverageColor(goodContours[edgeIndices[edgeOrder[1]]], frame);
    face.colors[1][2] = getAverageColor(goodContours[edgeIndices[edgeOrder[2]]], frame);
    face.colors[2][1] = getAverageColor(goodContours[edgeIndices[edgeOrder[3]]], frame);

    std::sort(cornerOrder.begin(), cornerOrder.end(), [&](int a, int b) {
        return cornerCenters[a].y < cornerCenters[b].y;
    });
    std::sort(cornerOrder.begin(), cornerOrder.begin() + 2, [&](int a, int b) {
        return cornerCenters[a].x < cornerCenters[b].x;
    });
    std::sort(cornerOrder.begin() + 2, cornerOrder.begin() + 4, [&](int a, int b) {
        return cornerCenters[a].x < cornerCenters[b].x;
    });

    face.colors[0][0] = getAverageColor(goodContours[cornerIndices[cornerOrder[0]]], frame);
    face.colors[0][2] = getAverageColor(goodContours[cornerIndices[cornerOrder[1]]], frame);
    face.colors[2][0] = getAverageColor(goodContours[cornerIndices[cornerOrder[2]]], frame);
    face.colors[2][2] = getAverageColor(goodContours[cornerIndices[cornerOrder[3]]], frame);

    cv::imshow("drawing", drawing);

    return face;
}

struct HSVColor {
    int h;
    int s;
    int v;
};

HSVColor rgbToHsv(Color rgb) {
    HSVColor hsv;

    double r = rgb.r / 255.0;
    double g = rgb.g / 255.0;
    double b = rgb.b / 255.0;

    double max = std::max(r, std::max(g, b));
    double min = std::min(r, std::min(g, b));

    hsv.v = max * 100;

    if (max == 0) {
        hsv.s = 0;
    } else {
        hsv.s = (max - min) / max * 100;
    }

    if (max == min) {
        hsv.h = 0;
    } else if (max == r) {
        hsv.h = (g - b) / (max - min) * 60;
    } else if (max == g) {
        hsv.h = (b - r) / (max - min) * 60 + 120;
    } else if (max == b) {
        hsv.h = (r - g) / (max - min) * 60 + 240;
    }

    if (hsv.h < 0) {
        hsv.h += 360;
    }

    return hsv;
}

double getDistance(HSVColor hsvA, HSVColor hsvB) {
    double h = hsvA.h - hsvB.h;
    double s = hsvA.s - hsvB.s;
    double v = hsvA.v - hsvB.v;

    if (hsvA.s + hsvB.s < 50) {
        h = 0;
    }

    return sqrt(h * h * 5 + s * s + v * v);
}

RubiksCube constructCube(Face faces[6]) {
    RubiksCube cube;

    HSVColor centers[6];

    for (int i = 0; i < 6; i++) {
        centers[i] = rgbToHsv(faces[i].colors[1][1]);
    }

    struct StickerData {
        int face, i, j;
        double distances[6];

        StickerData(int face, int i, int j, Color stickerColor, HSVColor* centers) {
            this->face = face;
            this->i = i;
            this->j = j;

            HSVColor hsv = rgbToHsv(stickerColor);

            for (int k = 0; k < 6; k++) {
                distances[k] = getDistance(hsv, centers[k]);
            }
        }
    };

    std::vector<StickerData> edges;
    std::vector<StickerData> corners;

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (j == 1 && k == 1) {
                    continue;
                }

                if (j == 1 || k == 1) {
                    edges.push_back(StickerData(i, j, k, faces[i].colors[j][k], centers));
                } else {
                    corners.push_back(StickerData(i, j, k, faces[i].colors[j][k], centers));
                }
            }
        }
    }

    int numEdges[6] = {0};
    int numCorners[6] = {0};

    while (edges.size()) {
        int bestEdgeIndex = -1;
        int bestFaceIndex = -1;

        double bestDistance = 1000000;

        for (int i = 0; i < edges.size(); i++) {
            for (int j = 0; j < 6; j++) {
                if (numEdges[j] < 4 && edges[i].distances[j] < bestDistance) {
                    bestDistance = edges[i].distances[j];
                    bestEdgeIndex = i;
                    bestFaceIndex = j;
                }
            }
        }

        numEdges[bestFaceIndex]++;
        StickerData& edge = edges[bestEdgeIndex];
        cube.sides[edge.face][edge.i][edge.j] = (Side) bestFaceIndex;

        edges.erase(edges.begin() + bestEdgeIndex);
    }

    while (corners.size()) {
        int bestCornerIndex = -1;
        int bestFaceIndex = -1;

        double bestDistance = 1000000;

        for (int i = 0; i < corners.size(); i++) {
            for (int j = 0; j < 6; j++) {
                if (numCorners[j] < 4 && corners[i].distances[j] < bestDistance) {
                    bestDistance = corners[i].distances[j];
                    bestCornerIndex = i;
                    bestFaceIndex = j;
                }
            }
        }

        numCorners[bestFaceIndex]++;
        StickerData& corner = corners[bestCornerIndex];
        cube.sides[corner.face][corner.i][corner.j] = (Side) bestFaceIndex;

        corners.erase(corners.begin() + bestCornerIndex);
    }

    return cube;
}

std::optional<RubiksCube> CubeScanner::tick() {
    cv::Mat frame;
    camera.read(frame);

    int width = frame.cols;
    int height = frame.rows;

    int areaSize = std::min(width, height) / 2;
    int areaX = (width - areaSize) / 2;
    int areaY = (height - areaSize) / 2;

    cv::Rect area(areaX, areaY, areaSize, areaSize);
    cv::Mat cropped = frame(area);
    //Draw rectangle
    if (this->currFace) {
        cv::rectangle(frame, area, cv::Scalar(0, 255, 0), 4);
    } else {
        cv::rectangle(frame, area, cv::Scalar(0, 0, 255), 4);
    }

    auto face = tryScanFace(cropped);
    if (face) {
        this->currFace = face.value();
    }

    //Draw small cube in top left
    if (this->currFace) {
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                cv::Rect rect(20 + 30 * c, 20 + 30 * r, 30, 30);
                Color color = this->currFace->colors[r][c];
                cv::rectangle(frame, rect, cv::Scalar(color.b, color.g, color.r), cv::FILLED);
            }
        }
    }

    std::string text = "Please place " + SIDE_NAMES[SIDE_ORDER[this->currFaceIdx]] + " face in the cube area";
    cv::putText(frame, text, cv::Point(130, 65), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);

    cv::imshow("Camera", frame);

    int key = cv::waitKey(30);

    //If it's ESC or Q return an empty cube
    if (key == 27 || key == 113) {
        return RubiksCube();
    }

    //If it's enter then move on to the next face
    if (key == 13) {
       if (!this->currFace) return std::nullopt;

        this->faces[SIDE_ORDER[this->currFaceIdx]] = *this->currFace;
        this->currFaceIdx++;

        if (this->currFaceIdx == 6) {
            return constructCube(this->faces);
        }

        this->currFace = std::nullopt;
    }

    return std::nullopt;
}

Color CubeScanner::getSideColor(Side side, RubiksCube& cube) {
    //Find the most saturated color that belongs in this side
    int maxSat = -1;
    Color maxColor;

    for (int face = 0; face < 6; face++) {
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                if (cube.sides[face][r][c] == side) {
                    Color color = this->faces[face].colors[r][c];
                    HSVColor hsv = rgbToHsv(color);

                    if (hsv.s > maxSat) {
                        maxSat = hsv.s;
                        maxColor = color;
                    }
                }
            }
        }
    }

    return maxColor;
}

