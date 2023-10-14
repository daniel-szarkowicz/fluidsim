#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Camera {
public:
    float fov_degrees;
    float aspect_ratio;
    float near_clip;
    float far_clip;

    glm::vec3 position;
    glm::vec3 look_at;
    glm::vec3 up;

    Camera(float fov_degrees, float aspect_ratio, float near_clip,
           float far_clip, glm::vec3 position, glm::vec3 look_at,
           glm::vec3 up = glm::vec3(0, 1, 0))
        : fov_degrees(fov_degrees), aspect_ratio(aspect_ratio),
          near_clip(near_clip), far_clip(far_clip), position(position),
          look_at(look_at), up(up) {}

    glm::mat4 view_projection() { return projection() * view(); }

    glm::mat4 projection() {
        return glm::perspective(glm::radians(fov_degrees), aspect_ratio,
                                near_clip, far_clip);
    }

    glm::mat4 view() { return glm::lookAt(position, look_at, up); }

    void transform_position(glm::mat4 transform) {
        position = glm::vec3(glm::vec4(position, 1.0f) * transform);
    }
};
