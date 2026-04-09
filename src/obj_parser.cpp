#include "obj_parser.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

// A simple .obj file parser, has a bunch of util stuff for now that will be moved elsewhere.
// This is just to be able to parse the default blender cube for some basic testing later.

// Windows file stuff

int win_print_last_error() {
    DWORD err_buf_sz = 512;
    char err_buf[512] = { 0 };
    DWORD last_error = GetLastError();
    DWORD chars_stored = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        0,
        last_error,
        0, // @Note: use proper LANGID?
        err_buf,
        err_buf_sz,
        0
    );

    if (chars_stored == 0) {
        printf("Failed to format the resulting message for last error: %d\n", last_error);

        return -1;
    }

    printf("%s\n", err_buf);

    return 0;
}

String file_read_string_alloc(const char *file) {
    String ret = { 0 };

    HANDLE handle = CreateFileA(
        file,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE) {
        printf("Failed to open %s: ", file);
        win_print_last_error();

        return ret;
    }

    // @Note: we don't read the high bytes since
    // ReadFile takes a DWORD as size anyway and can't be bothered
    // to do more atm.
    LPDWORD out_lpFileSizeHigh = NULL;
    DWORD file_size_low32 = GetFileSize(
        handle,
        out_lpFileSizeHigh
    );

    if (file_size_low32 == INVALID_FILE_SIZE) {
        win_print_last_error();
        return ret;
    }

    DWORD file_size = file_size_low32;

    // @Alloc
    ret.size = file_size;
    ret.str = (char *) malloc(sizeof(char) * file_size);

    DWORD bytes_read = 0;
    BOOL ok = ReadFile(
        handle,
        ret.str,
        (DWORD)ret.size,
        &bytes_read,
        NULL
    );

    if (!ok) {
        printf("Failed to read %s: ", file);
        win_print_last_error();

        return ret;
    }

    return ret;
}

String string_alloc_copy_from_cstr(char *cstr) {
    String ret = { 0 };

    ret.size = strlen(cstr);
    ret.str = (char *) malloc(sizeof(char) * ret.size);
    for (uint64_t i = 0; i < ret.size; i++) {
        ret.str[i] = cstr[i];
    }

    return ret;
}

void string_free(String string) {
    string.size = 0;
    free(string.str);
}

void string_dump(String string) {
    for (uint64_t i = 0; i < string.size; i++) {
        putchar(string.str[i]);
    }
}

// Parser

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

enum Obj_Line_Type {
    OBJ_LINE_TYPE_COMMENT,         // #
    OBJ_LINE_TYPE_MATERIAL_FILE,   // mtllib
    OBJ_LINE_TYPE_OBJECT,          // o
    OBJ_LINE_TYPE_VERTEX,          // v
    OBJ_LINE_TYPE_SHADING,         // s
    OBJ_LINE_TYPE_FACE,            // f
    OBJ_LINE_TYPE_MATERIAL_NAME,   // usemtl, defined in a separate .mtl file
    OBJ_LINE_TYPE_WHITESPACE,      //
    OBJ_LINE_TYPE_UNKNOWN,         //
};

void dump_obj_line_type(Obj_Line_Type line_type) {
    switch (line_type) {

        case OBJ_LINE_TYPE_COMMENT: {
            printf("OBJ_LINE_TYPE_COMMENT\n");
        } break;

        case OBJ_LINE_TYPE_MATERIAL_FILE: {
            printf("OBJ_LINE_TYPE_MATERIAL_FILE\n");
        } break;

        case OBJ_LINE_TYPE_OBJECT: {
            printf("OBJ_LINE_TYPE_OBJECT\n");
        } break;

        case OBJ_LINE_TYPE_VERTEX: {
            printf("OBJ_LINE_TYPE_VERTEX\n");
        } break;

        case OBJ_LINE_TYPE_SHADING: {
            printf("OBJ_LINE_TYPE_SHADING\n");
        } break;

        case OBJ_LINE_TYPE_FACE: {
            printf("OBJ_LINE_TYPE_FACE\n");
        } break;

        case OBJ_LINE_TYPE_MATERIAL_NAME: {
            printf("OBJ_LINE_TYPE_MATERIAL_NAME\n");
        } break;

        case OBJ_LINE_TYPE_WHITESPACE: {
            printf("OBJ_LINE_TYPE_WHITESPACE\n");
        } break;

        case OBJ_LINE_TYPE_UNKNOWN: {
            printf("OBJ_LINE_TYPE_UNKNOWN\n");
        } break;

        default: {
            printf("Unexpected: unhandled Obj_Line_Type, maybe missing a case?\n");
        } break;
    }
}

Obj_Line_Type next_obj_line_type(Parser *parser) {
    char peek = parser_peek(parser);

    if (peek == '#') {
        return OBJ_LINE_TYPE_COMMENT;
    }

    else if (peek == 'o') {
        return OBJ_LINE_TYPE_OBJECT;
    }

    else if (peek == 'v') {
        return OBJ_LINE_TYPE_VERTEX;
    }

    else if (peek == 's') {
        return OBJ_LINE_TYPE_SHADING;
    }

    else if (peek == 'f') {
        return OBJ_LINE_TYPE_FACE;
    }

    else if (peek == 'm') {
        assert(!parser_eat(parser, "mtllib"));

        return OBJ_LINE_TYPE_MATERIAL_FILE;
    }

    else if (peek == 'u') {
        assert(!parser_eat(parser, "usemtl"));

        return OBJ_LINE_TYPE_MATERIAL_NAME;
    }

    else if (isspace(peek)) {
        return OBJ_LINE_TYPE_WHITESPACE;
    }

    else if (peek == '\r') {
        parser->index += 2;

        return next_obj_line_type(parser);
    }

    return OBJ_LINE_TYPE_UNKNOWN;
}

void obj_parse_vertex_array(OBJ *obj, Parser *parser) {
    while (next_obj_line_type(parser) == OBJ_LINE_TYPE_VERTEX) {
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
    while (next_obj_line_type(parser) == OBJ_LINE_TYPE_FACE) {
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
        Obj_Line_Type line_type = next_obj_line_type(&parser);

        switch (line_type) {

            case OBJ_LINE_TYPE_COMMENT: {
                parser_skip_line(&parser);
            } break;

            case OBJ_LINE_TYPE_MATERIAL_FILE: {
                parser_skip_line(&parser);
            } break;

            case OBJ_LINE_TYPE_OBJECT: {
                parser_skip_line(&parser);
            } break;

            case OBJ_LINE_TYPE_VERTEX: {
                obj_parse_vertex_array(&ret, &parser);
            } break;

            case OBJ_LINE_TYPE_SHADING: {
                parser_skip_line(&parser);
            } break;

            case OBJ_LINE_TYPE_FACE: {
                obj_parse_face_elem_array(&ret, &parser);
            } break;

            case OBJ_LINE_TYPE_MATERIAL_NAME: {
                parser_skip_line(&parser);
            } break;

            case OBJ_LINE_TYPE_WHITESPACE: {
                parser_skip_line(&parser);
            } break;

            case OBJ_LINE_TYPE_UNKNOWN: {
                printf("OBJ_LINE_TYPE_UNKNOWN at index: %zd (char %c)\n", parser.index, parser.string.str[parser.index]);

                exit(1);
            } break;

            default: {
                printf("Unexpected: line_type not caught by switch, maybe a case is unhandled.\n");

                exit(1);
            } break;
        }
    }

    string_free(string_data);

    printf("parse finished..\n");

    return ret;
}