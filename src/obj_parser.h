#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

#include <stdint.h>

#include "array.h"
#include "vec3.h"

struct String {
    char *str;
    uint64_t size;
};

String string_alloc_copy_from_cstr(char *cstr);
void string_free(String string);

struct Parser {
    String string;
    uint64_t index;
};

char parser_peek(Parser *parser);
void parser_skip_line(Parser *parser);
bool parser_eat(Parser *parser, const char *str);
bool parser_done(Parser *parser);

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

#endif // TEXT_PARSER_H