
#pragma once

#include "color_palette.hh"
#include "camera.hh"
#include "system.hh"

#include <fstream>
#include <memory>
#include <vector>

#include <GL/glew.h>

class Renderer {
public:
  Renderer();
  ~Renderer();
  void init(int NBODS);
  void change_color(Color::ColorType color);
  void update(float DTIME);
  void display(float aspect_ratio) const;
  void reset_simulator();
  std::unique_ptr<System> simulator;
  Camera camera;
private:
  GLuint compile_shader(GLenum type, const char *path);
  GLuint create_shader_program(const char *vertexPath, const char *fragmentPath);
  int numbods;
  std::vector<glm::vec3> color_map;
  GLuint shader_program;
  GLuint texture_color;
  GLuint VAO;
  GLuint VBO;
  GLuint UBO;
  GLuint color_texture_loc;
};
