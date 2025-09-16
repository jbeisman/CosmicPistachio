
#pragma once

#ifdef ENABLE_AVX
    constexpr std::size_t ALIGN = 32;
    constexpr std::size_t CHUNK = 8;
#elif defined ENABLE_CUDA
    constexpr std::size_t ALIGN = 4;
    constexpr std::size_t CHUNK = 1;
#else
    constexpr std::size_t ALIGN = 16;
    constexpr std::size_t CHUNK = 4;
#endif


struct alignas(ALIGN) SIMDVec {
    float data[CHUNK] = {};
};
