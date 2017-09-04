/*
	TODO
*/

#ifndef EAE6320_GRAPHICS_CSPRITE_H
#define EAE6320_GRAPHICS_CSPRITE_H

// Include Files
//==============
	
#include <Engine/Results/Results.h>

#ifdef EAE6320_PLATFORM_D3D
#include "Direct3D/Includes.h"
#endif

#ifdef EAE6320_PLATFORM_GL
#include "OpenGL/Includes.h"
#endif

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

			cSprite() = default;
			~cSprite();

			// Data
			//=====

		private:
#if defined(EAE6320_PLATFORM_D3D)
			// A vertex buffer holds the data for each vertex
			ID3D11Buffer* m_vertexBuffer = nullptr;
			// D3D has an "input layout" object that associates the layout of the vertex format struct
			// with the input from a vertex shader
			ID3D11InputLayout* m_vertexInputLayout = nullptr;
#endif

#if defined(EAE6320_PLATFORM_GL)
			// A vertex buffer holds the data for each vertex
			GLuint s_vertexBufferId = 0;
			// A vertex array encapsulates the vertex data as well as the vertex input layout
			GLuint s_vertexArrayId = 0;
#endif


		}; // class cSprite

	} // namespace Graphics

} // namespace eae6320



#endif // EAE6320_GRAPHICS_CSPRITE_H