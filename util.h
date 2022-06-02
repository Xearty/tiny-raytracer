#pragma once

#include "types.h"
#include <stdio.h>
#include <stdlib.h>

char *read_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");

    if (file) {
        fseek(file, 0, SEEK_END);
        size_t filesize = ftell(file);
        rewind(file);

        char *buffer = (char *)malloc(filesize + 1);
        if (buffer) {
            fread(buffer, sizeof(char), filesize, file);
            buffer[filesize] = '\0';
        }

        fclose(file);
        return buffer;
    }

    return NULL;
}

static Color json_array_to_color(json_array_s *array) {
    assert(array->length == 3);
    json_array_element_s *start = array->start;
    
    return {
        (unsigned char)atoi(json_value_as_number(start->value)->number),
        (unsigned char)atoi(json_value_as_number(start->next->value)->number),
        (unsigned char)atoi(json_value_as_number(start->next->next->value)->number),
        255
    };
}

static vec3 json_array_to_vec3(json_array_s *array) {
    assert(array->length == 3);
    json_array_element_s *start = array->start;

    vec3 result = vec3(
        atof(json_value_as_number(start->value)->number),
        atof(json_value_as_number(start->next->value)->number),
        atof(json_value_as_number(start->next->next->value)->number)
    );

    return result;
}

bool load_world(World *world, const char *filepath) {
    char *file_content = read_file(filepath);

    if (!file_content) return false;

    json_parse_result_s result;
    size_t flags_bitset = json_parse_flags_default | json_parse_flags_allow_c_style_comments;
    json_value_s *root = json_parse_ex(file_content, strlen(file_content), flags_bitset, NULL, NULL, &result);
    free(file_content);

    if (result.error != json_parse_error_none) return false;

    json_object_s *root_object = (json_object_s *)root->payload;
    json_object_element_s *element = root_object->start;

    for (json_object_element_s *element = root_object->start; element; element = element->next) {
        if (element->value->type == json_type_array) {
            json_array_s *array = (json_array_s *)element->value->payload;
            const char *name = element->name->string;

            if (strcmp(name, "planes") == 0) {
                world->planes_count = array->length;
                world->planes = (Plane *)malloc(sizeof(Plane) * world->planes_count);

                int idx = 0;
                for (json_array_element_s *arr_el = array->start; arr_el; arr_el = arr_el->next, idx++) {
                    assert(arr_el->value->type == json_type_object);
                    json_object_s *object = (json_object_s *)arr_el->value->payload;
                    for (json_object_element_s *object_el = object->start; object_el; object_el = object_el->next) {
                        assert(object_el->value->type != json_type_object);

                        if (strcmp(object_el->name->string, "color") == 0) {
                            assert(object_el->value->type == json_type_array);
                            json_array_s *color_array = (json_array_s *)object_el->value->payload;
                            world->planes[idx].color = json_array_to_color(color_array);
                        }
                        else if (strcmp(object_el->name->string, "specular") == 0) {
                            assert(object_el->value->type == json_type_number);
                            world->planes[idx].specular = atoi(json_value_as_number(object_el->value)->number);
                        }
                        else if (strcmp(object_el->name->string, "point") == 0) {
                            assert(object_el->value->type == json_type_array);
                            json_array_s *point_array = (json_array_s *)object_el->value->payload;
                            world->planes[idx].point = json_array_to_vec3(point_array);
                        }
                        else if (strcmp(object_el->name->string, "normal") == 0) {
                            assert(object_el->value->type == json_type_array);
                            json_array_s *normal_array = (json_array_s *)object_el->value->payload;
                            world->planes[idx].normal = json_array_to_vec3(normal_array);
                        }
                    }
                }
            }
            else if (strcmp(name, "spheres") == 0) {
                world->spheres_count = array->length;
                world->spheres = (Sphere *)malloc(sizeof(Sphere) * world->spheres_count);

                int idx = 0;
                for (json_array_element_s *arr_el = array->start; arr_el; arr_el = arr_el->next, idx++) {
                    assert(arr_el->value->type == json_type_object);
                    json_object_s *object = (json_object_s *)arr_el->value->payload;
                    for (json_object_element_s *object_el = object->start; object_el; object_el = object_el->next) {
                        assert(object_el->value->type != json_type_object);

                        if (strcmp(object_el->name->string, "color") == 0) {
                            assert(object_el->value->type == json_type_array);
                            json_array_s *color_array = (json_array_s *)object_el->value->payload;
                            world->spheres[idx].color = json_array_to_color(color_array);
                        }
                        else if (strcmp(object_el->name->string, "specular") == 0) {
                            assert(object_el->value->type == json_type_number);
                            world->spheres[idx].specular = atoi(json_value_as_number(object_el->value)->number);
                        }
                        else if (strcmp(object_el->name->string, "center") == 0) {
                            assert(object_el->value->type == json_type_array);
                            json_array_s *center_array = (json_array_s *)object_el->value->payload;
                            world->spheres[idx].center = json_array_to_vec3(center_array);
                        }
                        else if (strcmp(object_el->name->string, "radius") == 0) {
                            assert(object_el->value->type == json_type_number);
                            world->spheres[idx].radius = (float)atof(json_value_as_number(object_el->value)->number);
                        }
                    }
                }
            }
            else if (strcmp(name, "light_sources") == 0) {
                world->light_sources_count = array->length;
                world->light_sources= (LightSource *)malloc(sizeof(LightSource) * world->light_sources_count);

                int idx = 0;
                for (json_array_element_s *arr_el = array->start; arr_el; arr_el = arr_el->next, idx++) {
                    assert(arr_el->value->type == json_type_object);
                    json_object_s *object = (json_object_s *)arr_el->value->payload;
                    for (json_object_element_s *object_el = object->start; object_el; object_el = object_el->next) {
                        assert(object_el->value->type != json_type_object);

                        if (strcmp(object_el->name->string, "type") == 0) {
                            assert(object_el->value->type == json_type_string);
                            json_string_s *json_str = (json_string_s *)object_el->value->payload;
                            const char *type = json_str->string;

                            if (strcmp(type, "ambient") == 0) {
                                world->light_sources[idx].type = AMBIENT_LIGHT;
                            }
                            else if (strcmp(type, "point") == 0) {
                                world->light_sources[idx].type = POINT_LIGHT;
                            }
                            else if (strcmp(type, "directional") == 0) {
                                world->light_sources[idx].type = DIRECTIONAL_LIGHT;
                            }
                        }
                        else if (strcmp(object_el->name->string, "intensity") == 0) {
                            assert(object_el->value->type == json_type_number);
                            world->light_sources[idx].intensity = (float)atof(json_value_as_number(object_el->value)->number);
                        }
                        else if (strcmp(object_el->name->string, "position") == 0) {
                            assert(object_el->value->type == json_type_array);
                            json_array_s *position_array = (json_array_s *)object_el->value->payload;
                            world->light_sources[idx].position = json_array_to_vec3(position_array);
                        }
                        else if (strcmp(object_el->name->string, "direction") == 0) {
                            assert(object_el->value->type == json_type_array);
                            json_array_s *direction_array = (json_array_s *)object_el->value->payload;
                            world->light_sources[idx].direction = json_array_to_vec3(direction_array);
                        }
                    }
                }
            }
        }
    }

    free(root);
    return true;
}