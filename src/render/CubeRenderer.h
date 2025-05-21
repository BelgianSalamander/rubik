//
// Created by Anatol on 26/12/2022.
//

#ifndef RUBIK_CUBERENDERER_H
#define RUBIK_CUBERENDERER_H

#include "CubeRenderer.h"
#include "src/cube/RubiksCube.h"
#include "Camera.h"

#include "glad/glad.h"

#include "src/util/easing.h"
#include "CubeScanner.h"
#include "CuberConnection.h"

#include <queue>
#include <mutex>
#include <chrono>
#include <thread>

struct GLFWwindow;

class CubeRenderer {
public:
    CubeRenderer();
    ~CubeRenderer();

    void mainLoop();

    void onResize(int width, int height);
    void onMouseMove(float x, float y);
    void onKeyPress(int key, int scancode, int action, int mods);

    void setCube(RubiksCube cube);

private:
    enum class SolverState {
        OFF,
        RUNNING,
        STOPPING
    };
    Color colorScheme[7] = {
            {255, 0, 0},
            {255, 171, 0},
            {0, 0, 255},
            {0, 255, 0},
            {255, 255, 0},
            {255, 255, 255},
            {0, 0, 0} //Outline color
    };

    RubiksCube cube{};
    Camera camera{};

    std::queue<Move> moveQueue;

    int width, height;

    bool firstMouse = true;
    float prevMouseX, prevMouseY;

    double lastFrameTime = 0;

    Move currentMove;
    bool isMoving = false;
    double turnStartTime = 0;
    double turnDuration;

    bool cameraMode;
    float turnSpeed = 0.f;

    float (*easingFunction)(float) = easeInOutBack;
    std::string easingFunctionName = "EaseInOutBack";

    //OpenGL stuff
    GLFWwindow* window;

    GLuint vao;
    GLuint vbo;

    GLuint shaderProgram;
    GLint sidesUniform;
    GLint transformUniform;
    GLint colorSchemeUniforms[7];

    std::optional<CubeScanner> scanner;
    CuberConnection robot;

    std::optional<std::thread> solverThread;
    std::optional<std::vector<Move>> solverResult;
    std::mutex solverMutex;
    SolverState solverState = SolverState::OFF;
    bool haltSolver = false;
    std::chrono::duration<double> timeTaken;

    //Cube handling
    void reset();
    void shuffle();
    void flushMoves();

    //GL methods
    void initGL();
    void shutdownGL();

    void checkShaderCompileErrors(GLuint shader, const char *string);

    //Other
    void renderCorner(Corner corner, glm::mat4 viewModel, Side currSpin, glm::mat4 spinTransform);
    void renderEdge(Edge edge, glm::mat4 viewModel, Side currSpin, glm::mat4 spinTransform);
    void renderCenter(Side side, glm::mat4 viewModel, Side currSpin, glm::mat4 spinTransform) const;

    void processMoves(double t);

    void renderCubit(glm::mat4 model, glm::mat4 viewProj, int *sides) const;

    void processInput(double dt, double currTime);

    void render(double dt, double currTime);

    void renderImGui();

    void toggleCameraMode();

    void uploadColorScheme();
    void pushAllMoves(std::vector<Move> moves);

    void launchSolver();
    void stopSolver();
};

#endif //RUBIK_CUBERENDERER_H
