//
// Created by Anatol on 26/12/2022.
//

#include <iostream>
#include "CubeRenderer.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "lib/imgui/imgui.h"
#include "lib/imgui/backends/imgui_impl_glfw.h"
#include "lib/imgui/backends/imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
#include "lib/glm/glm/glm.hpp"
#include "lib/glm/glm/matrix.hpp"
#include <map>
#include <random>
#include <cmath>
#include "src/util/easing.h"
#include "src/util/RedundantMovePreventor.h"
#include "src/cube/FastRubiksCube.h"
#include "src/cube/solve/solver.h"
#include "src/cube/solve/kociemba.h"

std::map<GLFWwindow*, CubeRenderer*> windowToRenderer;

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowToRenderer[window]->onResize(width, height);
}

void mouseMoveCallback(GLFWwindow* window, double x, double y) {
    windowToRenderer[window]->onMouseMove(x, y);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    windowToRenderer[window]->onKeyPress(key, scancode, action, mods);
}

/*
 * To render the cube, we will render it as its 26 separate cubies in 26 passes because I can't be bothered to do it any other way.
*/

const char * VERTEX_SHADER = R"(
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in float side;
layout (location = 2) in vec2 uv;

uniform mat4 projViewModel;
uniform int sides[6];

out vec3 color;
out vec2 texCoord;

//DEFINE COLORS FR
uniform vec3 colors[7] = vec3[](
    vec3(1.0, 0.0, 0.0), //RED (FRONT)
    vec3(1.0, 0.647, 0.0), //ORANGE (BACK)
    vec3(0.0, 0.0, 1.0), //BLUE (LEFT)
    vec3(0.0, 1.0, 0.0), //GREEN (RIGHT)
    vec3(1.0, 1.0, 0.0), //YELLOW (TOP)
    vec3(1.0, 1.0, 1.0), //WHITE (BOTTOM)
    vec3(0.0, 0.0, 0.0) //BLACK (OUTSIDE)
);

void main() {
    gl_Position = projViewModel * vec4(pos, 1.0);
    color = colors[sides[int(side)]];
    texCoord = uv;
}
)";

const char * FRAGMENT_SHADER = R"(
#version 330 core

in vec3 color;
in vec2 texCoord;

out vec4 fragColor;

uniform vec3 colors[7];

void main() {
    //float additional = 0.9 + 0.1 * sin(texCoord.x * 100.0 + texCoord.y * 100.0);
    float additional = 1.0;
    fragColor = vec4(additional * color, 1.0);

    if (texCoord.x < 0.05 || texCoord.x > 0.95 || texCoord.y < 0.05 || texCoord.y > 0.95) {
        fragColor = vec4(additional * colors[6], 1.0);
    }
}
)";


bool gladInitialized = false;

const float CUBE_VERTICES[] = {
        //FRONT (0)
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,

        //BACK (1)
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,

        //LEFT (2)
        -0.5f, -0.5f, -0.5f, 2.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 2.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 2.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 2.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 2.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 2.0f, 1.0f, 0.0f,

        //RIGHT (3)
        0.5f, -0.5f, 0.5f, 3.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 3.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 3.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 3.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 3.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 3.0f, 0.0f, 0.0f,

        //TOP (4)
        -0.5f, 0.5f, 0.5f, 4.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 4.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 4.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 4.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 4.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 4.0f, 0.0f, 0.0f,

        //BOTTOM (5)
        0.5f, -0.5f, 0.5f, 5.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 5.0f, 1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 5.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 5.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 5.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 5.0f, 1.0f, 0.0f
};

CubeRenderer::CubeRenderer()
    :
        cube(),
        robot("192.168.137.1", 12345)
{
    lastFrameTime = glfwGetTime();
    cameraMode = true;
    initGL();
}

CubeRenderer::~CubeRenderer() {
    shutdownGL();
}

void CubeRenderer::initGL() {
    //Create window
    width = 800;
    height = 600;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 16);
    window = glfwCreateWindow(width, height, "Cube Renderer", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseMoveCallback);
    glfwSetKeyCallback(window, keyCallback);

    //Remove cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladInitialized) {
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            exit(-1);
        }
        gladInitialized = true;
    }

    //Create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //Create VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);

    //Define vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //Create program
    shaderProgram = glCreateProgram();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VERTEX_SHADER, NULL);
    glCompileShader(vertexShader);
    checkShaderCompileErrors(vertexShader, "VERTEX");

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompileErrors(fragmentShader, "FRAGMENT");

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << std::endl << infoLog << std::endl;
        exit(-1);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);
    sidesUniform = glGetUniformLocation(shaderProgram, "sides");
    transformUniform = glGetUniformLocation(shaderProgram, "projViewModel");
    for (int i = 0; i < 7; i++) {
        std::string uniformName = "colors[" + std::to_string(i) + "]";
        colorSchemeUniforms[i] = glGetUniformLocation(shaderProgram, uniformName.c_str());
    }

    uploadColorScheme();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glEnable(GL_MULTISAMPLE);

    windowToRenderer[window] = this;


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void CubeRenderer::checkShaderCompileErrors(GLuint shader, const char *string) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::" << string << "::COMPILATION_FAILED" << std::endl;
        std::cout << infoLog << std::endl;
        exit(-1);
    }
}

void CubeRenderer::shutdownGL() {
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwTerminate();

    windowToRenderer.erase(window);
}

void CubeRenderer::onResize(int width, int height) {
    this->width = width;
    this->height = height;
    glViewport(0, 0, width, height);
}

void CubeRenderer::onMouseMove(float xpos, float ypos) {
    if (!cameraMode) return;

    if (firstMouse) {
        prevMouseX = xpos;
        prevMouseY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - prevMouseX;
    float yoffset = prevMouseY - ypos;

    prevMouseX = xpos;
    prevMouseY = ypos;

    this->camera.mouseMove(xoffset, yoffset);
}

void CubeRenderer::processInput(double dt, double currTime) {
    if (cameraMode) {
        this->camera.update(dt);
    }

    processMoves(currTime);
}

void CubeRenderer::render(double dt, double currTime) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    for (int i = 0; i < 3; i++) {
        glEnableVertexAttribArray(i);
    }

    Side currSpin = (Side) -1;
    glm::mat4 transform = glm::mat4(1.0f);

    if (isMoving) {
        currSpin = currentMove.side;
        float targetAngle;
        switch (currentMove.moveType) {
            case MoveType::CLOCKWISE:
                targetAngle = 90.0f;
                break;
            case MoveType::COUNTERCLOCKWISE:
                targetAngle = -90.0f;
                break;
            case MoveType::DOUBLE_TURN:
                targetAngle = 180.0f;
                break;
        }

        glm::vec3 axis = SIDE_NORMALS[currSpin];

        float t = (float) (currTime - turnStartTime) / (float) turnDuration;
        t = easingFunction(t);

        float angle = glm::mix(0.0f, targetAngle, t);

        transform = glm::rotate(transform, -glm::radians(angle)/* * (axis.x + axis.y + axis.z)*/, axis);
    }

    glm::mat4 view = this->camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(80.f), (float) width / (float) height, 0.1f, 100.0f);

    glm::mat4 combined = projection * view;

    for (int i = 0; i < 8; i++) {
        renderCorner((Corner) i, combined, currSpin, transform);
    }

    for (int i = 0; i < 12; i++) {
        renderEdge((Edge) i, combined, currSpin, transform);
    }

    for (int i = 0; i < 6; i++) {
        renderCenter((Side) i, combined, currSpin, transform);
    }
}

void CubeRenderer::renderImGui() {
    if (cameraMode) return;

    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Cube");

    ImGui::Text("Cube Controls");
    ImGui::BeginChild("Cube Controls", ImVec2(0, 90), true);
    if (this->isMoving) {
        ImGui::Text("Currently executing move: %s", this->currentMove.moveCode().c_str());
    } else {
        ImGui::Text("Currently executing move: None");
    }

    //Show move queue
    ImGui::Text("Move queue:");
    std::string moveQueue = "";

    std::queue<Move> moveQueueCopy = this->moveQueue;

    while (moveQueueCopy.size()) {
        moveQueue += moveQueueCopy.front().moveCode() + " ";
        moveQueueCopy.pop();
    }

    ImGui::Text("%s", moveQueue.c_str());

    if (ImGui::Button("Reset")) {
        this->reset();
    }

    ImGui::SameLine();
    if (ImGui::Button("Scramble")) {
        this->shuffle();
    }

    ImGui::SameLine();
    if (ImGui::Button("Flush")) {
        this->flushMoves();
    }
    ImGui::EndChild();


    ImGui::Text("Animation Controls");
    ImGui::BeginChild("Animation Controls", ImVec2(0, 60), true);

    // Slider for turn speed
    ImGui::SliderFloat("Turn speed", &this->turnSpeed, -2.0f, 5.0f);

    if (ImGui::BeginCombo("Turn easing", easingFunctionName.c_str())) {
        for (auto e: EASINGS) {
            auto name = e.first;
            auto func = e.second;

            bool is_selected = (easingFunction == func);

            if (ImGui::Selectable(name.c_str(), is_selected)) {
                easingFunction = func;
                easingFunctionName = name;
            }

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::EndChild();


    ImGui::Text("Cube Info");
    ImGui::BeginChild("Cube Info", ImVec2(0, 100), true);

    FastRubiksCube cube(this->cube);
    if (cube.isValid()) {
        uint32_t cornerHash = cube.getCornerIndex();
        uint64_t edgeHashGroupOne = cube.getPartialEdgeIndex(EDGE_GROUP_ONE);
        uint64_t edgeHashGroupTwo = cube.getPartialEdgeIndex(EDGE_GROUP_TWO);
        uint64_t edgeHashFull = cube.getEdgeIndex();
        uint32_t udSlice = UDSliceCoordinate(cube);

        ImGui::Text("Corner hash: %d", cornerHash);
        ImGui::Text("Edge hash (group one): %llu", edgeHashGroupOne);
        ImGui::Text("Edge hash (group two): %llu", edgeHashGroupTwo);
        ImGui::Text("Edge hash (full): %llu", edgeHashFull);
        ImGui::Text("UD slice: %d", udSlice);
    } else {
        ImGui::Text("Cube is not valid");
    }

    ImGui::EndChild();

    ImGui::Text("Cube Solver");
    ImGui::BeginChild("Cube Solver", ImVec2(0, 120), true);

    if (ImGui::Button("Solve")) {
        this->flushMoves();
        this->launchSolver();
    }

    if (this->solverState == SolverState::RUNNING) {
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            this->stopSolver();
        }
    }

    if (solverState == SolverState::OFF) {
        ImGui::Text("Solver is off");
    } else if (solverState == SolverState::RUNNING) {
        ImGui::Text("Solver is running");
    } else if (solverState == SolverState::STOPPING) {
        ImGui::Text("Solver is stopping");
    }

    solverMutex.lock();

    if (!solverResult.has_value()) {
        ImGui::Text("No solution discovered yet");
    } else {
        std::stringstream sol;
        for (Move move: solverResult.value()) {
            sol << move.moveCode() << " ";
        }

        //Multi-line text box
        ImGui::TextWrapped("Solution: %s", sol.str().c_str());

        if (ImGui::Button("Execute")) {
            pushAllMoves(solverResult.value());
        }
    }

    solverMutex.unlock();

    ImGui::EndChild();

    if (ImGui::Button("Scan cube") && !scanner) {
        scanner = std::move(CubeScanner());
    }

    if (robot.isActive()) {
        ImGui::Text("Connected to robot!");

        if (ImGui::Button("Reset")) {
            robot.resetRobot();
        }
    } else {
        ImGui::Text("Not connected to robot");
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CubeRenderer::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        robot.tick();

        while (!robot.movesExecuted.empty()) {
            moveQueue.push(robot.movesExecuted.front());
            robot.movesExecuted.pop();
        }

        double currentFrameTime = glfwGetTime();
        double dt = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        processInput(dt, currentFrameTime);
        render(dt, currentFrameTime);
        renderImGui();

        if (this->scanner) {
            auto res = scanner->tick();

            if (res) {
                while (!this->moveQueue.empty()) {
                    this->moveQueue.pop();
                }

                this->cube = *res;
                for (int i = 0; i < 6; i++) {
                    colorScheme[i] = scanner->getSideColor((Side) i, cube);
                }

                scanner.reset();
                uploadColorScheme();
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void CubeRenderer::renderCubit(glm::mat4 model, glm::mat4 viewProj, int sides[6]) const {
    glUniform1iv(sidesUniform, 6, sides);
    glUniformMatrix4fv(transformUniform, 1, GL_FALSE, glm::value_ptr(viewProj * model));
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void CubeRenderer::renderCorner(Corner corner, glm::mat4 viewModel, Side currSpin, glm::mat4 spinTransform) {
    int sides[6] = { 6, 6, 6, 6, 6, 6 };
    CornerData cornerData = cube.getCorner(corner);

    glm::vec3 translate = { 0.0f, 0.0f, 0.0f };

    sides[CORNER_UD[corner]] = cornerData.ud;
    sides[CORNER_LR[corner]] = cornerData.lr;
    sides[CORNER_FB[corner]] = cornerData.fb;

    translate += SIDE_NORMALS[CORNER_UD[corner]];
    translate += SIDE_NORMALS[CORNER_LR[corner]];
    translate += SIDE_NORMALS[CORNER_FB[corner]];

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, translate);

    if (CORNER_IS_IN_SIDE[corner][currSpin]) {
        model = spinTransform * model;
    }

    renderCubit(model, viewModel, sides);
}

void CubeRenderer::renderEdge(Edge edge, glm::mat4 viewModel, Side currSpin, glm::mat4 spinTransform) {
    int sides[6] = { 6, 6, 6, 6, 6, 6 };
    EdgeData edgeData = cube.getEdge(edge);
    EdgeData pos = fromEdge(edge);

    glm::vec3 translate = { 0.0f, 0.0f, 0.0f };

    sides[pos.a] = edgeData.a;
    sides[pos.b] = edgeData.b;

    translate += SIDE_NORMALS[pos.a];
    translate += SIDE_NORMALS[pos.b];

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, translate);

    if (EDGE_IS_IN_SIDE[edge][currSpin]) {
        model = spinTransform * model;
    }

    renderCubit(model, viewModel, sides);
}

void CubeRenderer::renderCenter(Side side, glm::mat4 viewModel, Side currSpin, glm::mat4 spinTransform) const {
    int sides[6] = { 6, 6, 6, 6, 6, 6 };
    sides[side] = side;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, SIDE_NORMALS[side]);

    if (currSpin == side) {
        model = spinTransform * model;
    }

    renderCubit(model, viewModel, sides);
}

void CubeRenderer::toggleCameraMode() {
    cameraMode = !cameraMode;
    if (cameraMode) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }
}

void CubeRenderer::onKeyPress(int key, int scancode, int action, int mods) {
    //If press escape toggle camera mode
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        toggleCameraMode();
    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        FastRubiksCube fast(cube);
        cube = reduceTest(fast).toRubiksCube();
    }

    //Detect moves
    if (cameraMode || action != GLFW_PRESS) {
        return;
    }

    MoveType moveType = CLOCKWISE;

    if (mods & GLFW_MOD_SHIFT) {
        if (mods & GLFW_MOD_CONTROL) {
            moveType = DOUBLE_TURN;
        } else {
            moveType = COUNTERCLOCKWISE;
        }
    }

    Move move;
    bool isMove = true;
    switch (key) {
        case GLFW_KEY_F:
            move = {FRONT, moveType};
            break;
        case GLFW_KEY_B:
            move = {BACK, moveType};
            break;
        case GLFW_KEY_U:
            move = {TOP, moveType};
            break;
        case GLFW_KEY_D:
            move = {BOTTOM, moveType};
            break;
        case GLFW_KEY_L:
            move = {LEFT, moveType};
            break;
        case GLFW_KEY_R:
            move = {RIGHT, moveType};
            break;
        default:
            isMove = false;
    }

    if (isMove) {
        pushAllMoves({move});
    }
}

void CubeRenderer::processMoves(double t) {
    if (isMoving) {
        if (t >= turnStartTime + turnDuration) {
            isMoving = false;
            cube.doMove(currentMove);
        }
    }

    if (!isMoving && !moveQueue.empty()) {
        Move move = moveQueue.front();
        moveQueue.pop();

        currentMove = move;
        turnStartTime = t;
        isMoving = true;
        turnDuration = move.moveType == DOUBLE_TURN ? 1 : 0.5;

        turnDuration /= pow(2, turnSpeed);
    }
}

void CubeRenderer::reset() {
    cube = RubiksCube();

    while (!moveQueue.empty()) {
        moveQueue.pop();
    }

    isMoving = false;
}

void CubeRenderer::shuffle() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 17);

    RedundantMovePreventor rmp;

    std::vector<Move> moves;
    for (int i = 0; i < 25; i++) {
        Move move{};

        do {
            move = ALL_MOVES[dis(gen)];
        } while (rmp.isRedundant(move));

        rmp.turnFace(move.side);
        moves.push_back(move);
    }

    pushAllMoves(moves);
}

void CubeRenderer::flushMoves() {
    if (isMoving) {
        cube.doMove(currentMove);
    }

    while (!moveQueue.empty()) {
        cube.doMove(moveQueue.front());
        moveQueue.pop();
    }

    isMoving = false;
}

void CubeRenderer::setCube(RubiksCube cube) {
    this->cube = cube;
}

void CubeRenderer::uploadColorScheme() {
    for (int i = 0; i < 7; i++) {
        glm::vec3 color;

        color.r = this->colorScheme[i].r / 255.0f;
        color.g = this->colorScheme[i].g / 255.0f;
        color.b = this->colorScheme[i].b / 255.0f;

        glUniform3fv(colorSchemeUniforms[i], 1, glm::value_ptr(color));
    }
}

void CubeRenderer::pushAllMoves(std::vector<Move> moves) {
    if (moves.empty()) {
        return;
    }

    if (robot.isActive()) {
        std::stringstream ss;
        for (int i = 0; i < moves.size(); i++) {
            ss << moves[i].moveCode();

            if (i != moves.size() - 1) {
                ss << " ";
            }
        }

        robot.doMoves(ss.str());
    }

    if (!robot.isActive()) {
        for (int i = 0; i < moves.size(); i++) {
            moveQueue.push(moves[i]);
        }
    }
}

void CubeRenderer::launchSolver() {
    if (this->solverState != SolverState::OFF) {
        return;
    }

    this->solverState = SolverState::RUNNING;
    this->solverResult = std::nullopt;
    this->haltSolver = false;

    this->solverThread.reset();
    this->solverThread = std::thread([this]() {
        auto start = std::chrono::high_resolution_clock::now();
        auto optMoves = solve(FastRubiksCube(this->cube), this->haltSolver);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Solved in " << elapsed.count() << "s" << std::endl;
        this->timeTaken = elapsed;

        this->solverState = SolverState::OFF;

        this->solverMutex.lock();
        this->solverResult = optMoves;
        this->solverMutex.unlock();
    });
    this->solverThread->detach();
}

void CubeRenderer::stopSolver() {
    if (this->solverState == SolverState::OFF) {
        return;
    }

    this->haltSolver = true;
    this->solverState = SolverState::STOPPING;
}
