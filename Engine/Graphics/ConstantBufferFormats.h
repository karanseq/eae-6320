/*
    This file defines the layout of the constant data
    that the CPU sends to the GPU

    These must exactly match the constant buffer definitions in shader programs.
*/

#ifndef EAE6320_GRAPHICS_CONSTANTBUFFERFORMATS_H
#define EAE6320_GRAPHICS_CONSTANTBUFFERFORMATS_H

// Include Files
//==============

#include "Configuration.h"

// Format Definitions
//===================

namespace eae6320
{
    namespace Graphics
    {
        namespace ConstantBufferFormats
        {
            struct sPerFrame
            {
                float g_elapsedSecondCount_systemTime = 0.0f;
                float g_elapsedSecondCount_simulationTime = 0.0f;
                float padding[2];   // For float4 alignment
            };

            struct sPerMaterial
            {
                // TODO: Replace this with sColor
                struct
                {
                    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
                } g_color;
            };

            struct sPerDrawCall
            {
                struct
                {
                    float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f;
                } g_position;
            };
        }
    }
}

#endif  // EAE6320_GRAPHICS_CONSTANTBUFFERFORMATS_H
