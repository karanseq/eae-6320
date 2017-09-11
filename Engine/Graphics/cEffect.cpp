// Include Files
//==============

#include "cEffect.h"

// Static Data Initialization
//===========================

const std::string eae6320::Graphics::cEffect::s_vertexShaderFolderPath("data/Shaders/Vertex/");
const std::string eae6320::Graphics::cEffect::s_fragmentShaderFolderPath("data/Shaders/Fragment/");
const std::string eae6320::Graphics::cEffect::s_shaderFileExtension(".shd");

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

eae6320::cResult eae6320::Graphics::cEffect::Initialize()
{
	auto result = eae6320::Results::Success;

	{
		std::string vertexFilePath(s_vertexShaderFolderPath);
		vertexFilePath.append(m_vertexShaderFileName);
		vertexFilePath.append(s_shaderFileExtension);

		if (!(result = eae6320::Graphics::cShader::s_manager.Load(vertexFilePath.c_str(),
			m_vertexShader, eae6320::Graphics::ShaderTypes::Vertex)))
		{
			EAE6320_ASSERT(false);
			goto OnExit;
		}
	}

	{
		std::string vertexFilePath(s_fragmentShaderFolderPath);
		vertexFilePath.append(m_fragmentShaderFileName);
		vertexFilePath.append(s_shaderFileExtension);

		if (!(result = eae6320::Graphics::cShader::s_manager.Load(vertexFilePath.c_str(),
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

eae6320::Graphics::cEffect::cEffect(const std::string& i_vertexShaderFileName, const std::string& i_fragmentShaderFileName)
	: m_vertexShaderFileName(i_vertexShaderFileName), m_fragmentShaderFileName(i_fragmentShaderFileName)
{

}

eae6320::Graphics::cEffect::~cEffect()
{
	CleanUp();
}