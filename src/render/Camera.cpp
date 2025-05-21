//
// Created by Anatol on 26/12/2022.
//

#include "Camera.h"

#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"

Camera::Camera()
    : position(2, 2, 2)
{

}

void Camera::update(float dt) {
    glm::vec3 eyeDir = getForward();

    glm::vec3 left = glm::cross(WORLD_UP, eyeDir);
    glm::vec3 forward = glm::cross(left, WORLD_UP);

    //Normalize
    left = glm::normalize(left);
    forward = glm::normalize(forward);

    const float SPEED = 4;

    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS) {
        position += forward * dt * SPEED;
    }

    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS) {
        position -= forward * dt * SPEED;
    }

    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) == GLFW_PRESS) {
        position += left * dt * SPEED;
    }

    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) == GLFW_PRESS) {
        position -= left * dt * SPEED;
    }

    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS) {
        position += WORLD_UP * dt * SPEED;
    }

    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        position -= WORLD_UP * dt * SPEED;
    }
}

void Camera::mouseMove(float dx, float dy) {
    const float SCALE = 0.5f;

    yaw += dx * SCALE;
    pitch += dy * SCALE;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }

    if (pitch < -89.0f) {
        pitch = -89.0f;
    }
}

glm::mat4 Camera::getViewMatrix() {
    glm::vec3 eyeDir = getForward();

    return glm::lookAt(position, position + eyeDir, WORLD_UP);
}

glm::vec3 Camera::getForward() const {
    glm::vec3 forward;
    forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    forward.y = sin(glm::radians(pitch));
    forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    return glm::normalize(forward);
}
