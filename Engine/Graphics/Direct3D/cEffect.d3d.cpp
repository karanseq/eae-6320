// Include Files
//==============

#include "../cEffect.h"

#include "Includes.h"
#include "../sContext.h"

#include <Engine/Asserts/Asserts.h>

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cEffect::Bind() const
{
	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT(direct3dImmediateContext);

	ID3D11ClassInstance* const* noInterfaces = nullptr;
	constexpr unsigned int interfaceCount = 0;
	// Vertex shader
	{
		EAE6320_ASSERT( m_vertexShader );
		auto* const shader = cShader::s_manager.Get( m_vertexShader );
		EAE6320_ASSERT( shader && shader->m_shaderObject.vertex );
		direct3dImmediateContext->VSSetShader( shader->m_shaderObject.vertex, noInterfaces, interfaceCount );
	}
	// Fragment shader
	{
		EAE6320_ASSERT( m_fragmentShader );
		auto* const shader = cShader::s_manager.Get( m_fragmentShader );
		EAE6320_ASSERT( shader && shader->m_shaderObject.fragment );
		direct3dImmediateContext->PSSetShader( shader->m_shaderObject.fragment, noInterfaces, interfaceCount );
	}

	m_renderState.Bind();
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cEffect::InitializePlatform()
{
	return eae6320::Results::Success;
}

eae6320::cResult eae6320::Graphics::cEffect::CleanUpPlatform()
{
	return eae6320::Results::Success;
}
