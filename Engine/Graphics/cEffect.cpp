// Include Files
//==============

#include "cEffect.h"

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cEffect::Bind() const
{
	BindPlatform();

	m_renderState.Bind();
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cEffect::Initialize(const char* i_vertexShaderName, const char* i_fragmentShaderName)
{
	auto result = eae6320::Results::Success;

	{
		if (!(result = eae6320::Graphics::cShader::s_manager.Load(i_vertexShaderName,
			m_vertexShader, eae6320::Graphics::ShaderTypes::Vertex)))
		{
			EAE6320_ASSERT(false);
			goto OnExit;
		}
	}

	{
		if (!(result = eae6320::Graphics::cShader::s_manager.Load(i_fragmentShaderName,
			m_fragmentShader, eae6320::Graphics::ShaderTypes::Fragment)))
		{
			EAE6320_ASSERT(false);
			goto OnExit;
		}
	}

	{
		constexpr uint8_t defaultRenderState = 0;
		if (!(result = m_renderState.Initialize(defaultRenderState)))
		{
			EAE6320_ASSERT(false);
			goto OnExit;
		}
	}
	if (!(result = InitializePlatform()))
	{
		EAE6320_ASSERT(false);
		goto OnExit;
	}

OnExit:

	return result;
}

eae6320::cResult eae6320::Graphics::cEffect::CleanUp()
{
	auto result = eae6320::Results::Success;

	if (m_vertexShader)
	{
		const auto localResult = cShader::s_manager.Release(m_vertexShader);
		if (!localResult)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = localResult;
			}
		}
	}
	if (m_fragmentShader)
	{
		const auto localResult = cShader::s_manager.Release(m_fragmentShader);
		if (!localResult)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = localResult;
			}
		}
	}
	{
		const auto localResult = m_renderState.CleanUp();
		if (!localResult)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = localResult;
			}
		}
	}
	{
		const auto localResult = CleanUpPlatform();
		if (!localResult)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = localResult;
			}
		}
	}

	return result;
}

eae6320::Graphics::cEffect::~cEffect()
{
	CleanUp();
}