#include "lighting.hpp"
#include <cassert>
#include <cmath>
int main() {
    using namespace Tiny3D;
    std::vector<Light> lights;
    Light sun;
    sun.type = Light::Type::Directional;
    sun.direction = {0, -1, 0};
    sun.intensity = 1.0f;
    lights.push_back(sun);
    Color c = litColor({200, 100, 50, 255}, {0, 1, 0}, {0,0,0}, lights, 0.1f);
    assert(c.r == 220 && c.g == 110 && c.b == 55);
    Light point;
    point.type = Light::Type::Point;
    point.position = {0, 2, 0};
    point.range = 4;
    point.intensity = 1;
    lights.push_back(point);
    Color near = litColor({100,100,100,255}, {0,1,0}, {0,0,0}, lights, 0.1f);
    Color far = litColor({100,100,100,255}, {0,1,0}, {0,10,0}, lights, 0.1f);
    assert(near.r > far.r);
    return 0;
}
