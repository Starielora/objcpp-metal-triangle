#include <metal_stdlib>

using namespace metal;

struct VertexIn
{
    float2 position [[attribute(0)]];
    float3 color [[attribute(1)]];
};

struct VertexOut
{
    float4 position [[position]];
    float4 color;
};

vertex VertexOut vertex_main(const VertexIn vertex_in [[stage_in]])
{
    return VertexOut{
        .position = float4(vertex_in.position, 0, 1),
        .color = float4(vertex_in.color,1)
    };
}

fragment float4 fragment_main(VertexOut in [[stage_in]])
{
    return in.color;
}
