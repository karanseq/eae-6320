/*
	TODO
*/

#ifndef EAE6320_GRAPHICS_CSPRITE_H
#define EAE6320_GRAPHICS_CSPRITE_H

// Include Files
//==============

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

			eae6320::cResult Initialize();
			eae6320::cResult CleanUp();

			cSprite(const eae6320::Math::sVector2d& i_centre, const eae6320::Math::sVector2d& i_extents);
			~cSprite();

		private:
			// Generates vertex data in counter-clockwise winding, based on centre and extents
			void GetVertexData(eae6320::Graphics::VertexFormats::sSprite* o_vertexData) const;

			// Data
			//=====

		private:
			eae6320::Math::sVector2d m_centre;
			eae6320::Math::sVector2d m_extents;

#if defined( EAE6320_PLATFORM_D3D )
			// A vertex buffer holds the data for each vertex
			ID3D11Buffer* m_vertexBuffer = nullptr;
			// D3D has an "input layout" object that associates the layout of the vertex format struct
			// with the input from a vertex shader
			ID3D11InputLayout* m_vertexInputLayout = nullptr;
#endif

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