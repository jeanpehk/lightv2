#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

#include "obj_parser.h"

const int WINDOW_WIDTH  = 1024;
const int WINDOW_HEIGHT = 720;

const SDL_GPUViewport VIEWPORT = {
    160.0f, // x
    120.0f, // y
    320.0f, // w
    240.0f, // h
    0.1f,   // min_depth
    1.0f    // max_depth
};

static const char *HLSL_ENTRY_POINT_NAME_VERT = "VSMain";
static const char *HLSL_ENTRY_POINT_NAME_FRAG = "PSMain";

int main(int argc, char *argv[]) {
    Arena arena = arena_alloc();

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

    // GPU Init code.

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

    // Load shaders.
    // It would be nice if we could just generate all the create info stuff while compiling.

    const char *vert_shader_fn = "src/shaders/shader.vert.spv";
    String *vert_code = file_read_string(&arena, vert_shader_fn);

    SDL_GPUShaderCreateInfo shader_info_vert = {};
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

    SDL_GPUShaderCreateInfo shader_info_frag = {};
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
    uint32_t verts_size = sizeof(float) * 3 * num_verts;

    /*
    uint32_t num_indices = 3;
    uint32_t indices_size = sizeof(uint32_t) * num_indices;
    */

    printf("Creating the graphics pipeline..\n");

    SDL_GPUVertexBufferDescription vertex_buffer_description = {};
    vertex_buffer_description.slot = 0;
    vertex_buffer_description.pitch = sizeof(float) * 3;
    vertex_buffer_description.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertex_buffer_description.instance_step_rate = 0; // must be 0

    SDL_GPUVertexAttribute vertex_attribute = {};
    vertex_attribute.location = 0;
    vertex_attribute.buffer_slot = 0;
    vertex_attribute.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertex_attribute.offset = 0;

    SDL_GPUVertexInputState vertex_input_state = {};
    vertex_input_state.vertex_buffer_descriptions = &vertex_buffer_description;
    vertex_input_state.num_vertex_buffers = 1;
    vertex_input_state.vertex_attributes = &vertex_attribute;
    vertex_input_state.num_vertex_attributes = 1;

    SDL_GPUColorTargetDescription color_target_description = {};
    color_target_description.format = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);

    SDL_GPUGraphicsPipelineTargetInfo graphics_pipeline_target_info = {};
    graphics_pipeline_target_info.color_target_descriptions = &color_target_description;
    graphics_pipeline_target_info.num_color_targets = 1;

    SDL_GPUGraphicsPipelineCreateInfo graphics_pipeline_info = {};
    graphics_pipeline_info.vertex_shader = shader_vert;
    graphics_pipeline_info.fragment_shader = shader_frag;
    graphics_pipeline_info.vertex_input_state = vertex_input_state;
    graphics_pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    graphics_pipeline_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    graphics_pipeline_info.target_info = graphics_pipeline_target_info;
    SDL_GPUGraphicsPipeline *graphics_pipeline = SDL_CreateGPUGraphicsPipeline(gpu_device, &graphics_pipeline_info);

    SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(gpu_device, &graphics_pipeline_info);
    if (pipeline == NULL) {
        printf("Pipeline creation failed: %s", SDL_GetError());

        return -1;
    }

    SDL_ReleaseGPUShader(gpu_device, shader_vert);
    SDL_ReleaseGPUShader(gpu_device, shader_frag);

    /*
    SDL_GPUBufferCreateInfo buffer_info_index = { 0 };
    buffer_info_index.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    buffer_info_index.size = indices_size;
    buffer_info_index.props = 0;

    SDL_GPUBuffer *gpu_buffer_index = SDL_CreateGPUBuffer(gpu_device, &buffer_info_index);
    if (gpu_buffer_index == NULL) {
        printf("GPU Buffer creation failed: %s", SDL_GetError());

        return -1;
    }
        */


    // Create a GPU buffer.

    SDL_GPUBufferCreateInfo buffer_info_vertex = {};
    buffer_info_vertex.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    buffer_info_vertex.size = verts_size;
    buffer_info_vertex.props = 0;

    SDL_GPUBuffer *gpu_buffer_vertex = SDL_CreateGPUBuffer(gpu_device, &buffer_info_vertex);
    if (gpu_buffer_vertex == NULL) {
        printf("GPU Buffer creation failed: %s", SDL_GetError());

        return -1;
    }

    // Create a transfer buffer.

    SDL_GPUTransferBufferCreateInfo transfer_buffer_info_vertex = {};
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

    // Map transfer buffer so we can upload data to it and then unmap it.

    float *transfer_data = (float *) SDL_MapGPUTransferBuffer(gpu_device, transfer_buffer_vertex, false);

    transfer_data[0] = 0.0f;
    transfer_data[1] = 1.0f;
    transfer_data[2] = 0.0f;

    transfer_data[3] = 1.0f;
    transfer_data[4] = 0.0f;
    transfer_data[5] = 0.0f;

    transfer_data[6] = 0.0f;
    transfer_data[7] = 0.0f;
    transfer_data[8] = 0.0f;

    SDL_UnmapGPUTransferBuffer(gpu_device, transfer_buffer_vertex);

    // Upload the data to the GPU.

    printf("Uploading data to GPU..\n");

    SDL_GPUCommandBuffer *cmd_buffer = SDL_AcquireGPUCommandBuffer(gpu_device);
    if (cmd_buffer == NULL) {
        printf("Command Buffer creation failed: %s", SDL_GetError());

        return -1;
    }

    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(cmd_buffer);

    SDL_GPUTransferBufferLocation transfer_buffer_vertex_location = {};
    transfer_buffer_vertex_location.transfer_buffer = transfer_buffer_vertex;
    transfer_buffer_vertex_location.offset = 0;

    SDL_GPUBufferRegion gpu_buffer_region_vertex = {};
    gpu_buffer_region_vertex.buffer = gpu_buffer_vertex;
    gpu_buffer_region_vertex.offset = 0;
    gpu_buffer_region_vertex.size = verts_size;

    SDL_UploadToGPUBuffer(
        copy_pass,
        &transfer_buffer_vertex_location,
        &gpu_buffer_region_vertex,
        false
    );

    // Index

    /*
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
    */

    SDL_EndGPUCopyPass(copy_pass);
    if (!SDL_SubmitGPUCommandBuffer(cmd_buffer)) {
        printf("Submit command buffer failed: %s", SDL_GetError());

        return -1;
    }

    SDL_ReleaseGPUTransferBuffer(gpu_device, transfer_buffer_vertex);
    //SDL_ReleaseGPUTransferBuffer(gpu_device, transfer_buffer_index);

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

        // Draw

        {
            SDL_GPUCommandBuffer *cmd_buffer_frame = SDL_AcquireGPUCommandBuffer(gpu_device);
            if (cmd_buffer_frame == NULL) {
                printf("Command Buffer creation failed: %s", SDL_GetError());

                return -1;
            }

            SDL_GPUTexture *swapchain_texture;
            if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buffer_frame, window, &swapchain_texture, NULL, NULL)) {
                printf("SDL_WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());

                return -1;
            }

            SDL_GPUColorTargetInfo color_target_info = {};
            color_target_info.texture = swapchain_texture;
            color_target_info.clear_color = { 0.75f, 0.75f, 0.75f, 1.0f };
            color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
            color_target_info.store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
                cmd_buffer_frame,
                &color_target_info,
                1,
                NULL
            );

            SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline);

            //SDL_SetGPUViewport(render_pass, &VIEWPORT);

            SDL_GPUBufferBinding buffer_binding = {};
            buffer_binding.buffer = gpu_buffer_vertex;
            buffer_binding.offset = 0;
            SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);

            SDL_DrawGPUPrimitives(
                render_pass,
                3, // num_vertices
                1, // num_instances
                0, // first_vertex
                0  // first instance
            );

            SDL_EndGPURenderPass(render_pass);

            SDL_SubmitGPUCommandBuffer(cmd_buffer_frame);
        }
    }

    SDL_ReleaseGPUGraphicsPipeline(gpu_device, graphics_pipeline);
    SDL_ReleaseGPUBuffer(gpu_device, gpu_buffer_vertex);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}