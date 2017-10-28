/*
    This file should be #included by all shaders
*/

// Version Information
//====================

#if defined( EAE6320_PLATFORM_GL )

    // GLSL shaders require the version to be #defined before anything else in the shader
    #version 420

#endif

// Types
//======

#if defined( EAE6320_PLATFORM_D3D )

    #define vec2 float2
    #define vec3 float3
    #define vec4 float4
    #define mat2 float2x2
    #define mat3 float3x3
    #define mat4 float4x4

#elif defined( EAE6320_PLATFORM_GL )

    #define float2 vec2
    #define float3 vec3
    #define float4 vec4
    #define float2x2 mat2
    #define float3x3 mat3
    #define float4x4 mat4

#endif

// Constant Buffers
//=================

#if defined( EAE6320_PLATFORM_D3D )

    #define DeclareConstantBuffer( i_name, i_id ) cbuffer i_name : register( b##i_id )
    #define DeclareTexture( i_name, i_id ) Texture2D i_name : register( t##i_id )
    #define DeclareSamplerState( i_name, i_id ) SamplerState i_name : register( s##i_id )
    #define GetSampledColor( i_color_texture, i_uv, i_color_samplerState ) i_color_texture.Sample( i_color_samplerState, i_uv )
    #define Mul( i_matrix, i_vector ) mul( i_matrix, i_vector )

#elif defined( EAE6320_PLATFORM_GL )

    #define DeclareConstantBuffer( i_name, i_id ) layout( std140, binding = i_id ) uniform i_name
    #define DeclareTexture( i_name, i_id ) layout( binding = i_id ) uniform sampler2D i_name
    #define DeclareSamplerState( i_name, i_id )
    #define GetSampledColor( i_color_texture, i_uv, i_color_samplerState ) texture2D( i_color_texture, i_uv )
    #define Mul( i_matrix, i_vector ) i_matrix * i_vector

#endif

DeclareConstantBuffer( g_constantBuffer_perFrame, 0 )
{
    float4x4 g_transform_worldToCamera;
    float4x4 g_transform_cameraToProjected;

    float g_elapsedSecondCount_systemTime;
    float g_elapsedSecondCount_simulationTime;
    // For float4 alignment
    float2 g_padding;
};

DeclareConstantBuffer( g_constantBuffer_perMaterial, 1 )
{
    float4 g_color;
};

DeclareConstantBuffer( g_constantBuffer_perDrawCall, 2 )
{
    float4x4 g_transform_localToWorld;
};
