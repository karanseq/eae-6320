// Include Files
//==============

#include "../cView.h"

#include "Includes.h"
#include "../sColor.h"
#include "../sContext.h"

#include <Engine/Asserts/Asserts.h>

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cView::Clear(const sColor& i_color) const
{
    {
        glClearColor(i_color.r, i_color.g, i_color.b, i_color.a);
        EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
    }
    {
        constexpr GLbitfield clearColor = GL_COLOR_BUFFER_BIT;
        glClear(clearColor);
        EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
    }
}

void eae6320::Graphics::cView::Swap() const
{
    const auto deviceContext = sContext::g_context.deviceContext;
    EAE6320_ASSERT(deviceContext != NULL);

    const auto glResult = SwapBuffers(deviceContext);
    EAE6320_ASSERT(glResult != FALSE);
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cView::Initialize(const sInitializationParameters& i_initializationParameters)
{
    return eae6320::Results::Success;
}

eae6320::cResult eae6320::Graphics::cView::CleanUp()
{
    return eae6320::Results::Success;
}
