#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

#include <stdint.h>

#include "array.h"
#include "vec3.h"

// @Note: We only support triangle data.
struct OBJ_Face {
    uint64_t vertex_indices[3];
    uint64_t tex_coord_indices[3];
    uint64_t normal_indices[3];
};

struct OBJ {

    // These are declared separately in .obj
    Array<Vec3> vertices;
    Array<Vec3> texture_coords;
    Array<Vec3> normals;

    // Indices to all previously defined arrays (vertices/tex_coords/normals)
    Array<OBJ_Face> faces;
};

OBJ obj_parse(const char *fn);
void obj_dump_vertices(OBJ *obj);
void obj_dump_faces(OBJ *obj);
void obj_free(OBJ obj);

#endif // TEXT_PARSER_H