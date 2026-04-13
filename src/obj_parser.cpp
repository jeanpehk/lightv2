#include "obj_parser.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "core.h"

// Parser

struct Parser {
    String string;
    uint64_t index;
};

char parser_peek(Parser *parser) {
    if (parser->index + 1 >= parser->string.size) {
        return '\0';
    }

    return parser->string.str[parser->index];
}

bool parser_done(Parser *parser) {
    if (parser->index >= parser->string.size) return true;

    char c = parser_peek(parser);
    if (c == '\0') return true;

    return false;
}

void parser_skip_line(Parser *parser) {

    while (!parser_done(parser)) {

        char c = parser_peek(parser);
        if (c == '\n') {
            parser->index += 1;

            return;
        }

        if (c == '\r') {
            parser->index += 2;

            return;
        }

        parser->index += 1;
    }
}

bool parser_eat(Parser *parser, const char *str) {
    for (uint64_t i = 0; i < strlen(str); i++) {
        if (parser_done(parser)) return true;

        char c = parser_peek(parser);
        if (!(c == str[i])) return true;

        parser->index += 1;
    }

    return false;
}

bool parser_eat_newline(Parser *parser) {
    char c = parser_peek(parser);

    if (c == '\n') {
        parser->index += 1;

        return false;
    }

    if (c == '\r') {
        parser->index += 2;

        return false;
    }

    return true;
}

uint64_t parser_next_word_len(Parser *parser) {
    uint64_t len = 0;
    uint64_t stored_index = parser->index;

    char c = parser_peek(parser);
    while (!isspace(c) && c != '\0') {
        parser->index += 1;
        len += 1;

        c = parser_peek(parser);
    }

    parser->index = stored_index;

    return len;
}

bool parser_eat_uint(Parser *parser, uint64_t *u) {
    uint64_t next_word_len = parser_next_word_len(parser);

    // @Todo: can output garbage since we don't confirm it's a valid uint.
    *u = (uint64_t) atoi(&parser->string.str[parser->index]);

    parser->index += next_word_len;

    return false;
}

bool parser_eat_float(Parser *parser, float *f) {
    uint64_t next_word_len = parser_next_word_len(parser);

    // @Todo: can output garbage since we don't confirm it's a valid float.
    *f = (float) atof(&parser->string.str[parser->index]);

    parser->index += next_word_len;

    return false;
}

// OBJ parse

void obj_parse_vertex_array(OBJ *obj, Parser *parser) {
    while (parser_peek(parser) == 'v') {
        Vec3 v = { 0 };

        assert(!parser_eat(parser, "v "));
        assert(!parser_eat_float(parser, &v.x));

        assert(!parser_eat(parser, " "));
        assert(!parser_eat_float(parser, &v.y));

        assert(!parser_eat(parser, " "));
        assert(!parser_eat_float(parser, &v.z));

        if (parser_done(parser)) return;

        assert(!parser_eat_newline(parser));

        obj->vertices.push(v);
    }
}

void obj_parse_face_elem_array(OBJ *obj, Parser *parser) {
    while (parser_peek(parser) == 'f') {
        OBJ_Face face = { 0 };

        assert(!parser_eat(parser, "f "));
        assert(!parser_eat_uint(parser, &face.vertex_indices[0]));

        // @Todo: add tex_coords/normals, we will crash here now if there are any.

        assert(!parser_eat(parser, " "));
        assert(!parser_eat_uint(parser, &face.vertex_indices[1]));

        assert(!parser_eat(parser, " "));
        assert(!parser_eat_uint(parser, &face.vertex_indices[2]));

        // @Note: Indices in .obj format start from 1 so we always have to do this.
        face.vertex_indices[0] -= 1;
        face.vertex_indices[1] -= 1;
        face.vertex_indices[2] -= 1;

        obj->faces.push(face);

        if (parser_done(parser)) return;

        assert(!parser_eat_newline(parser));
    }
}

OBJ obj_parse(const char *fn) {
    OBJ ret = { 0 };
    Parser parser = { 0 };

    String string_data = file_read_string_alloc(fn);

    parser.string = string_data;
    parser.index = 0;

    ret.vertices = array_create<Vec3>();
    ret.normals = array_create<Vec3>();
    ret.faces = array_create<OBJ_Face>();

    while (!parser_done(&parser)) {

        char c = parser_peek(&parser);

        if (c == '#') {
            parser_skip_line(&parser);
        }
        else if (c == 'm') { // mtllib = material file
            parser_skip_line(&parser);
        }
        else if (c == 'o') { // o = object
            parser_skip_line(&parser);
        }
        else if (c == 'v') { // v = vertex
            obj_parse_vertex_array(&ret, &parser);
        }
        else if (c == 's') { // s = shading
            parser_skip_line(&parser);
        }
        else if (c == 'f') { // f = face
            obj_parse_face_elem_array(&ret, &parser);
        }
        else if (c == 'u') { // usemtl = material name
            parser_skip_line(&parser);
        }
        // @Todo: we should mb check for garbage within the line here.
        else if (isspace(c)) {
            parser_skip_line(&parser);
        } else {
            printf("Unknown line start at index: %zd (char %c)\n", parser.index, parser.string.str[parser.index]);

            exit(1);
        }
    }

    string_free(string_data);

    printf("parse finished..\n");

    return ret;
}

void obj_dump_vertices(OBJ *obj) {
    for (uint64_t i = 0; i < obj->vertices.len; i++) {
        Vec3 v = obj->vertices.get(i);
        printf("%zd: %f %f %f\n", i, v.x, v.y, v.z);
    }
}

void obj_dump_faces(OBJ *obj) {
    for (uint64_t i = 0; i < obj->faces.len; i++) {
        OBJ_Face face = obj->faces.get(i);
        printf("%zd: %zd %zd %zd\n", i, face.vertex_indices[0], face.vertex_indices[1], face.vertex_indices[2]);
    }
}

void obj_free(OBJ obj) {
    array_destroy(obj.vertices);
    array_destroy(obj.texture_coords);
    array_destroy(obj.normals);
}