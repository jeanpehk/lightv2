#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

#include <stdint.h>

#include "array.h"
#include "vec3.h"
#include "core.h"

// @Note: We only support triangle data.
struct OBJ_Face {
    uint64_t vertex_indices[3];
    uint64_t tex_coord_indices[3];
    uint64_t normal_indices[3];
};

struct OBJ {

    // These are declared separately in .obj
    List<Vec3> vertices;
    List<Vec3> texture_coords;
    List<Vec3> normals;

    // Indices to all previously defined arrays (vertices/tex_coords/normals)
    List<OBJ_Face> faces;
};

OBJ *obj_parse(Arena *arena, const char *fn);
void obj_dump_vertices(OBJ *obj);
void obj_dump_faces(OBJ *obj);

#endif // TEXT_PARSER_H