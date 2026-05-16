@echo off

set SRC_DIR=src/shaders/
set OUT_DIR=src/shaders/

:: glsl example: glslangValidator %SRC_DIR%shader.vert.glsl --target-env vulkan1.0 -o %OUT_DIR%shader.vert.spv

:: the dxc with windows sdk (atleast on my machine) does not come with a spirv backend so we use the one provided by vulkansdk
%DXC_VULKAN% -T vs_6_0 -spirv -E VSMain %SRC_DIR%shader.vert.hlsl -Fo %OUT_DIR%shader.vert.spv
%DXC_VULKAN% -T ps_6_0 -spirv -E PSMain %SRC_DIR%shader.frag.hlsl -Fo %OUT_DIR%shader.frag.spv