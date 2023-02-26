#pragma once

#include "defines.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
public:
  Camera(const glm::vec3 &position = {0.0, 0.0, 3.0});

  Camera(f32 x, f32 y, f32 z);

  void move(const glm::vec3 &offset);

  void rotate(f32 pitch, f32 yaw);

  void reset();

  const glm::vec3 &get_position() const { return _position; }

  const glm::quat &get_orientation() const { return _orientation; }

  f32 get_pitch() const { return _pitch; }
  f32 get_yaw() const { return _yaw; }

  glm::mat4 get_view_matrix();

  glm::vec3 get_front();

  glm::vec3 get_back();

  glm::vec3 get_up();

  glm::vec3 get_down();

  glm::vec3 get_right();

  glm::vec3 get_left();

  f32 get_movement_speed() const { return _movementSpeed; }

  f32 get_rotation_speed() const { return _rotationSpeed; }

private:
  void update_orientation();

private:
  glm::vec3 _position = {0.0, 0.0, 0.0};
  glm::quat _orientation = {1, 0, 0, 0};
  f32 _pitch = 0.0f;
  f32 _yaw = 0.0f;

  f32 _rotationSpeed = 45.0f;
  f32 _movementSpeed = 1.5f;
};
