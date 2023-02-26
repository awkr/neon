#include "camera.h"
#include <cstdio>

Camera::Camera(const glm::vec3 &position) : _position{position} {}

Camera::Camera(f32 x, f32 y, f32 z) : _position{x, y, z} {}

void Camera::move(const glm::vec3 &offset) { _position += offset; }

void Camera::rotate(f32 pitch, f32 yaw) {
  _pitch += pitch;
  _yaw += yaw;

  update_orientation();
}

void Camera::reset() {
  _position = glm::vec3(0.0, 0.0, 3.0);
  _pitch = 0;
  _yaw = 0;
  update_orientation();
}

glm::mat4 Camera::get_view_matrix() {
  // auto rotation = glm::mat4_cast(orientation);
  // auto translation = glm::translate(glm::mat4(1.0), cameraPos);
  //
  // view = glm::inverse(rotation * translation);

  auto rotation = glm::mat4_cast(glm::conjugate(_orientation));
  auto translation = glm::translate(glm::mat4(1.0), -_position);
  auto view = rotation * translation;
  return view;
}

glm::vec3 Camera::get_front() { return _orientation * glm::vec3(0.0, 0.0, -1.0); }

glm::vec3 Camera::get_back() { return -get_front(); }

glm::vec3 Camera::get_up() { return _orientation * glm::vec3(0.0, 1.0, 0.0); }

glm::vec3 Camera::get_down() { return -get_up(); }

glm::vec3 Camera::get_right() { return glm::normalize(glm::cross(get_front(), get_up())); }

glm::vec3 Camera::get_left() { return -get_right(); }

void Camera::update_orientation() {
  auto aroundY = glm::angleAxis(glm::radians(_yaw), glm::vec3(0, 1, 0));
  auto aroundX = glm::angleAxis(glm::radians(_pitch), glm::vec3(1, 0, 0));

  _orientation = aroundY * aroundX;
}
