// Include Files
//==============

#include "../Graphics.h"

#include "Includes.h"
#include "../cConstantBuffer.h"
#include "../ConstantBufferFormats.h"
#include "../cRenderState.h"
#include "../cSamplerState.h"
#include "../cShader.h"
#include "../sContext.h"
#include "../VertexFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Concurrency/cEvent.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Time/Time.h>
#include <Engine/UserOutput/UserOutput.h>
#include <utility>

// Static Data Initialization
//===========================

namespace
{
	// Constant buffer object
	eae6320::Graphics::cConstantBuffer s_constantBuffer_perFrame( eae6320::Graphics::ConstantBufferTypes::PerFrame );
	// In our class we will only have a single sampler state
	eae6320::Graphics::cSamplerState s_samplerState;

	// Submission Data
	//----------------

	// This struct's data is populated at submission time;
	// it must cache whatever is necessary in order to render a frame
	struct sDataRequiredToRenderAFrame
	{
		eae6320::Graphics::ConstantBufferFormats::sPerFrame constantData_perFrame;
	};
	// In our class there will be two copies of the data required to render a frame:
	//	* One of them will be getting populated by the data currently being submitted by the application loop thread
	//	* One of them will be fully populated, 
	sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
	auto* s_dataBeingSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
	auto* s_dataBeingRenderedByRenderThread = &s_dataRequiredToRenderAFrame[1];
	// The following two events work together to make sure that
	// the main/render thread and the application loop thread can work in parallel but stay in sync:
	// This event is signaled by the application loop thread when it has finished submitting render data for a frame
	// (the main/render thread waits for the signal)
	eae6320::Concurrency::cEvent s_whenAllDataHasBeenSubmittedFromApplicationThread;
	// This event is signaled by the main/render thread when it has swapped render data pointers.
	// This means that the renderer is now working with all the submitted data it needs to render the next frame,
	// and the application loop thread can start submitting data for the following frame
	// (the application loop thread waits for the signal)
	eae6320::Concurrency::cEvent s_whenDataForANewFrameCanBeSubmittedFromApplicationThread;

	// Shading Data
	//-------------

	eae6320::Graphics::cShader::Handle s_vertexShader;
	eae6320::Graphics::cShader::Handle s_fragmentShader;
	GLuint s_programId = 0;

	eae6320::Graphics::cRenderState s_renderState;

	// Geometry Data
	//--------------

	// A vertex buffer holds the data for each vertex
	GLuint s_vertexBufferId = 0;
	// A vertex array encapsulates the vertex data as well as the vertex input layout
	GLuint s_vertexArrayId = 0;
}

// Helper Function Declarations
//=============================

namespace
{
	eae6320::cResult InitializeGeometry();
	eae6320::cResult InitializeShadingData();
}

// Interface
//==========

// Submission
//-----------

void eae6320::Graphics::SubmitElapsedTime( const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_simulationTime )
{
	EAE6320_ASSERT( s_dataBeingSubmittedByApplicationThread );
	auto& constantData_perFrame = s_dataBeingSubmittedByApplicationThread->constantData_perFrame;
	constantData_perFrame.g_elapsedSecondCount_systemTime = i_elapsedSecondCount_systemTime;
	constantData_perFrame.g_elapsedSecondCount_simulationTime = i_elapsedSecondCount_simulationTime;
}

eae6320::cResult eae6320::Graphics::WaitUntilDataForANewFrameCanBeSubmitted( const unsigned int i_timeToWait_inMilliseconds )
{
	return Concurrency::WaitForEvent( s_whenDataForANewFrameCanBeSubmittedFromApplicationThread, i_timeToWait_inMilliseconds );
}

eae6320::cResult eae6320::Graphics::SignalThatAllDataForAFrameHasBeenSubmitted()
{
	return s_whenAllDataHasBeenSubmittedFromApplicationThread.Signal();
}

// Render
//-------

void eae6320::Graphics::RenderFrame()
{
	// Wait for the application loop to submit data to be rendered
	{
		const auto result = Concurrency::WaitForEvent( s_whenAllDataHasBeenSubmittedFromApplicationThread );
		if ( result )
		{
			// Switch the render data pointers so that
			// the data that the application just submitted becomes the data that will now be rendered
			std::swap( s_dataBeingSubmittedByApplicationThread, s_dataBeingRenderedByRenderThread );
			// Once the pointers have been swapped the application loop can submit new data
			const auto result = s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Signal();
			if ( !result )
			{
				EAE6320_ASSERTF( false, "Couldn't signal that new graphics data can be submitted" );
				Logging::OutputError( "Failed to signal that new render data can be submitted" );
				UserOutput::Print( "The renderer failed to signal to the application that new graphics data can be submitted."
					" The application is probably in a bad state and should be exited" );
				return;
			}
		}
		else
		{
			EAE6320_ASSERTF( false, "Waiting for the graphics data to be submitted failed" );
			Logging::OutputError( "Waiting for the application loop to submit data to be rendered failed" );
			UserOutput::Print( "The renderer failed to wait for the application to submit data to be rendered."
				" The application is probably in a bad state and should be exited" );
			return;
		}
	}

	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		// Black is usually used
		{
			glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
			EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
		}
		{
			constexpr GLbitfield clearColor = GL_COLOR_BUFFER_BIT;
			glClear( clearColor );
			EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
		}
	}

	EAE6320_ASSERT( s_dataBeingRenderedByRenderThread );

	// Update the per-frame constant buffer
	{
		// Copy the data from the system memory that the application owns to GPU memory
		auto& constantData_perFrame = s_dataBeingRenderedByRenderThread->constantData_perFrame;
		s_constantBuffer_perFrame.Update( &constantData_perFrame );
	}

	// Bind the shading data
	{
		{
			EAE6320_ASSERT( s_programId != 0 );
			glUseProgram( s_programId );
			EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
		}
		s_renderState.Bind();
	}
	// Draw the geometry
	{
		// Bind a specific vertex buffer to the device as a data source
		{
			glBindVertexArray( s_vertexArrayId );
			EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
		}
		// Render triangles from the currently-bound vertex buffer
		{
			// The mode defines how to interpret multiple vertices as a single "primitive";
			// a triangle list is defined
			// (meaning that every primitive is a triangle and will be defined by three vertices)
			constexpr GLenum mode = GL_TRIANGLES;
			// It's possible to start rendering primitives in the middle of the stream
			constexpr GLint indexOfFirstVertexToRender = 0;
			// As of this comment we are only drawing a single triangle
			// (you will have to update this code in future assignments!)
			constexpr unsigned int triangleCount = 1;
			constexpr unsigned int vertexCountPerTriangle = 3;
			constexpr auto vertexCountToRender = triangleCount * vertexCountPerTriangle;
			glDrawArrays( mode, indexOfFirstVertexToRender, vertexCountToRender );
			EAE6320_ASSERT( glGetError() == GL_NO_ERROR );
		}
	}

	// Everything has been drawn to the "back buffer", which is just an image in memory.
	// In order to display it the contents of the back buffer must be "presented"
	// (or "swapped" with the "front buffer")
	{
		const auto deviceContext = sContext::g_context.deviceContext;
		EAE6320_ASSERT( deviceContext != NULL );

		const auto glResult = SwapBuffers( deviceContext );
		EAE6320_ASSERT( glResult != FALSE );
	}

	// Once everything has been drawn the data that was submitted for this frame
	// should be cleaned up and cleared.
	// so that the struct can be re-used (i.e. so that data for a new frame can be submitted to it)
	{
		// (At this point in the class there isn't anything that needs to be cleaned up)
	}
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::Initialize( const sInitializationParameters& i_initializationParameters )
{
	auto result = Results::Success;

	// Initialize the platform-specific context
	if ( !( result = sContext::g_context.Initialize( i_initializationParameters ) ) )
	{
		EAE6320_ASSERT( false );
		goto OnExit;
	}
	// Initialize the asset managers
	{
		if ( !( result = cShader::s_manager.Initialize() ) )
		{
			EAE6320_ASSERT( false );
			goto OnExit;
		}
	}

	// Initialize the platform-independent graphics objects
	{
		if ( result = s_constantBuffer_perFrame.Initialize() )
		{
			// There is only a single per-frame constant buffer that is re-used
			// and so it can be bound at initialization time and never unbound
			s_constantBuffer_perFrame.Bind(
				// In our class both vertex and fragment shaders use per-frame constant data
				ShaderTypes::Vertex | ShaderTypes::Fragment );
		}
		else
		{
			EAE6320_ASSERT( false );
			goto OnExit;
		}
		if ( result = s_samplerState.Initialize() )
		{
			// There is only a single sampler state that is re-used
			// and so it can be bound at initialization time and never unbound
			s_samplerState.Bind();
		}
		else
		{
			EAE6320_ASSERT( false );
			goto OnExit;
		}
	}
	// Initialize the events
	{
		if ( !( result = s_whenAllDataHasBeenSubmittedFromApplicationThread.Initialize( Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled ) ) )
		{
			EAE6320_ASSERT( false );
			goto OnExit;
		}
		if ( !( result = s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Initialize( Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled,
			Concurrency::EventState::Signaled ) ) )
		{
			EAE6320_ASSERT( false );
			goto OnExit;
		}
	}
	// Initialize the shading data
	{
		if ( !( result = InitializeShadingData() ) )
		{
			EAE6320_ASSERT( false );
			goto OnExit;
		}
	}
	// Initialize the geometry
	{
		if ( !( result = InitializeGeometry() ) )
		{
			EAE6320_ASSERT( false );
			goto OnExit;
		}
	}

OnExit:

	return result;
}

eae6320::cResult eae6320::Graphics::CleanUp()
{
	auto result = Results::Success;

	{
		if ( s_vertexArrayId != 0 )
		{
			// Make sure that the vertex array isn't bound
			{
				// Unbind the vertex array
				glBindVertexArray( 0 );
				const auto errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					if ( result )
					{
						result = Results::Failure;
					}
					EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					Logging::OutputError( "OpenGL failed to unbind all vertex arrays before cleaning up geometry: %s",
						reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				}
			}
			constexpr GLsizei arrayCount = 1;
			glDeleteVertexArrays( arrayCount, &s_vertexArrayId );
			const auto errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				if ( result )
				{
					result = Results::Failure;
				}
				EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				Logging::OutputError( "OpenGL failed to delete the vertex array: %s",
					reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
			}
			s_vertexArrayId = 0;
		}
		if ( s_vertexBufferId != 0 )
		{
			constexpr GLsizei bufferCount = 1;
			glDeleteBuffers( bufferCount, &s_vertexBufferId );
			const auto errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				if ( result )
				{
					result = Results::Failure;
				}
				EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				Logging::OutputError( "OpenGL failed to delete the vertex buffer: %s",
					reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
			}
			s_vertexBufferId = 0;
		}
	}
	if ( s_programId != 0 )
	{
		glDeleteProgram( s_programId );
		const auto errorCode = glGetError();
		if ( errorCode != GL_NO_ERROR )
		{
			if ( result )
			{
				result = eae6320::Results::Failure;
			}
			EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
			eae6320::Logging::OutputError( "OpenGL failed to delete the program: %s",
				reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
		}
		s_programId = 0;
	}
	if ( s_vertexShader )
	{
		const auto localResult = cShader::s_manager.Release( s_vertexShader );
		if ( !localResult )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = localResult;
			}
		}
	}
	if ( s_fragmentShader )
	{
		const auto localResult = cShader::s_manager.Release( s_fragmentShader );
		if ( !localResult )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = localResult;
			}
		}
	}
	{
		const auto localResult = s_renderState.CleanUp();
		if ( !localResult )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = localResult;
			}
		}
	}

	{
		const auto localResult = s_constantBuffer_perFrame.CleanUp();
		if ( !localResult )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = localResult;
			}
		}
	}
	{
		const auto localResult = s_samplerState.CleanUp();
		if ( !localResult )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = localResult;
			}
		}
	}

	{
		const auto localResult = cShader::s_manager.CleanUp();
		if ( !localResult )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = localResult;
			}
		}
	}
	
	{
		const auto localResult = sContext::g_context.CleanUp();
		if ( !localResult )
		{
			EAE6320_ASSERT( false );
			if ( result )
			{
				result = localResult;
			}
		}
	}

	return result;
}

// Helper Function Definitions
//============================

namespace
{
	eae6320::cResult InitializeGeometry()
	{
		auto result = eae6320::Results::Success;

		// Create a vertex array object and make it active
		{
			constexpr GLsizei arrayCount = 1;
			glGenVertexArrays( arrayCount, &s_vertexArrayId );
			const auto errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				glBindVertexArray( s_vertexArrayId );
				const auto errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					eae6320::Logging::OutputError( "OpenGL failed to bind a new vertex array: %s",
						reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					goto OnExit;
				}
			}
			else
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				eae6320::Logging::OutputError( "OpenGL failed to get an unused vertex array ID: %s",
					reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				goto OnExit;
			}
		}
		// Create a vertex buffer object and make it active
		{
			constexpr GLsizei bufferCount = 1;
			glGenBuffers( bufferCount, &s_vertexBufferId );
			const auto errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				glBindBuffer( GL_ARRAY_BUFFER, s_vertexBufferId );
				const auto errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					eae6320::Logging::OutputError( "OpenGL failed to bind a new vertex buffer: %s",
						reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					goto OnExit;
				}
			}
			else
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				eae6320::Logging::OutputError( "OpenGL failed to get an unused vertex buffer ID: %s",
					reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				goto OnExit;
			}
		}
		// Assign the data to the buffer
		{
			constexpr unsigned int triangleCount = 1;
			constexpr unsigned int vertexCountPerTriangle = 3;
			const auto vertexCount = triangleCount * vertexCountPerTriangle;
			eae6320::Graphics::VertexFormats::sGeometry vertexData[vertexCount];
			{
				vertexData[0].x = 0.0f;
				vertexData[0].y = 0.0f;

				vertexData[1].x = 1.0f;
				vertexData[1].y = 0.0f;

				vertexData[2].x = 1.0f;
				vertexData[2].y = 1.0f;
			}
			const auto bufferSize = vertexCount * sizeof( eae6320::Graphics::VertexFormats::sGeometry );
			EAE6320_ASSERT( bufferSize < ( uint64_t( 1u ) << ( sizeof( GLsizeiptr ) * 8 ) ) );
			glBufferData( GL_ARRAY_BUFFER, static_cast<GLsizeiptr>( bufferSize ), reinterpret_cast<GLvoid*>( vertexData ),
				// In our class we won't ever read from the buffer
				GL_STATIC_DRAW );
			const auto errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				eae6320::Logging::OutputError( "OpenGL failed to allocate the vertex buffer: %s",
					reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				goto OnExit;
			}
		}
		// Initialize vertex format
		{
			// The "stride" defines how large a single vertex is in the stream of data
			// (or, said another way, how far apart each position element is)
			const auto stride = static_cast<GLsizei>( sizeof( eae6320::Graphics::VertexFormats::sGeometry ) );

			// Position (0)
			// 2 floats == 8 bytes
			// Offset = 0
			{
				constexpr GLuint vertexElementLocation = 0;
				constexpr GLint elementCount = 2;
				constexpr GLboolean notNormalized = GL_FALSE;	// The given floats should be used as-is
				glVertexAttribPointer( vertexElementLocation, elementCount, GL_FLOAT, notNormalized, stride,
					reinterpret_cast<GLvoid*>( offsetof( eae6320::Graphics::VertexFormats::sGeometry, x ) ) );
				const auto errorCode = glGetError();
				if ( errorCode == GL_NO_ERROR )
				{
					glEnableVertexAttribArray( vertexElementLocation );
					const GLenum errorCode = glGetError();
					if ( errorCode != GL_NO_ERROR )
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						eae6320::Logging::OutputError( "OpenGL failed to enable the POSITION vertex attribute at location %u: %s",
							vertexElementLocation, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						goto OnExit;
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					eae6320::Logging::OutputError( "OpenGL failed to set the POSITION vertex attribute at location %u: %s",
						vertexElementLocation, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					goto OnExit;
				}
			}
		}

	OnExit:

		return result;
	}

	eae6320::cResult InitializeShadingData()
	{
		auto result = eae6320::Results::Success;

		if ( !( result = eae6320::Graphics::cShader::s_manager.Load( "data/Shaders/Vertex/example.shd",
			s_vertexShader, eae6320::Graphics::ShaderTypes::Vertex ) ) )
		{
			EAE6320_ASSERT( false );
			goto OnExit;
		}
		if ( !( result = eae6320::Graphics::cShader::s_manager.Load( "data/Shaders/Fragment/example.shd",
			s_fragmentShader, eae6320::Graphics::ShaderTypes::Fragment ) ) )
		{
			EAE6320_ASSERT( false );
			goto OnExit;
		}
		{
			constexpr uint8_t defaultRenderState = 0;
			if ( !( result = s_renderState.Initialize( defaultRenderState ) ) )
			{
				EAE6320_ASSERT( false );
				goto OnExit;
			}
		}

		// Create a program
		{
			s_programId = glCreateProgram();
			const auto errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				eae6320::Logging::OutputError( "OpenGL failed to create a program: %s",
					reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				goto OnExit;
			}
			else if ( s_programId == 0 )
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERT( false );
				eae6320::Logging::OutputError( "OpenGL failed to create a program" );
				goto OnExit;
			}
		}
		// Attach the shaders to the program
		{
			// Vertex
			{
				glAttachShader( s_programId, eae6320::Graphics::cShader::s_manager.Get( s_vertexShader )->m_shaderId );
				const auto errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					eae6320::Logging::OutputError( "OpenGL failed to attach the vertex shader to the program: %s",
						reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					goto OnExit;
				}
			}
			// Fragment
			{
				glAttachShader( s_programId, eae6320::Graphics::cShader::s_manager.Get( s_fragmentShader )->m_shaderId );
				const auto errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					eae6320::Logging::OutputError( "OpenGL failed to attach the fragment shader to the program: %s",
						reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					goto OnExit;
				}
			}
		}
		// Link the program
		{
			glLinkProgram( s_programId );
			const auto errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				// Get link info
				// (this won't be used unless linking fails
				// but it can be useful to look at when debugging)
				std::string linkInfo;
				{
					GLint infoSize;
					glGetProgramiv( s_programId, GL_INFO_LOG_LENGTH, &infoSize );
					const auto errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						struct sLogInfo
						{
							GLchar* memory;
							sLogInfo( const size_t i_size ) { memory = reinterpret_cast<GLchar*>( malloc( i_size ) ); }
							~sLogInfo() { if ( memory ) free( memory ); }
						} info( static_cast<size_t>( infoSize ) );
						GLsizei* const dontReturnLength = nullptr;
						glGetProgramInfoLog( s_programId, static_cast<GLsizei>( infoSize ), dontReturnLength, info.memory );
						const auto errorCode = glGetError();
						if ( errorCode == GL_NO_ERROR )
						{
							linkInfo = info.memory;
						}
						else
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
							eae6320::Logging::OutputError( "OpenGL failed to get link info of the program: %s",
								reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
							goto OnExit;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						eae6320::Logging::OutputError( "OpenGL failed to get the length of the program link info: %s",
							reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						goto OnExit;
					}
				}
				// Check to see if there were link errors
				GLint didLinkingSucceed;
				{
					glGetProgramiv( s_programId, GL_LINK_STATUS, &didLinkingSucceed );
					const auto errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						if ( didLinkingSucceed == GL_FALSE )
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF( false, linkInfo.c_str() );
							eae6320::Logging::OutputError( "The program failed to link: %s",
								linkInfo.c_str() );
							goto OnExit;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						eae6320::Logging::OutputError( "OpenGL failed to find out if linking of the program succeeded: %s",
							reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
						goto OnExit;
					}
				}
			}
			else
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				eae6320::Logging::OutputError( "OpenGL failed to link the program: %s",
					reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				goto OnExit;
			}
		}

	OnExit:

		if ( !result )
		{
			if ( s_programId != 0 )
			{
				glDeleteProgram( s_programId );
				const auto errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
					eae6320::Logging::OutputError( "OpenGL failed to delete the program: %s",
						reinterpret_cast<const char*>( gluErrorString( errorCode ) ) );
				}
				s_programId = 0;
			}
		}
		return result;
	}
}
