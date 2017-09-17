/*
	TODO
*/

#ifndef EAE6320_GRAPHICS_CVIEW_H
#define EAE6320_GRAPHICS_CVIEW_H

// Include Files
//==============

#include "Graphics.h"

#include <Engine/Results/Results.h>

// Forward Declarations
//=====================

#ifdef EAE6320_PLATFORM_D3D
	struct ID3D11RenderTargetView;
	struct ID3D11DepthStencilView;
#endif

namespace eae6320
{
	namespace Graphics
	{
		struct sColor;
	}
}

// Class Declaration
//==================

namespace eae6320
{
	namespace Graphics
	{
		class cView
		{
			// Interface
			//==========

		public:

			// Render
			//-------

			void Clear(const sColor& i_color) const;
			void Swap() const;

			// Initialization / Clean Up
			//--------------------------

			eae6320::cResult Initialize(const sInitializationParameters& i_initializationParameters);
			eae6320::cResult CleanUp();

			cView() = default;
			~cView();

		private:

			// Data
			//=====
			
#if defined(EAE6320_PLATFORM_D3D)
			// In Direct3D "views" are objects that allow a texture to be used a particular way:
			// A render target view allows a texture to have color rendered to it
			ID3D11RenderTargetView* m_renderTargetView = nullptr;
			// A depth/stencil view allows a texture to have depth rendered to it
			ID3D11DepthStencilView* m_depthStencilView = nullptr;
#endif

		}; // class cView

	} // namespace Graphics

} // namespace eae6320

#endif // EAE6320_GRAPHICS_CVIEW_H
