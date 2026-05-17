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

    // Quick buffer data for a triangle.

    uint32_t num_verts = 3;
    uint32_t verts_size = sizeof(float) * num_verts * 3;
    float *verts = (float*) arena_push_zero(&arena, verts_size);

    verts[0] = 0.0f;
    verts[1] = 0.0f;
    verts[2] = 0.0f;

    verts[3] = 1.0f;
    verts[4] = 1.0f;
    verts[5] = 0.0f;

    verts[6] = 0.0f;
    verts[7] = 1.0f;
    verts[8] = 0.0f;

    uint32_t num_indices = 3;
    uint32_t indices_size = sizeof(uint32_t) * num_indices;
    uint32_t *indices = (uint32_t*) arena_push_zero(&arena, indices_size);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    // Create GPU Buffers.

    SDL_GPUBufferCreateInfo buffer_info_vertex = { 0 };
    buffer_info_vertex.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    buffer_info_vertex.size = verts_size;
    buffer_info_vertex.props = 0;

    printf("Create gpu buffers..\n");

    SDL_GPUBuffer *gpu_buffer_vertex = SDL_CreateGPUBuffer(gpu_device, &buffer_info_vertex);
    if (gpu_buffer_vertex == NULL) {
        printf("GPU Buffer creation failed: %s", SDL_GetError());

        return -1;
    }

    SDL_GPUBufferCreateInfo buffer_info_index = { 0 };
    buffer_info_index.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    buffer_info_index.size = indices_size;
    buffer_info_index.props = 0;

    SDL_GPUBuffer *gpu_buffer_index = SDL_CreateGPUBuffer(gpu_device, &buffer_info_index);
    if (gpu_buffer_index == NULL) {
        printf("GPU Buffer creation failed: %s", SDL_GetError());

        return -1;
    }

    {
        printf("Upload data to GPU..\n");

        SDL_GPUCommandBuffer *cmd_buffer = SDL_AcquireGPUCommandBuffer(gpu_device);
        if (cmd_buffer == NULL) {
            printf("Command Buffer creation failed: %s", SDL_GetError());

            return -1;
        }

        SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(cmd_buffer);

        // Vertex

        SDL_GPUTransferBufferCreateInfo transfer_buffer_info_vertex = { 0 };
        transfer_buffer_info_vertex.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transfer_buffer_info_vertex.size = verts_size;
        transfer_buffer_info_vertex.props = 0;

        SDL_GPUTransferBuffer *transfer_buffer_vertex = SDL_CreateGPUTransferBuffer(
            gpu_device,
            &transfer_buffer_info_vertex
        );
        if (transfer_buffer_vertex == NULL) {
            printf("Transfer buffer creation failed: %s", SDL_GetError());

            return -1;
        }

        SDL_GPUTransferBufferLocation transfer_buffer_vertex_location = { 0 };
        transfer_buffer_vertex_location.transfer_buffer = transfer_buffer_vertex;
        transfer_buffer_vertex_location.offset = 0;

        SDL_GPUBufferRegion gpu_buffer_region_vertex = { 0 };
        gpu_buffer_region_vertex.buffer = gpu_buffer_vertex;
        gpu_buffer_region_vertex.offset = 0;
        gpu_buffer_region_vertex.size = verts_size;

        SDL_UploadToGPUBuffer(
            copy_pass,
            &transfer_buffer_vertex_location,
            &gpu_buffer_region_vertex,
            false // cycle
        );

        // Index

        SDL_GPUTransferBufferCreateInfo transfer_buffer_info_index = { 0 };
        transfer_buffer_info_index.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transfer_buffer_info_index.size = indices_size;
        transfer_buffer_info_index.props = 0;

        SDL_GPUTransferBuffer *transfer_buffer_index = SDL_CreateGPUTransferBuffer(
            gpu_device,
            &transfer_buffer_info_index
        );
        if (transfer_buffer_index == NULL) {
            printf("Transfer buffer creation failed: %s", SDL_GetError());

            return -1;
        }

        SDL_GPUTransferBufferLocation transfer_buffer_index_location = { 0 };
        transfer_buffer_index_location.transfer_buffer = transfer_buffer_index;
        transfer_buffer_index_location.offset = 0;

        SDL_GPUBufferRegion gpu_buffer_region_index = { 0 };
        gpu_buffer_region_index.buffer = gpu_buffer_index;
        gpu_buffer_region_index.offset = 0;
        gpu_buffer_region_index.size = indices_size;

        SDL_UploadToGPUBuffer(
            copy_pass,
            &transfer_buffer_index_location,
            &gpu_buffer_region_index,
            false // cycle
        );

        SDL_EndGPUCopyPass(copy_pass);

        SDL_ReleaseGPUTransferBuffer(gpu_device, transfer_buffer_vertex);
        SDL_ReleaseGPUTransferBuffer(gpu_device, transfer_buffer_index);

        if (!SDL_SubmitGPUCommandBuffer(cmd_buffer)) {
            printf("Submit command buffer failed: %s", SDL_GetError());

            return -1;
        }
    }

    // Load OBJ file

    /*
    OBJ *obj = obj_parse(&arena, "assets/blender_default_cube.obj");
    obj_dump_vertices(obj);
    obj_dump_faces(obj);
    */

    bool done = false;

    printf("Enter Main Loop..\n");

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