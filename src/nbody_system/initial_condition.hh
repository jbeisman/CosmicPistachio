
#pragma once

#include <vector>
#include "system.hh"
#include "vec.hh"

std::vector<Vec3<float>> generate_frisbee(int n_bodies, float rad);

void rotating_4(System &system);
