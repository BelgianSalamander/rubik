//
// Created by Anatol on 26/12/2022.
//

#ifndef RUBIK_CAMERA_H
#define RUBIK_CAMERA_H

#include "glm/glm.hpp"

static const glm::vec3 WORLD_UP = glm::vec3(0, 1, 0);

class Camera {
public:
    Camera();

    void update(float dt);
    void mouseMove(float dx, float dy);

    glm::mat4 getViewMatrix();
private:
    float yaw = 0;
    float pitch = 0;
    float roll = 0;

    glm::vec3 position = glm::vec3(0, 0, 0);

    glm::vec3 getForward() const;
};


#endif //RUBIK_CAMERA_H
