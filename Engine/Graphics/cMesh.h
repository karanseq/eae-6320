/*
    TODO
*/

#ifndef EAE6320_GRAPHICS_CMESH_H
#define EAE6320_GRAPHICS_CMESH_H

// Include Files
//==============

#include <Engine/Assets/ReferenceCountedAssets.h>
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
            struct sMesh;
        }
        struct sColor;
    }

    namespace Math
    {
        struct sVector;
    }
}

// Class Declaration
//==================

namespace eae6320
{
    namespace Graphics
    {
        class cMesh
        {
            // Interface
            //==========

        public:

            // Render
            //-------

            void Draw() const;

            // Initialization / Clean Up
            //--------------------------

            static cResult Create(cMesh*& o_mesh, const uint16_t i_vertexCount, const eae6320::Math::sVector* i_vertices, const uint16_t* i_indices, const eae6320::Graphics::sColor* i_colors);

        public:

            // Reference Counting
            //-------------------

            EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

            EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cMesh);

        private:

            cResult Initialize(const uint16_t i_vertexCount, const eae6320::Math::sVector* i_vertices, const uint16_t* i_indices, const eae6320::Graphics::sColor* i_colors);
            cResult CleanUp();

            cMesh() = default;
            ~cMesh();

        private:

            // Implementation
            //===============

            void GetVertexBufferData(VertexFormats::sMesh* o_vertexData, const uint16_t i_vertexCount, const eae6320::Math::sVector* i_vertices, const eae6320::Graphics::sColor* i_colors) const;

            void GetIndexBufferData(uint16_t* o_indexData, const uint16_t i_vertexCount, const uint16_t* i_indices) const;

        private:

            // Data
            //=====

            static const uint8_t s_verticesPerTriangle = 3;

#if defined( EAE6320_PLATFORM_D3D )
            // A vertex buffer holds the data for each vertex
            ID3D11Buffer* m_vertexBuffer = nullptr;
            // An index buffer holds the indices to vertex data
            ID3D11Buffer* m_indexBuffer = nullptr;
            // D3D has an "input layout" object that associates the layout of the vertex format struct
            // with the input from a vertex shader
            ID3D11InputLayout* m_vertexInputLayout = nullptr;
#endif

            uint16_t m_indexCount = 0;

            EAE6320_ASSETS_DECLAREREFERENCECOUNT();

#if defined( EAE6320_PLATFORM_GL )
            // A vertex buffer holds the data for each vertex
            GLuint m_vertexBufferId = 0;
            // An index buffer holds the indices to vertex data
            GLuint m_indexBufferId = 0;
            // A vertex array encapsulates the vertex data as well as the vertex input layout
            GLuint m_vertexArrayId = 0;
#endif

        }; // class cMesh

    } // namespace Graphics

} // namespace eae6320

#endif // EAE6320_GRAPHICS_CMESH_H
