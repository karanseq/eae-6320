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

			static eae6320::cResult Load(cEffect*& o_effect, const char* i_vertexShaderName, const char* i_fragmentShaderName);

		public:

			// Reference Counting
			//-------------------

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cEffect);

		private:

			eae6320::cResult Initialize(const char* i_vertexShaderName, const char* i_fragmentShaderName);
			eae6320::cResult CleanUp();

			cEffect() = default;
			~cEffect();

		private:

			void BindPlatform() const;
			eae6320::cResult InitializePlatform();
			eae6320::cResult CleanUpPlatform();

			// Data
			//=====

		private:

			eae6320::Graphics::cShader::Handle m_vertexShader;
			eae6320::Graphics::cShader::Handle m_fragmentShader;

#if defined( EAE6320_PLATFORM_GL )
			GLuint m_programId = 0;
#endif

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();

			eae6320::Graphics::cRenderState m_renderState;

		}; // class cEffect

	} // namespace Graphics

} // namespace eae6320

#endif // EAE6320_GRAPHICS_CEFFECT_H
