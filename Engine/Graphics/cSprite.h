/*
    TODO
*/

#ifndef EAE6320_GRAPHICS_CSPRITE_H
#define EAE6320_GRAPHICS_CSPRITE_H

// Include Files
//==============

#include <Engine/Assets/ReferenceCountedAssets.h>
#include <Engine/Math/sVector2d.h>
#include <Engine/Results/Results.h>

#ifdef EAE6320_PLATFORM_GL
#include "OpenGL/Includes.h"
#endif

// Forward Declarations
//=====================

#ifdef EAE6320_PLATFORM_D3D
    struct ID3D11Buffer;
    struct ID3D11InputLayout;
#endif

namespace eae6320
{
    namespace Graphics
    {
        namespace VertexFormats
        {
            struct sSprite;
        }
    }
}

// Class Declaration
//==================

namespace eae6320
{
    namespace Graphics
    {
        class cSprite
        {
            // Interface
            //==========

        public:

            // Render
            //-------

            void Draw() const;

            // Initialization / Clean Up
            //--------------------------

            static cResult Create(cSprite*& o_sprite, const Math::sVector2d& i_origin, const Math::sVector2d& i_extents);

        public:

            // Reference Counting
            //-------------------

            EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

            EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cSprite);

        private:

            cResult Initialize(const Math::sVector2d& i_origin, const Math::sVector2d& i_extents);
            cResult CleanUp();

            cSprite() = default;
            ~cSprite();

        private:

            // Implementation
            //===============

            // Generates vertex positions for a quad in counter-clockwise winding, based on origin and extents
            void GetVertexPositions(VertexFormats::sSprite* o_vertexData, const Math::sVector2d& i_origin, const Math::sVector2d& i_extents) const;

            // Generates texture coordinates for a quad, based on origin and extents
            void GetVertexTextureCoordinates(VertexFormats::sSprite* o_vertexData, const Math::sVector2d& i_origin, const Math::sVector2d& i_extents) const;

        private:

            // Data
            //=====

#if defined( EAE6320_PLATFORM_D3D )
            // A vertex buffer holds the data for each vertex
            ID3D11Buffer* m_vertexBuffer = nullptr;
            // D3D has an "input layout" object that associates the layout of the vertex format struct
            // with the input from a vertex shader
            ID3D11InputLayout* m_vertexInputLayout = nullptr;
#endif

            EAE6320_ASSETS_DECLAREREFERENCECOUNT();

#if defined( EAE6320_PLATFORM_GL )
            // A vertex buffer holds the data for each vertex
            GLuint m_vertexBufferId = 0;
            // A vertex array encapsulates the vertex data as well as the vertex input layout
            GLuint m_vertexArrayId = 0;
#endif

        }; // class cSprite

    } // namespace Graphics

} // namespace eae6320



#endif // EAE6320_GRAPHICS_CSPRITE_H