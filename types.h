#pragma once

#include "raylib.h"
#include <glm/glm.hpp>

using namespace glm;

struct Object {
    Color color;
    int specular;
};

struct Plane : public Object {
    vec3 point;
    vec3 normal;
};

struct Sphere : public Object {
    float radius;
    vec3 center;
};

enum LightSourceType {
    POINT_LIGHT = 0,
    DIRECTIONAL_LIGHT,
    AMBIENT_LIGHT
};

struct LightSource {
    LightSourceType type;
    vec3 position;
    vec3 direction;
    float intensity;
};

struct World {
    size_t planes_count;
    Plane *planes;

    size_t spheres_count;
    Sphere *spheres;

    size_t light_sources_count;
    LightSource *light_sources;
};

struct Xcamera {
    vec3 pos;
    vec3 x_basis;
    vec3 y_basis;
    vec3 z_basis;
};

struct Xray {
    vec3 origin;
    vec3 D;
};
