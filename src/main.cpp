#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

#include "obj_parser.h"

const int WINDOW_WIDTH  = 1024;
const int WINDOW_HEIGHT = 720;

static const char *HLSL_ENTRY_POINT_NAME_VERT = "VSMain";
static const char *HLSL_ENTRY_POINT_NAME_FRAG = "PSMain";

int main(int argc, char *argv[]) {
    bool success = SDL_Init(SDL_INIT_VIDEO);
    if (!success) {
        printf("SDL_Init failed: %s\n", SDL_GetError());

        return 1;
    }

    printf("SDL Initialized\n");

    SDL_Window *window = SDL_CreateWindow("Window",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());

        return -1;
    }

    // GPU inits
    bool debug_mode = true;
    SDL_GPUDevice *gpu_device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV, // vulkan
        debug_mode,
        "vulkan"
    );
    if (gpu_device == NULL) {
        printf("GPU Device creation failed: %s\n", SDL_GetError());

        return -1;
    }

    if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
        printf("Window claim for gpu device failed: %s\n", SDL_GetError());

        return -1;
    }

    Arena arena = arena_alloc();

    // Load shaders.
    // It would be nice if we could just generate all the create info stuff while compiling.

    const char *vert_shader_fn = "src/shaders/shader.vert.spv";
    String *vert_code = file_read_string(&arena, vert_shader_fn);

    SDL_GPUShaderCreateInfo shader_info_vert = { 0 };
    shader_info_vert.code_size = vert_code->len;
    shader_info_vert.code = (const Uint8 *) vert_code->str;
    shader_info_vert.entrypoint = HLSL_ENTRY_POINT_NAME_VERT;
    shader_info_vert.format = SDL_GPU_SHADERFORMAT_SPIRV;
    shader_info_vert.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    shader_info_vert.num_samplers = 0;
    shader_info_vert.num_storage_textures = 0;
    shader_info_vert.num_storage_buffers = 0;
    shader_info_vert.num_uniform_buffers = 0;
    shader_info_vert.props = 0; // A properties ID for extensions.

    SDL_GPUShader *shader_vert = SDL_CreateGPUShader(gpu_device, &shader_info_vert);
    if (shader_vert == NULL) {
        printf("Shader creation failed (%s): %s", vert_shader_fn, SDL_GetError());

        return -1;
    }

    const char *frag_shader_fn = "src/shaders/shader.frag.spv";
    String *frag_code = file_read_string(&arena, frag_shader_fn);

    SDL_GPUShaderCreateInfo shader_info_frag = { 0 }; // Not sure if we could reuse the earlier struct or if this data needs to stay stable.
    shader_info_frag.code_size = frag_code->len;
    shader_info_frag.code = (const Uint8 *) frag_code->str;
    shader_info_frag.entrypoint = HLSL_ENTRY_POINT_NAME_FRAG;
    shader_info_frag.format = SDL_GPU_SHADERFORMAT_SPIRV;
    shader_info_frag.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    shader_info_frag.num_samplers = 0;
    shader_info_frag.num_storage_textures = 0;
    shader_info_frag.num_storage_buffers = 0;
    shader_info_frag.num_uniform_buffers = 0;
    shader_info_frag.props = 0; // A properties ID for extensions.

    SDL_GPUShader *shader_frag = SDL_CreateGPUShader(gpu_device, &shader_info_frag);
    if (shader_frag == NULL) {
        printf("Shader creation failed (%s): %s", frag_shader_fn, SDL_GetError());

        return -1;
    }

    // Load OBJ file

    OBJ *obj = obj_parse(&arena, "assets/blender_default_cube.obj");
    obj_dump_vertices(obj);
    obj_dump_faces(obj);

    bool done = false;

    while (!done) {
        // Handle events.

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }

        const bool *kb_state = SDL_GetKeyboardState(NULL);

        if (kb_state[SDL_SCANCODE_ESCAPE]) {
            done = true;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}