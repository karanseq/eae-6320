/*
    TODO
*/

#ifndef EAE6320_GRAPHICS_CEFFECT_H
#define EAE6320_GRAPHICS_CEFFECT_H

// Include Files
//==============

#include "cRenderState.h"
#include "cShader.h"
    
#include <Engine/Assets/ReferenceCountedAssets.h>
#include <Engine/Results/Results.h>

#ifdef EAE6320_PLATFORM_GL
    #include "OpenGL/Includes.h"
#endif

// Class Declaration
//==================

namespace eae6320
{
    namespace Graphics
    {
        class cEffect
        {
            // Interface
            //==========

        public:

            // Render
            //-------

            void Bind() const;

            // Initialization / Clean Up
            //--------------------------

            static cResult Create(cEffect*& o_effect, const char* i_vertexShaderName, const char* i_fragmentShaderName, uint8_t i_renderStateBits);

        public:

            // Reference Counting
            //-------------------

            EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

            EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cEffect);

            // Data
            //=====

        public:

            FORCEINLINE const cRenderState& GetRenderState() const { return m_renderState; }

        private:

            cResult Initialize(const char* i_vertexShaderName, const char* i_fragmentShaderName, uint8_t i_renderStateBits);
            cResult CleanUp();

            cEffect() = default;
            ~cEffect();

        private:

            // Implementation
            //===============

            void BindPlatform() const;
            cResult InitializePlatform();
            cResult CleanUpPlatform();

            // Data
            //=====

        private:

            cShader::Handle m_vertexShader;
            cShader::Handle m_fragmentShader;

#if defined( EAE6320_PLATFORM_GL )
            GLuint m_programId = 0;
#endif

            EAE6320_ASSETS_DECLAREREFERENCECOUNT();

            cRenderState m_renderState;

        }; // class cEffect

    } // namespace Graphics

} // namespace eae6320

#endif // EAE6320_GRAPHICS_CEFFECT_H
