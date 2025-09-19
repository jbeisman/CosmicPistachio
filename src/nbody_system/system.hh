
#pragma once

#include <vector>

#include "simd_vec.hh"

class System {
public:
	System() = default;
	bool setup(int nbodies);
	void advance(float timestep);
	void write_points(int filenum);
	std::vector<SIMDVec> PosX; // Position data
	std::vector<SIMDVec> PosY;
	std::vector<SIMDVec> PosZ;
	std::vector<SIMDVec> VelX; // Velocity data
	std::vector<SIMDVec> VelY;
	std::vector<SIMDVec> VelZ;
	std::vector<SIMDVec> AccX; // Acceleration data
	std::vector<SIMDVec> AccY;
	std::vector<SIMDVec> AccZ;
	std::vector<SIMDVec> Mass; // Mass data
	std::vector<std::size_t> Cidx; // Chunk index
	int num_bodies{0};
	float elapsed_time{0.0};
private:
	void update_velocities(float timestep);
	void update_positions(float timestep);
	void accumulate_forces();
	void accumulate_forces_AVX();
};

