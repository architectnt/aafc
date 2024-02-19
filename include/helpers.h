#ifndef HELPERS_H
#define HELPERS_H 1

#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(value, lower, upper) (Max(Min(value, upper), lower))

// Constant division (allows faster computation)
inline constexpr float INT16_REC = 1.0f / 32767.0f;
inline constexpr float INT24_REC = 1.0f / 8388607.0f;
inline constexpr float INT8_REC = 1.0f / 127.0f;

inline constexpr float SF8_REC = 1.0f / 15.0f;

#endif