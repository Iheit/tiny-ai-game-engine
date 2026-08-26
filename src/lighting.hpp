#pragma once
#include "engine.hpp"
#include <cmath>
#include <vector>

namespace Tiny3D {

struct Light {
    enum class Type { Directional, Point };
    Type type = Type::Directional;
    Vec3 position{0, 4, 0};
    Vec3 direction{-0.35f, -0.8f, -0.45f};
    Color color{255, 244, 220, 255};
    float intensity = 1.0f;
    float range = 10.0f;
};

inline float saturate(float v) { return std::clamp(v, 0.0f, 1.0f); }
inline Vec3 multiply(Vec3 a, Vec3 b) { return {a.x*b.x,a.y*b.y,a.z*b.z}; }
inline Color litColor(Color base, Vec3 normal, Vec3 worldPosition,
                       const std::vector<Light>& lights, float ambient = 0.16f) {
    Vec3 n = normalize(normal);
    float r = ambient, g = ambient, b = ambient;
    for (const auto& light : lights) {
        Vec3 l;
        float attenuation = 1.0f;
        if (light.type == Light::Type::Directional) {
            l = normalize(light.direction * -1.0f);
        } else {
            Vec3 delta = light.position - worldPosition;
            float d = length(delta);
            if (d >= light.range) continue;
            l = normalize(delta);
            float q = 1.0f - d / std::max(0.001f, light.range);
            attenuation = q*q;
        }
        float ndotl = std::max(0.0f, dot(n, l));
        float contribution = ndotl * light.intensity * attenuation;
        r += (light.color.r / 255.0f) * contribution;
        g += (light.color.g / 255.0f) * contribution;
        b += (light.color.b / 255.0f) * contribution;
    }
    return {
        static_cast<uint8_t>(std::clamp(base.r * r, 0.0f, 255.0f)),
        static_cast<uint8_t>(std::clamp(base.g * g, 0.0f, 255.0f)),
        static_cast<uint8_t>(std::clamp(base.b * b, 0.0f, 255.0f)),
        base.a
    };
}

} // namespace Tiny3D
