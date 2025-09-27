
#pragma once

#include <glm/glm.hpp>

class Camera {
public:
  Camera() = default;
  Camera(glm::vec3 camera_position, glm::vec3 look_at, glm::vec3 up_vec);
  void start_drag(const glm::vec2 &mouse_pos);
  void end_drag();
  void update_drag(const glm::vec2 &mouse_pos, const glm::vec2 &view_size);
  void update_zoom(float scroll_offset);
  glm::mat4x4 get_view_matrix() const;
  glm::mat4x4 get_projection_matrix(float aspect_ratio) const;
  glm::mat4x4 get_model_matrix() const;
  void set_camera_view(glm::vec3 eye);
  void update_view_matrix();
  glm::vec3 get_cam_pos() const;
  glm::vec3 get_up_vec() const;
  glm::vec3 get_LookAt() const;
  glm::vec3 get_view_direction() const;
  glm::vec3 get_right_vec() const;
private:
  glm::mat4x4 view_matrix;
  glm::vec3 cam_pos;
  glm::vec3 up_vector;
  glm::vec3 look_at;
  glm::vec2 prev_mouse_pos;
  bool is_drag{false};
  float zoom_sensitivity{1.0f};
  float zoom_acceleration{1.0f};
  float zoom_acceleration_rate{0.2f};
  float max_zoom_acceleration{5.0};
  double last_zoom_time{0.0};
  double zoom_reset_threshold{400.0};
};
