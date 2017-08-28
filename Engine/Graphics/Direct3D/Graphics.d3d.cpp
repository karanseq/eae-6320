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
	// In Direct3D "views" are objects that allow a texture to be used a particular way:
	// A render target view allows a texture to have color rendered to it
	ID3D11RenderTargetView* s_renderTargetView = nullptr;
	// A depth/stencil view allows a texture to have depth rendered to it
	ID3D11DepthStencilView* s_depthStencilView = nullptr;

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

	eae6320::Graphics::cRenderState s_renderState;

	// Geometry Data
	//--------------

	// A vertex buffer holds the data for each vertex
	ID3D11Buffer* s_vertexBuffer = nullptr;
	// D3D has an "input layout" object that associates the layout of the vertex format struct
	// with the input from a vertex shader
	ID3D11InputLayout* s_vertexInputLayout = nullptr;
}

// Helper Function Declarations
//=============================

namespace
{
	eae6320::cResult InitializeGeometry();
	eae6320::cResult InitializeShadingData();
	eae6320::cResult InitializeViews( const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight );
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

	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT( direct3dImmediateContext );

	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		EAE6320_ASSERT( s_renderTargetView );

		// Black is usually used
		constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		direct3dImmediateContext->ClearRenderTargetView( s_renderTargetView, clearColor );
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
			ID3D11ClassInstance* const* noInterfaces = nullptr;
			constexpr unsigned int interfaceCount = 0;
			// Vertex shader
			{
				EAE6320_ASSERT( s_vertexShader );
				auto* const shader = cShader::s_manager.Get( s_vertexShader );
				EAE6320_ASSERT( shader && shader->m_shaderObject.vertex );
				direct3dImmediateContext->VSSetShader( shader->m_shaderObject.vertex, noInterfaces, interfaceCount );
			}
			// Fragment shader
			{
				EAE6320_ASSERT( s_fragmentShader );
				auto* const shader = cShader::s_manager.Get( s_fragmentShader );
				EAE6320_ASSERT( shader && shader->m_shaderObject.fragment );
				direct3dImmediateContext->PSSetShader( shader->m_shaderObject.fragment, noInterfaces, interfaceCount );
			}
		}
		s_renderState.Bind();
	}
	// Draw the geometry
	{
		// Bind a specific vertex buffer to the device as a data source
		{
			EAE6320_ASSERT( s_vertexBuffer );
			constexpr unsigned int startingSlot = 0;
			constexpr unsigned int vertexBufferCount = 1;
			// The "stride" defines how large a single vertex is in the stream of data
			constexpr unsigned int bufferStride = sizeof( VertexFormats::sGeometry );
			// It's possible to start streaming data in the middle of a vertex buffer
			constexpr unsigned int bufferOffset = 0;
			direct3dImmediateContext->IASetVertexBuffers( startingSlot, vertexBufferCount, &s_vertexBuffer, &bufferStride, &bufferOffset );
		}
		// Specify what kind of data the vertex buffer holds
		{
			// Set the layout (which defines how to interpret a single vertex)
			{
				EAE6320_ASSERT( s_vertexInputLayout );
				direct3dImmediateContext->IASetInputLayout( s_vertexInputLayout );
			}
			// Set the topology (which defines how to interpret multiple vertices as a single "primitive";
			// the vertex buffer was defined as a triangle list
			// (meaning that every primitive is a triangle and will be defined by three vertices)
			direct3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		}
		// Render triangles from the currently-bound vertex buffer
		{
			// As of this comment only a single triangle is drawn
			// (you will have to update this code in future assignments!)
			constexpr unsigned int triangleCount = 1;
			constexpr unsigned int vertexCountPerTriangle = 3;
			constexpr auto vertexCountToRender = triangleCount * vertexCountPerTriangle;
			// It's possible to start rendering primitives in the middle of the stream
			constexpr unsigned int indexOfFirstVertexToRender = 0;
			direct3dImmediateContext->Draw( vertexCountToRender, indexOfFirstVertexToRender );
		}
	}

	// Everything has been drawn to the "back buffer", which is just an image in memory.
	// In order to display it the contents of the back buffer must be "presented"
	// (or "swapped" with the "front buffer")
	{
		auto* const swapChain = sContext::g_context.swapChain;
		EAE6320_ASSERT( swapChain );
		constexpr unsigned int swapImmediately = 0;
		constexpr unsigned int presentNextFrame = 0;
		const auto result = swapChain->Present( swapImmediately, presentNextFrame );
		EAE6320_ASSERT( SUCCEEDED( result ) );
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
	// Initialize the views
	{
		if ( !( result = InitializeViews( i_initializationParameters.resolutionWidth, i_initializationParameters.resolutionHeight ) ) )
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

	if ( s_renderTargetView )
	{
		s_renderTargetView->Release();
		s_renderTargetView = nullptr;
	}
	if ( s_depthStencilView )
	{
		s_depthStencilView->Release();
		s_depthStencilView = nullptr;
	}
	if ( s_vertexBuffer )
	{
		s_vertexBuffer->Release();
		s_vertexBuffer = nullptr;
	}
	if ( s_vertexInputLayout )
	{
		s_vertexInputLayout->Release();
		s_vertexInputLayout = nullptr;
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

		auto* const direct3dDevice = eae6320::Graphics::sContext::g_context.direct3dDevice;
		EAE6320_ASSERT( direct3dDevice );

		// Initialize vertex format
		{
			// Load the compiled binary vertex shader for the input layout
			eae6320::Platform::sDataFromFile vertexShaderDataFromFile;
			std::string errorMessage;
			if ( result = eae6320::Platform::LoadBinaryFile( "data/Shaders/Vertex/vertexInputLayout_geometry.shd", vertexShaderDataFromFile, &errorMessage ) )
			{
				// Create the vertex layout

				// These elements must match the VertexFormats::sGeometry layout struct exactly.
				// They instruct Direct3D how to match the binary data in the vertex buffer
				// to the input elements in a vertex shader
				// (by using so-called "semantic" names so that, for example,
				// "POSITION" here matches with "POSITION" in shader code).
				// Note that OpenGL uses arbitrarily assignable number IDs to do the same thing.
				constexpr unsigned int vertexElementCount = 1;
				D3D11_INPUT_ELEMENT_DESC layoutDescription[vertexElementCount] = {};
				{
					// Slot 0

					// POSITION
					// 2 floats == 8 bytes
					// Offset = 0
					{
						auto& positionElement = layoutDescription[0];

						positionElement.SemanticName = "POSITION";
						positionElement.SemanticIndex = 0;	// (Semantics without modifying indices at the end can always use zero)
						positionElement.Format = DXGI_FORMAT_R32G32_FLOAT;
						positionElement.InputSlot = 0;
						positionElement.AlignedByteOffset = offsetof( eae6320::Graphics::VertexFormats::sGeometry, x );
						positionElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
						positionElement.InstanceDataStepRate = 0;	// (Must be zero for per-vertex data)
					}
				}

				const auto d3dResult = direct3dDevice->CreateInputLayout( layoutDescription, vertexElementCount,
					vertexShaderDataFromFile.data, vertexShaderDataFromFile.size, &s_vertexInputLayout );
				if ( FAILED( result ) )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, "Geometry vertex input layout creation failed (HRESULT %#010x)", d3dResult );
					eae6320::Logging::OutputError( "Direct3D failed to create the geometry vertex input layout (HRESULT %#010x)", d3dResult );
				}

				vertexShaderDataFromFile.Free();
			}
			else
			{
				EAE6320_ASSERTF( false, errorMessage.c_str() );
				eae6320::Logging::OutputError( "The geometry vertex input layout shader couldn't be loaded: %s", errorMessage.c_str() );
				goto OnExit;
			}
		}
		// Vertex Buffer
		{
			constexpr unsigned int triangleCount = 1;
			constexpr unsigned int vertexCountPerTriangle = 3;
			const auto vertexCount = triangleCount * vertexCountPerTriangle;
			eae6320::Graphics::VertexFormats::sGeometry vertexData[vertexCount];
			{
				vertexData[0].x = 0.0f;
				vertexData[0].y = 0.0f;

				vertexData[1].x = 1.0f;
				vertexData[1].y = 1.0f;

				vertexData[2].x = 1.0f;
				vertexData[2].y = 0.0f;
			}
			D3D11_BUFFER_DESC bufferDescription{};
			{
				const auto bufferSize = vertexCount * sizeof( eae6320::Graphics::VertexFormats::sGeometry );
				EAE6320_ASSERT( bufferSize < ( uint64_t( 1u ) << ( sizeof( bufferDescription.ByteWidth ) * 8 ) ) );
				bufferDescription.ByteWidth = static_cast<unsigned int>( bufferSize );
				bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
				bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
				bufferDescription.MiscFlags = 0;
				bufferDescription.StructureByteStride = 0;	// Not used
			}
			D3D11_SUBRESOURCE_DATA initialData{};
			{
				initialData.pSysMem = vertexData;
				// (The other data members are ignored for non-texture buffers)
			}

			const auto d3dResult = direct3dDevice->CreateBuffer( &bufferDescription, &initialData, &s_vertexBuffer );
			if ( FAILED( d3dResult ) )
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF( false, "Geometry vertex buffer creation failed (HRESULT %#010x)", d3dResult );
				eae6320::Logging::OutputError( "Direct3D failed to create a geometry vertex buffer (HRESULT %#010x)", d3dResult );
				goto OnExit;
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

	OnExit:

		return result;
	}

	eae6320::cResult InitializeViews( const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight )
	{
		auto result = eae6320::Results::Success;

		ID3D11Texture2D* backBuffer = nullptr;
		ID3D11Texture2D* depthBuffer = nullptr;

		auto& g_context = eae6320::Graphics::sContext::g_context;
		auto* const direct3dDevice = g_context.direct3dDevice;
		EAE6320_ASSERT( direct3dDevice );
		auto* const direct3dImmediateContext = g_context.direct3dImmediateContext;
		EAE6320_ASSERT( direct3dImmediateContext );

		// Create a "render target view" of the back buffer
		// (the back buffer was already created by the call to D3D11CreateDeviceAndSwapChain(),
		// but a "view" of it is required to use as a "render target",
		// meaning a texture that the GPU can render to)
		{
			// Get the back buffer from the swap chain
			{
				constexpr unsigned int bufferIndex = 0;	// This must be 0 since the swap chain is discarded
				const auto d3dResult = g_context.swapChain->GetBuffer( bufferIndex, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &backBuffer ) );
				if ( FAILED( d3dResult ) )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, "Couldn't get the back buffer from the swap chain (HRESULT %#010x)", d3dResult );
					eae6320::Logging::OutputError( "Direct3D failed to get the back buffer from the swap chain (HRESULT %#010x)", d3dResult );
					goto OnExit;
				}
			}
			// Create the view
			{
				constexpr D3D11_RENDER_TARGET_VIEW_DESC* const accessAllSubResources = nullptr;
				const auto d3dResult = direct3dDevice->CreateRenderTargetView( backBuffer, accessAllSubResources, &s_renderTargetView );
				if ( FAILED( d3dResult ) )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, "Couldn't create render target view (HRESULT %#010x)", d3dResult );
					eae6320::Logging::OutputError( "Direct3D failed to create the render target view (HRESULT %#010x)", d3dResult );
					goto OnExit;
				}
			}
		}
		// Create a depth/stencil buffer and a view of it
		{
			// Unlike the back buffer no depth/stencil buffer exists until and unless it is explicitly created
			{
				D3D11_TEXTURE2D_DESC textureDescription{};
				{
					textureDescription.Width = i_resolutionWidth;
					textureDescription.Height = i_resolutionHeight;
					textureDescription.MipLevels = 1;	// A depth buffer has no MIP maps
					textureDescription.ArraySize = 1;
					textureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 24 bits for depth and 8 bits for stencil
					{
						DXGI_SAMPLE_DESC& sampleDescription = textureDescription.SampleDesc;

						sampleDescription.Count = 1;	// No multisampling
						sampleDescription.Quality = 0;	// Doesn't matter when Count is 1
					}
					textureDescription.Usage = D3D11_USAGE_DEFAULT;	// Allows the GPU to write to it
					textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
					textureDescription.CPUAccessFlags = 0;	// CPU doesn't need access
					textureDescription.MiscFlags = 0;
				}
				// The GPU renders to the depth/stencil buffer and so there is no initial data
				// (like there would be with a traditional texture loaded from disk)
				constexpr D3D11_SUBRESOURCE_DATA* const noInitialData = nullptr;
				const auto d3dResult = direct3dDevice->CreateTexture2D( &textureDescription, noInitialData, &depthBuffer );
				if ( FAILED( d3dResult ) )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, "Couldn't create depth buffer (HRESULT %#010x)", d3dResult );
					eae6320::Logging::OutputError( "Direct3D failed to create the depth buffer resource (HRESULT %#010x)", d3dResult );
					goto OnExit;
				}
			}
			// Create the view
			{
				constexpr D3D11_DEPTH_STENCIL_VIEW_DESC* const noSubResources = nullptr;
				const auto d3dResult = direct3dDevice->CreateDepthStencilView( depthBuffer, noSubResources, &s_depthStencilView );
				if ( FAILED( d3dResult ) )
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF( false, "Couldn't create depth stencil view (HRESULT %#010x)", d3dResult );
					eae6320::Logging::OutputError( "Direct3D failed to create the depth stencil view (HRESULT %#010x)", d3dResult );
					goto OnExit;
				}
			}
		}

		// Bind the views
		{
			constexpr unsigned int renderTargetCount = 1;
			direct3dImmediateContext->OMSetRenderTargets( renderTargetCount, &s_renderTargetView, s_depthStencilView );
		}
		// Specify that the entire render target should be visible
		{
			D3D11_VIEWPORT viewPort{};
			{
				viewPort.TopLeftX = viewPort.TopLeftY = 0.0f;
				viewPort.Width = static_cast<float>( i_resolutionWidth );
				viewPort.Height = static_cast<float>( i_resolutionHeight );
				viewPort.MinDepth = 0.0f;
				viewPort.MaxDepth = 1.0f;
			}
			constexpr unsigned int viewPortCount = 1;
			direct3dImmediateContext->RSSetViewports( viewPortCount, &viewPort );
		}

	OnExit:

		// Regardless of success or failure the two texture resources should be released
		// (if successful the views will hold internal references to the resources)
		if ( backBuffer )
		{
			backBuffer->Release();
			backBuffer = nullptr;
		}
		if ( depthBuffer )
		{
			depthBuffer->Release();
			depthBuffer= nullptr;
		}

		return result;
	}
}
