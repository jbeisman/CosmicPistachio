
#include <execution>
#include <algorithm>
#include <cmath>
#include <numeric>

#include <iostream>
#include <iomanip>
#include <fstream>

#include "system.hh"
#include "initial_condition.hh"

#ifdef ENABLE_AVX
#include <immintrin.h>
#endif

template <typename T>
inline T inv_sqrt(const T x)
{
#ifdef ENABLE_CUDA
  return rsqrtf(x);
#else
  __m128 y = _mm_set_ss(x); y = _mm_rsqrt_ss(y); return _mm_cvtss_f32(y);
#endif
}

bool System::setup(int nbodies) {

	if (nbodies % CHUNK != 0) return false;

	this->num_bodies = nbodies;
	
	this->PosX = std::vector<SIMDVec>(this->num_bodies/CHUNK);
	this->PosY = std::vector<SIMDVec>(this->num_bodies/CHUNK);
	this->PosZ = std::vector<SIMDVec>(this->num_bodies/CHUNK);
	
	this->VelX = std::vector<SIMDVec>(this->num_bodies/CHUNK);
	this->VelY = std::vector<SIMDVec>(this->num_bodies/CHUNK);
	this->VelZ = std::vector<SIMDVec>(this->num_bodies/CHUNK);
	
	this->AccX = std::vector<SIMDVec>(this->num_bodies/CHUNK);
	this->AccY = std::vector<SIMDVec>(this->num_bodies/CHUNK);
	this->AccZ = std::vector<SIMDVec>(this->num_bodies/CHUNK);
	
	this->Mass = std::vector<SIMDVec>(this->num_bodies/CHUNK);

	this->Cidx  = std::vector<int>(this->num_bodies/CHUNK);
	std::iota(std::begin(this->Cidx), std::end(this->Cidx), 0);

    rotating_4(*this);

    return true;
}


void System::advance(float timestep) {

	const float half_dt = timestep / 2;
	update_velocities(half_dt);
	update_positions(timestep);

#ifdef ENABLE_AVX
	accumulate_forces_AVX();
#else
	accumulate_forces();
#endif

	update_velocities(half_dt);

	this->elapsed_time += timestep;
}


void System::update_velocities(float timestep) {
  	const float dt{timestep};

	auto *vx = this->VelX.data();
	auto *vy = this->VelY.data();
	auto *vz = this->VelZ.data();
	auto const *ax = this->AccX.data();
	auto const *ay = this->AccY.data();
	auto const *az = this->AccZ.data();

  	std::for_each(std::execution::par_unseq, std::begin(this->Cidx),
  										std::end(this->Cidx), [=](int i) {
  		for (std::size_t j = 0; j < CHUNK; j++) {
			vx[i].data[j] += ax[i].data[j] * dt;
			vy[i].data[j] += ay[i].data[j] * dt;
			vz[i].data[j] += az[i].data[j] * dt;
  		}
  	});
}


void System::update_positions(float timestep) {
  	const float dt{timestep};

	auto *px = this->PosX.data();
	auto *py = this->PosY.data();
	auto *pz = this->PosZ.data();
	auto const *vx = this->VelX.data();
	auto const *vy = this->VelY.data();
	auto const *vz = this->VelZ.data();

  	std::for_each(std::execution::par_unseq, std::begin(this->Cidx),
  									std::end(this->Cidx), [=](int i) {
  		for (std::size_t j = 0; j < CHUNK; j++) {
			px[i].data[j] += vx[i].data[j] * dt;
			py[i].data[j] += vy[i].data[j] * dt;
			pz[i].data[j] += vz[i].data[j] * dt;
  		}
  	});
}


void System::accumulate_forces() {

	auto const *px = this->PosX.data();
	auto const *py = this->PosY.data();
	auto const *pz = this->PosZ.data();
	auto const *ms = this->Mass.data();
	auto *ax = this->AccX.data();
	auto *ay = this->AccY.data();
	auto *az = this->AccZ.data();

	std::size_t NBODS = this->num_bodies;
	std::for_each(std::execution::par_unseq, std::begin(this->Cidx),
									std::end(this->Cidx), [=](int i) {

        for (std::size_t j = 0; j < CHUNK; j++) {
            const float p_x = px[i].data[j];
            const float p_y = py[i].data[j]; 
            const float p_z = pz[i].data[j];
            float r_x = 0.0f;
            float r_y = 0.0f;
            float r_z = 0.0f;

            for (std::size_t ii = 0; ii < NBODS/CHUNK; ii++) {
                for (std::size_t jj = 0; jj < CHUNK; jj++) {
                    float dx = px[ii].data[jj] - p_x;
                    float dy = py[ii].data[jj] - p_y;
                    float dz = pz[ii].data[jj] - p_z;
                    float d2 = dx * dx + dy * dy + dz * dz;
                          d2 += 0.0001;
                    float inv = inv_sqrt(d2);
                    float imp = ms[ii].data[jj] * inv * inv * inv;
                    r_x += dx * imp;
                    r_y += dy * imp;
                    r_z += dz * imp;
                }
            }
            ax[i].data[j] = r_x;
            ay[i].data[j] = r_y;
            az[i].data[j] = r_z;
        }
	});
}



void System::accumulate_forces_AVX() {

#ifdef ENABLE_AVX

	auto const *px = this->PosX.data();
	auto const *py = this->PosY.data();
	auto const *pz = this->PosZ.data();
	auto const *ms = this->Mass.data();
	auto *ax = this->AccX.data();
	auto *ay = this->AccY.data();
	auto *az = this->AccZ.data();

	std::size_t NBODS = this->num_bodies;
	std::for_each(std::execution::par_unseq, std::begin(this->Cidx),
									std::end(this->Cidx), [=](int i) {

		// Softening squared - TODO -- do something abut this
    	constexpr float soft_sqrd = 0.0001f;

    	// Structured 256-bit loads (8 unique floats per __m256)
    	const __m256 p_xi = _mm256_load_ps(&px[i].data[0]);
    	const __m256 p_yi = _mm256_load_ps(&py[i].data[0]);
    	const __m256 p_zi = _mm256_load_ps(&pz[i].data[0]);

    	// Create 256-bit vectors for accumulating accelerations  
    	__m256 result_x = _mm256_setzero_ps();
    	__m256 result_y = _mm256_setzero_ps();
    	__m256 result_z = _mm256_setzero_ps();

    	// Loop through array of 256-bit vectors
    	for (std::size_t j = 0; j < NBODS/CHUNK; j++) {
    	    for (std::size_t k = 0; k < CHUNK; k++) {

    	        // Set all 8 floats in 256-bit vector to the same value
    	        const __m256 p_xj = _mm256_set1_ps(px[j].data[k]);
    	        const __m256 p_yj = _mm256_set1_ps(py[j].data[k]);
    	        const __m256 p_zj = _mm256_set1_ps(pz[j].data[k]);
    	        const __m256 mass = _mm256_set1_ps(ms[j].data[k]);

    	        // Distance between positions (p_x contains 8 unique floats and p_xx contains 8 copies of the same value)
    	        __m256 d_x = _mm256_sub_ps(p_xj, p_xi);
    	        __m256 d_y = _mm256_sub_ps(p_yj, p_yi);
    	        __m256 d_z = _mm256_sub_ps(p_zj, p_zi);

    	        // Square the distance and add the squared softening length
    	        __m256 d_sqrd = _mm256_add_ps( 
    	                        _mm256_add_ps( 
    	                        _mm256_mul_ps(d_x, d_x), 
    	                        _mm256_mul_ps(d_y, d_y)), 
    	                        _mm256_mul_ps(d_z, d_z));
    	        d_sqrd = _mm256_add_ps(d_sqrd, _mm256_set1_ps(soft_sqrd));

    	        // Calculate inv_d_cubed = 1 / d_sqrd^(3/2)
    	        __m256 inv_d = _mm256_rsqrt_ps(d_sqrd); //rsqrt_nr_256(d_sqrd) or rsqrt_nr2_256(d_sqrd);
    	        __m256 inv_d_cubed = _mm256_mul_ps(inv_d,_mm256_mul_ps(inv_d, inv_d ));

    	        // impulse = mass_j * inv_d_cubed
    	        __m256 impulse = _mm256_mul_ps(mass, inv_d_cubed);

    	        // Get acceleration by giving the impulse a direction -- multiply it with d_x
    	        // accel = mass_j (m_1 * r_01) / (d^2 + e^2)^(3/2)
    	        // Accumulate accelerations into result vectors
    	        result_x = _mm256_add_ps(result_x, _mm256_mul_ps( d_x, impulse ) );
    	        result_y = _mm256_add_ps(result_y, _mm256_mul_ps( d_y, impulse ) );
    	        result_z = _mm256_add_ps(result_z, _mm256_mul_ps( d_z, impulse ) );
    	    }
    	}
    	_mm256_store_ps(&ax[i].data[0], result_x);
    	_mm256_store_ps(&ay[i].data[0], result_y);
    	_mm256_store_ps(&az[i].data[0], result_z);
	});
#endif
}

void System::write_points(int filenum) {
  	std::ofstream outfile("velocity_magnitude." + std::to_string(filenum) + ".3D");
  	outfile << std::setprecision(8);
  	outfile << "x y z\n";
  	outfile << "#coordflag xyz\n";
  	for (std::size_t j = 0; j < this->num_bodies/CHUNK; j++) {
    	for (std::size_t k = 0; k < CHUNK; k++) {
  			outfile << this->PosX[j].data[k] << " " << this->PosY[j].data[k] << " "
            << this->PosZ[j].data[k] << "\n";
        }
  	}
}

