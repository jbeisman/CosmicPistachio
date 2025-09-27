
#pragma once
#include <vector>
// #include <GL/glew.h>
#include <glm/glm.hpp>

// Enum to specify the desired colormap

namespace Color {
enum ColorType {
  Magma,
  BlueOrange,
  Viridis,
  Plasma,
  Rainbow,

};
}

std::vector<glm::vec3> getColormap(Color::ColorType type);
