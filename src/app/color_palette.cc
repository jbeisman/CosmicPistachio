
#include "color_palette.hh"
#include <iostream>

std::vector<glm::vec3> getColormap(Color::ColorType type) {
  std::vector<glm::vec3> color_map;

  switch (type) {
  case Color::ColorType::Viridis:
    // Viridis-like gradient
    color_map.push_back(glm::vec3(0.267f, 0.005f, 0.329f)); // Dark Purple
    color_map.push_back(glm::vec3(0.198f, 0.297f, 0.528f)); // Dark Blue
    color_map.push_back(glm::vec3(0.127f, 0.561f, 0.553f)); // Teal
    color_map.push_back(glm::vec3(0.321f, 0.760f, 0.443f)); // Green
    color_map.push_back(glm::vec3(0.723f, 0.817f, 0.280f)); // Lime Green
    color_map.push_back(glm::vec3(0.993f, 0.906f, 0.145f)); // Yellow
    break;
  case Color::ColorType::Magma:
    // Magma-like gradient
    color_map.push_back(glm::vec3(0.996f, 0.992f, 0.812f)); // Light Yellow
    color_map.push_back(glm::vec3(0.992f, 0.707f, 0.268f)); // Light Orange
    color_map.push_back(glm::vec3(0.819f, 0.376f, 0.247f)); // Orange
    color_map.push_back(glm::vec3(0.505f, 0.170f, 0.463f)); // Purple
    color_map.push_back(glm::vec3(0.086f, 0.071f, 0.301f)); // Dark Blue
    color_map.push_back(glm::vec3(0.000f, 0.000f, 0.000f)); // Black
    break;
  case Color::ColorType::Plasma:
    // Plasma-like colormap
    color_map.push_back(glm::vec3(0.051f, 0.012f, 0.267f)); // Dark Blue
    color_map.push_back(glm::vec3(0.337f, 0.016f, 0.498f)); // Purple
    color_map.push_back(glm::vec3(0.710f, 0.059f, 0.506f)); // Red-Purple
    color_map.push_back(glm::vec3(0.922f, 0.306f, 0.365f)); // Orange-Red
    color_map.push_back(glm::vec3(0.976f, 0.612f, 0.188f)); // Orange
    color_map.push_back(glm::vec3(0.996f, 0.890f, 0.0f));   // Yellow
    break;
  case Color::ColorType::Rainbow:
    // Rainbow gradient
    color_map.push_back(glm::vec3(0.0f, 0.0f, 1.0f)); // Blue
    color_map.push_back(glm::vec3(0.0f, 0.5f, 1.0f)); // Light Blue
    color_map.push_back(glm::vec3(0.0f, 1.0f, 0.0f)); // Green
    color_map.push_back(glm::vec3(1.0f, 1.0f, 0.0f)); // Yellow
    color_map.push_back(glm::vec3(1.0f, 0.5f, 0.0f)); // Orange
    color_map.push_back(glm::vec3(1.0f, 0.0f, 0.0f)); // Red
    break;
  case Color::ColorType::BlueOrange:
    // Simple two-tone gradient
    color_map.push_back(glm::vec3(0.976f, 0.612f, 0.188f)); // Orange
    color_map.push_back(glm::vec3(0.0f, 0.5f, 1.0f));       // Light Blue
    break;
  default:
    // Return an empty vector for an unrecognized type
    std::cerr << "Warning: Unknown colormap type requested. Returning empty "
                 "vector.\n";
    break;
  }

  return color_map;
}