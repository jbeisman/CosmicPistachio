
#include "renderer.hh"

#include <iostream>
#include <filesystem>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL.h>


Renderer::Renderer()
  : simulator(std::make_unique<System>()),
    camera(),
    numbods(0),
    shader_program(0),
    texture_color(0),
    VAO(0),
    VBO(0),
    UBO(0)
{
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
}

void Renderer::init(int NBODS) {
  this->numbods = NBODS;

  this->simulator->setup(NBODS);
  this->simulator->interleave_data();
  
  this->camera = Camera(glm::vec3(0.0f, 0.0f, -2000000.0f),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f));


  // Generate buffers
  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
  glGenBuffers(1, &this->UBO);

  // Bind vertex array
  glBindVertexArray(this->VAO);
  glEnableVertexAttribArray(0);

  // Bind vertex buffer and setup data
  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * NBODS * 3,
    this->simulator->flatPos.data(), GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                        (void *)0);

  // Setup uniform buffer
  glBindBuffer(GL_UNIFORM_BUFFER, this->UBO);
  glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // Bind UBO to binding point 0
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->UBO);

  // Unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Load and compile shaders
  std::filesystem::path base_path = SDL_GetBasePath();
  std::filesystem::path vs_name = base_path / "shaders/vert.glsl";
  std::filesystem::path fs_name = base_path / "shaders/frag.glsl";

  this->shader_program = create_shader_program(vs_name.c_str(), fs_name.c_str());
  glUseProgram(this->shader_program);

  // Add uniform block to linear shader program
  GLuint blockIndex = glGetUniformBlockIndex(this->shader_program, "UBO");
  glUniformBlockBinding(this->shader_program, blockIndex,
                        0);
}

void Renderer::update(float DTIME) {
  //if (!simulator->sim_is_paused()) {

    // Run a timestep if ready
    this->simulator->advance(DTIME);
    this->simulator->interleave_data();

    // Bind new position data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * this->numbods * 3,
                    this->simulator->flatPos.data());
  //}
}

void Renderer::change_color(Color::ColorType color) {
  this->color_map = getColormap(color);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, this->color_map.size(), 0, GL_RGB,
                 GL_FLOAT, this->color_map.data());
}

void Renderer::display(float aspect_ratio) const {

  glUseProgram(this->shader_program);

  // Update UBO data
  glBindBuffer(GL_UNIFORM_BUFFER, this->UBO);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                  glm::value_ptr(this->camera.get_view_matrix()));
  glBufferSubData(
      GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
      glm::value_ptr(this->camera.get_projection_matrix(aspect_ratio)));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // Bind VAO and draw
  glBindVertexArray(this->VAO);
  glDrawArrays(GL_POINTS, 0, this->numbods);
  glBindVertexArray(0);
}

void Renderer::reset_simulator() {
  //this->simulator.reset();
  //this->simulator = std::make_unique<ParticleMeshSimulator>();
}

Renderer::~Renderer() {
  if (glIsVertexArray(this->VAO)) {
    glBindVertexArray(this->VAO);
    glDisableVertexAttribArray(0);
    glDeleteVertexArrays(1, &this->VAO);
  }
  if (glIsTexture(this->texture_color)) {
    glBindTexture(GL_TEXTURE_1D, 0);
    glDeleteTextures(1, &this->texture_color);
  }
  if (glIsProgram(this->shader_program)) {
    glUseProgram(0);
    glDeleteProgram(this->shader_program);
  }
  if (glIsBuffer(this->VBO)) glDeleteBuffers(1, &this->VBO);
  if (glIsBuffer(this->UBO)) glDeleteBuffers(1, &this->UBO);
}

GLuint Renderer::compile_shader(GLenum type, const char *path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open shader file: " + std::string(path));
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string srcStr = buffer.str();
  const char *src = srcStr.c_str();

  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  // Error checking
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    constexpr size_t LOG_SIZE = 512;
    char info[LOG_SIZE];
    glGetShaderInfoLog(shader, LOG_SIZE, nullptr, info);
    std::cerr << "Shader compile error: " << info << std::endl;
    throw std::runtime_error("");
  }

  return shader;
}

GLuint Renderer::create_shader_program(const char *vertexPath,
                                     const char *fragmentPath) {
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vertexPath);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragmentPath);

  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  // Error checking
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    constexpr size_t LOG_SIZE = 512;
    char info[LOG_SIZE];
    glGetProgramInfoLog(program, LOG_SIZE, nullptr, info);
    std::cerr << "Shader link error: " << info << std::endl;
    throw std::runtime_error("");
  }

  glDeleteShader(vs);
  glDeleteShader(fs);
  return program;
}

