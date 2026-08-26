#pragma once
#include "engine.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace Tiny3D {

inline float saturate(float v) { return std::clamp(v, 0.0f, 1.0f); }
inline Vec3 multiply(Vec3 a, Vec3 b) { return {a.x*b.x,a.y*b.y,a.z*b.z}; }
inline Vec3 reflect(Vec3 incident, Vec3 normal) {
    return incident - normal * (2.0f * dot(incident, normal));
}

// Small, deterministic Blinn-Phong lighting model used by the software renderer.
// It deliberately keeps the API simple while providing much better falloff and
// highlights than the original fixed face-shading approximation.
struct LightingSettings {
    Color ambientColor{38, 42, 52, 255};
    float ambientStrength = 0.22f;
    float specularStrength = 0.28f;
    float shininess = 32.0f;
};

inline Color litColor(Color base, Vec3 normal, Vec3 worldPosition,
                      const std::vector<Light>& lights,
                      Vec3 viewPosition = {0,0,0},
                      const LightingSettings& settings = {}) {
    const Vec3 n = normalize(normal);
    const Vec3 viewDir = normalize(viewPosition - worldPosition);

    float r = (settings.ambientColor.r / 255.0f) * settings.ambientStrength;
    float g = (settings.ambientColor.g / 255.0f) * settings.ambientStrength;
    float b = (settings.ambientColor.b / 255.0f) * settings.ambientStrength;

    for (const auto& light : lights) {
        Vec3 lightDir{};
        float attenuation = 1.0f;

        if (light.type == Light::Type::Directional) {
            lightDir = normalize(light.direction * -1.0f);
        } else {
            const Vec3 delta = light.position - worldPosition;
            const float distance = length(delta);
            const float range = std::max(0.001f, light.range);
            if (distance >= range) continue;

            lightDir = normalize(delta);
            // Smooth, bounded range falloff. This avoids the harsh edge of the
            // previous linear-square approximation.
            const float x = saturate(distance / range);
            const float smooth = 1.0f - x * x * (3.0f - 2.0f * x);
            attenuation = smooth * smooth;
        }

        const float ndotl = std::max(0.0f, dot(n, lightDir));
        const float diffuse = ndotl * std::max(0.0f, light.intensity) * attenuation;

        const Vec3 halfDir = normalize(lightDir + viewDir);
        const float specular = std::pow(std::max(0.0f, dot(n, halfDir)),
                                        std::max(1.0f, settings.shininess))
                             * settings.specularStrength
                             * std::max(0.0f, light.intensity)
                             * attenuation;

        const float lr = light.color.r / 255.0f;
        const float lg = light.color.g / 255.0f;
        const float lb = light.color.b / 255.0f;
        r += lr * (diffuse + specular);
        g += lg * (diffuse + specular);
        b += lb * (diffuse + specular);
    }

    return {
        static_cast<uint8_t>(std::clamp(base.r * r, 0.0f, 255.0f)),
        static_cast<uint8_t>(std::clamp(base.g * g, 0.0f, 255.0f)),
        static_cast<uint8_t>(std::clamp(base.b * b, 0.0f, 255.0f)),
        base.a
    };
}

} // namespace Tiny3D
