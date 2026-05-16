// VS_ = naming convention apparently (vertex shader)

/*
cbuffer MatrixBuffer : register(b0)
{
    matrix worldViewProjection;
};
*/

struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

    //output.position = mul(float4(input.position, 1.0f), worldViewProjection);
    output.position = float4(input.position, 1.0f);

    return output;
}