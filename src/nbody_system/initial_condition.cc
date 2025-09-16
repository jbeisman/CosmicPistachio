
#include <algorithm>
#include <execution>
#include <random>

#include <iostream>

#include "math_functions.hh"
#include "initial_condition.hh"
#include "simd_vec.hh"

namespace {
std::random_device rd;
std::mt19937 rg(rd());
std::uniform_real_distribution<float> dist(-1.f, 1.f);
std::lognormal_distribution<float> lgdist(0.0f, 1.0f);
static constexpr float PI = 3.1415926f;
}; // namespace


// Generate disc of particles
std::vector<Vec3<float>> generate_frisbee(int n_bodies, float rad) {
  auto randomGenerator = [=]() {
    Vec3<float> p;
    const float theta = PI * 2 * dist(rg);
    const float radius = dist(rg) * rad;
    p.x = cos(theta) * radius;
    p.y = sin(theta) * radius;
    p.z = dist(rg) * rad / 10.f;
    return p;
  };
  std::vector<Vec3<float>> particles(n_bodies);
  std::generate(particles.begin(), particles.end(), randomGenerator);
  return particles;
}


template <typename T> std::vector<T> generate_random_mass(int n_bodies) {
  auto randomGenerator = [=]() {
    T mass = 1.0f + lgdist(rg) * 1e4f;
    return mass;
  };
  std::vector<T> mass(n_bodies, 0.0f);
  std::generate(mass.begin(), mass.end(), randomGenerator);
  return mass;
}


template <class vecT, typename T>
std::vector<vecT> orbital_velocity(const std::vector<vecT> &pos,
                                   const T large_mass) {
  std::vector<vecT> vel(pos.size(), vecT());
  std::transform(std::execution::par_unseq, std::begin(pos), std::end(pos),
                 std::begin(vel), [=](const vecT &pos) {
                   vecT v = cross_product(pos, vecT(0.f, 0.f, 1.f));
                   T orbital_vel = sqrtf(large_mass / magnitude(v));
                   v = normalize(v) * orbital_vel;
                   return v;
                 });
  return vel;
}


template <class vecT, typename T>
std::vector<vecT> generate_hollow_sphere(int n_bodies, T rad) {
  auto randomGenerator = [=]() {
    vecT p;
    const T theta = PI * 2 * dist(rg);
    const T phi = PI * dist(rg);
    const T radius = rad + dist(rg) * rad / 100.f;
    p.x = sin(theta) * cos(phi) * radius;
    p.y = sin(theta) * sin(phi) * radius;
    p.z = cos(theta) * radius;
    return p;
  };
  std::vector<vecT> particles(n_bodies, vecT());
  std::generate(particles.begin(), particles.end(), randomGenerator);
  return particles;
}


template <class vecT> std::vector<vecT> generate_two_sphere(int n_bodies) {
  const vecT com1(30000.0f, 0.0f, 0.0f);
  const vecT com2(-30000.0f, 0.0f, 0.0f);
  const int half = n_bodies / 2;
  const int other_half = n_bodies - half;
  std::vector<vecT> p1 = generate_hollow_sphere<vecT>(half, 20000.f);
  std::vector<vecT> p2 = generate_hollow_sphere<vecT>(other_half, 20000.f);
  std::transform(std::execution::par_unseq,
                p1.begin(), p1.end(), p1.begin(),
                [=](auto& p) { return p + com1; });
  std::transform(std::execution::par_unseq,
                p2.begin(), p2.end(), p2.begin(),
                [=](auto& p) { return p + com2; });
  p1.insert(p1.end(), p2.begin(), p2.end());
  return p1;
}


// create "galaxy" from two spherical shells that orbit a larger mass
void add_galaxy_to_system(
    std::vector<Vec3<float>>& tmp_sysPos,
    std::vector<Vec3<float>>& tmp_sysVel,
    std::vector<float>& tmp_sysMss,
    const Vec3<float> &center,
    int n_bodies, float large_mass)
{
  // get random mass distribution, positions, and orbital velocities
  std::vector<float> m = generate_random_mass<float>(n_bodies);
  // arrange as two hollow spheres centered and opposed about a large mass
  std::vector<Vec3<float>> p = generate_two_sphere<Vec3<float>>(n_bodies);
  // calculate rotational velocity of two-sphere system about 0,0,0 (local) coordinate
  std::vector<Vec3<float>> v = orbital_velocity(p, large_mass);
  // set first position of two-sphere system to center of galaxy
  p[0] = {0.f, 0.f, 0.f};
  v[0] = {0.f, 0.f, 0.f};
  m[0] = large_mass;

  // shift two-sphere positions by center coordinate
  std::transform(std::execution::par_unseq,
                p.begin(), p.end(), p.begin(),
                [=](auto& pos) { return pos + center; });

  // insert two-sphere into system vectors
  tmp_sysPos.insert(tmp_sysPos.end(), p.begin(), p.end());
  tmp_sysVel.insert(tmp_sysVel.end(), v.begin(), v.end());
  tmp_sysMss.insert(tmp_sysMss.end(), m.begin(), m.end());
}


// create system with 4 "galaxies" orbiting a large mass
void rotating_4(System &system) {

  // split nbodies into 4 groups
  const int quad = system.num_bodies / 4;
  const int last_quad = system.num_bodies - quad * 3;

  // assign subgroup centers at corners
  const Vec3<float> center1(400000.0f, 400000.0f, 0.0f);
  const Vec3<float> center2(-400000.0f, -400000.0f, 0.0f);
  const Vec3<float> center3(400000.0f, -400000.0f, 0.0f);
  const Vec3<float> center4(-400000.0f, 400000.0f, 0.0f);

  // temporary system vectors
  auto tmp_sysPos  =  std::vector<Vec3<float>>();
  auto tmp_sysVel  = std::vector<Vec3<float>>();
  auto tmp_sysMss = std::vector<float>();
  // add "galaxies" to system
  add_galaxy_to_system(tmp_sysPos, tmp_sysVel, tmp_sysMss, center1, quad, 1.e10f);
  add_galaxy_to_system(tmp_sysPos, tmp_sysVel, tmp_sysMss, center2, quad, 1.e10f);
  add_galaxy_to_system(tmp_sysPos, tmp_sysVel, tmp_sysMss, center3, quad, 1.e10f);
  add_galaxy_to_system(tmp_sysPos, tmp_sysVel, tmp_sysMss, center4, last_quad, 1.e10f);

  // calculate orbital velocity of system about global 0,0,0 coordinate
  // add it to system velocity
  std::vector<Vec3<float>> orb_vel = orbital_velocity(tmp_sysPos, 1e13f);
  std::transform(std::execution::par_unseq,
                orb_vel.begin(), orb_vel.end(),
                tmp_sysVel.begin(), tmp_sysVel.begin(),
                [=](const auto& ov, auto& sv) { return sv + ov; });
  // set last position to center of system
  tmp_sysPos[system.num_bodies-1] = {0.f, 0.f, 0.f};
  tmp_sysVel[system.num_bodies-1] = {0.f, 0.f, 0.f};
  tmp_sysMss[system.num_bodies-1] = 1.e13f;

    for (std::size_t ii = 0; ii < system.num_bodies/CHUNK; ii++) {
        for (std::size_t jj = 0; jj < CHUNK; jj++) {
          std::size_t idx = ii * CHUNK + jj;
          
          system.PosX[ii].data[jj] = tmp_sysPos[idx].x;
          system.PosY[ii].data[jj] = tmp_sysPos[idx].y;
          system.PosZ[ii].data[jj] = tmp_sysPos[idx].z;

          system.VelX[ii].data[jj] = tmp_sysVel[idx].x;
          system.VelY[ii].data[jj] = tmp_sysVel[idx].y;
          system.VelZ[ii].data[jj] = tmp_sysVel[idx].z;

          system.Mass[ii].data[jj] = tmp_sysMss[idx];
        }
    }
}

