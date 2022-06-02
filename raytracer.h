#pragma once

#include "types.h"
#include <stddef.h>
#include <algorithm>

#include <glm/glm.hpp>

using namespace glm;

#define VIEWPORT_DISTANCE 1.0f
#define inf std::numeric_limits<float>::infinity()

void draw_pixel(int x, int y, Color color) {
    int new_x = GetScreenWidth() / 2 + x;
    int new_y = GetScreenHeight() / 2 - y;
    DrawPixel(new_x, new_y, color);
}

bool intersect_ray_plane(Xray ray, Plane plane, float *t) {
    *t = dot(plane.point - ray.origin, plane.normal) / dot(ray.D, plane.normal);
    return *t > 0;
}

bool intersect_ray_sphere(Xray ray, Sphere sphere, float *t) {
    vec3 CO = ray.origin - sphere.center;
    float a = dot(ray.D, ray.D);
    float b = 2 * dot(CO, ray.D);
    float c = dot(CO, CO) - sphere.radius * sphere.radius;

    float discriminant = b * b - 4 * a * c;
    float root1 = (-b + sqrtf(discriminant)) / (2 * a);
    float root2 = (-b - sqrtf(discriminant)) / (2 * a);

    *t = inf;
    if (root1 > 0) *t = root1;
    if (root2 > 0 && root2 < root1) *t = root2;

    return (discriminant >= 0 && *t > 0 && *t < inf);
}

bool is_in_shadow(World *world, Xcamera *camera, vec3 point, LightSource light) {
    vec3 light_vec;
    float closest_t = inf;

    if (light.type == DIRECTIONAL_LIGHT) {
        light_vec = light.direction;
    } else { // POINT LIGHT
        light_vec = light.position - point;
    }

    Xray light_ray;
    light_ray.origin = point;
    light_ray.D = light_vec;

    float t;
    for (int i = 0; i < world->planes_count; ++i) {
        if (intersect_ray_plane(light_ray, world->planes[i], &t)) {
            if (t < closest_t && t > 0.001f) {
                closest_t = t;
            }
        }
    }

    for (int i = 0; i < world->spheres_count; ++i) {
        if (intersect_ray_sphere(light_ray, world->spheres[i], &t)) {
            if (t < closest_t && t > 0.001f) {
                closest_t = t;
            }
        }
    }

    return (light.type == DIRECTIONAL_LIGHT && closest_t != inf) || closest_t < 1.0f;
}

float compute_lighting(World *world, Xcamera *camera, vec3 point, vec3 normal, int specular) {
    LightSource *light_sources = world->light_sources;
    size_t lights_count  = world->light_sources_count;

    float intensity = 0.0f;

    for (int i = 0; i < lights_count; ++i) {
        if (light_sources[i].type == AMBIENT_LIGHT) {
            intensity += light_sources[i].intensity;
        } else {
            vec3 light_vec;
            if (light_sources[i].type == POINT_LIGHT) {
                light_vec = light_sources[i].position - point;
            } else { // DIRECTIONAL LIGHT
                light_vec = light_sources[i].direction;
            }

            if (!is_in_shadow(world, camera, point, light_sources[i])) {
                float current_intensity = dot(normalize(light_vec), normal) * light_sources[i].intensity;

                if (specular != -1) {
                    vec3 proj = normal * dot(light_vec, normal);
                    vec3 complement = light_vec - proj;
                    vec3 R = proj - complement;
                    current_intensity += current_intensity * (float)pow(dot(normalize(R), normalize(camera->pos - point)), specular);
                }

                if (current_intensity > 0.0f) {
                    intensity += current_intensity;
                }
            }
        }
    }
    
    return intensity;
}

void apply_lighting(Color *color, float intensity) {
    color->r = std::min((int)(color->r * intensity), 255);
    color->g = std::min((int)(color->g * intensity), 255);
    color->b = std::min((int)(color->b * intensity), 255);
}

Color trace_ray(World *world, Xcamera* camera, Xray ray, float min_t, float max_t) {
    Color pix_color = { 7, 11, 52, 255 };
    vec3 normal;

    float closest_t = inf;
    Object *closest_object = NULL;

    float t = 0;
    for (int i = 0; i < world->planes_count; ++i) {
        if (intersect_ray_plane(ray, world->planes[i], &t)) {
            if (t < closest_t && t >= min_t && t < max_t) {
                closest_t = t;
                closest_object = &world->planes[i];
                normal = normalize(world->planes[i].normal);
            }
        }
    }

    for (int i = 0; i < world->spheres_count; ++i) {
        if (intersect_ray_sphere(ray, world->spheres[i], &t)) {
            if (t < closest_t && t >= min_t && t < max_t) {
                closest_t = t;
                closest_object = &world->spheres[i];
                vec3 hit_point = ray.origin + ray.D * t;
                normal = normalize(hit_point - world->spheres[i].center);
            }
        }
    }

    if (closest_object) {
        pix_color = closest_object->color;
        float specular = closest_object->specular;
        vec3 hit_point = ray.origin + ray.D * closest_t;
        apply_lighting(&pix_color, compute_lighting(world, camera, hit_point, normal, specular));
    }

    return pix_color;
}

vec3 map_screen_to_viewport(int x, int y, Xcamera *camera) {
    vec3 result = camera->pos;
    result -= camera->z_basis * VIEWPORT_DISTANCE;
    float aspect_ratio = (float)GetScreenWidth() / GetScreenHeight();
    result += camera->y_basis * ((float)y / GetScreenHeight());
    result -= camera->x_basis * ((float)x / GetScreenWidth()) * aspect_ratio;
    return result;
}

void camera_look_at(Xcamera *camera, vec3 target) {
    camera->z_basis = normalize(-target);
    camera->x_basis = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera->z_basis));
    camera->y_basis = normalize(cross(camera->z_basis, camera->x_basis));
}

vec3 euler_angles_to_vec3(float pitch, float yaw) {
    float x = cos(radians(yaw)) * cos(radians(pitch));
    float y = sin(radians(pitch));
    float z = sin(radians(yaw)) * cos(radians(pitch));
    return vec3(x, y, z);
}

void clamp_euler_angles(float *pitch, float *yaw) {
    if (*pitch > 89) *pitch = 89;
    if (*pitch < -89) *pitch = -89;

    while (*yaw < -180) *yaw += 360;
    while (*yaw > 180) *yaw -= 360;
}