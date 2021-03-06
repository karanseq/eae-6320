// Include Files
//==============

#include "Graphics.h"

#include "cConstantBuffer.h"
#include "ConstantBufferFormats.h"
#include "cEffect.h"
#include "cMesh.h"
#include "cSamplerState.h"
#include "cSprite.h"
#include "cTexture.h"
#include "cView.h"
#include "sCamera.h"
#include "sContext.h"
#include "sColor.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Concurrency/cEvent.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Math/cMatrix_transformation.h>
#include <Engine/Math/cQuaternion.h>
#include <Engine/Math/sVector.h>
#include <Engine/Math/sVector2d.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Time/Time.h>
#include <Engine/UserOutput/UserOutput.h>

#include <algorithm>
#include <utility>
#include <vector>

// Static Data Initialization
//===========================

namespace
{
    // A view is responsible for clearing the render target and swapping the buffers
    // In Direct3D, it is also responsible for initializing the render target & depth/stencil views
    eae6320::Graphics::cView                                        s_view;

    // Constant buffer objects
    eae6320::Graphics::cConstantBuffer                              s_constantBuffer_perFrame(eae6320::Graphics::ConstantBufferTypes::PerFrame);
    eae6320::Graphics::cConstantBuffer                              s_constantBuffer_perDrawCall(eae6320::Graphics::ConstantBufferTypes::PerDrawCall);
    // In our class we will only have a single sampler state
    eae6320::Graphics::cSamplerState                                s_samplerState;

    // Submission Data
    //----------------

    struct sDataRequiredToRenderASprite
    {
        eae6320::Graphics::cTexture*                                constantData_texture = nullptr;
        eae6320::Graphics::cEffect*                                 constantData_effect = nullptr;
        eae6320::Graphics::cSprite*                                 constantData_sprite = nullptr;
    };

    struct sDataRequiredToRenderAMesh
    {
        eae6320::Graphics::cTexture*                                constantData_texture = nullptr;
        eae6320::Graphics::cEffect*                                 constantData_effect = nullptr;
        eae6320::Graphics::cMesh*                                   constantData_mesh = nullptr;
        eae6320::Math::sVector                                      constantData_position;
        eae6320::Math::cQuaternion                                  constantData_orientation;
    };

    // This struct's data is populated at submission time;
    // it must cache whatever is necessary in order to render a frame
    struct sDataRequiredToRenderAFrame
    {
        std::vector<sDataRequiredToRenderAMesh>                     meshRenderDataList;
        std::vector<sDataRequiredToRenderASprite>                   spriteRenderDataList;
        eae6320::Graphics::ConstantBufferFormats::sPerFrame         constantData_perFrame;
        eae6320::Graphics::ConstantBufferFormats::sPerDrawCall      constantData_perDrawCall;
        eae6320::Graphics::sColor                                   backgroundColor;
        float                                                       depthBufferClearDepth;
    };
    // In our class there will be two copies of the data required to render a frame:
    //	* One of them will be getting populated by the data currently being submitted by the application loop thread
    //	* One of them will be fully populated, 
    sDataRequiredToRenderAFrame                                     s_dataRequiredToRenderAFrame[2];
    auto*                                                           s_dataBeingSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
    auto*                                                           s_dataBeingRenderedByRenderThread = &s_dataRequiredToRenderAFrame[1];
    // The following two events work together to make sure that
    // the main/render thread and the application loop thread can work in parallel but stay in sync:
    // This event is signaled by the application loop thread when it has finished submitting render data for a frame
    // (the main/render thread waits for the signal)
    eae6320::Concurrency::cEvent                                    s_whenAllDataHasBeenSubmittedFromApplicationThread;
    // This event is signaled by the main/render thread when it has swapped render data pointers.
    // This means that the renderer is now working with all the submitted data it needs to render the next frame,
    // and the application loop thread can start submitting data for the following frame
    // (the application loop thread waits for the signal)
    eae6320::Concurrency::cEvent                                    s_whenDataForANewFrameCanBeSubmittedFromApplicationThread;
}

// Interface
//==========

// Submission
//-----------

void eae6320::Graphics::SubmitElapsedTime(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_simulationTime)
{
    EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
    auto& constantData_perFrame = s_dataBeingSubmittedByApplicationThread->constantData_perFrame;
    constantData_perFrame.g_elapsedSecondCount_systemTime = i_elapsedSecondCount_systemTime;
    constantData_perFrame.g_elapsedSecondCount_simulationTime = i_elapsedSecondCount_simulationTime;
}

void eae6320::Graphics::SubmitBackgroundColor(const sColor& i_backgroundColor)
{
    EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
    s_dataBeingSubmittedByApplicationThread->backgroundColor = i_backgroundColor;
}

void eae6320::Graphics::SubmitDepthToClear(const float i_depth /* = 1.0f */)
{
    EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
    s_dataBeingSubmittedByApplicationThread->depthBufferClearDepth = i_depth;
}

void eae6320::Graphics::SubmitCamera(const sCamera& i_camera, const Math::sVector& i_position, const Math::cQuaternion& i_orientation)
{
    EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
    auto& constantData_perFrame = s_dataBeingSubmittedByApplicationThread->constantData_perFrame;
    constantData_perFrame.g_transform_worldToCamera = Math::cMatrix_transformation::CreateWorldToCameraTransform(i_orientation, i_position);
    constantData_perFrame.g_transform_cameraToProjected = Math::cMatrix_transformation::CreateCameraToProjectedTransform_perspective(i_camera.m_verticalFieldOfView_inRadians, i_camera.m_aspectRatio, i_camera.m_z_nearPlane, i_camera.m_z_farPlane);
}
    
void eae6320::Graphics::SubmitMeshToBeRendered(cMesh* i_meshToDraw, cEffect* i_effectToBind, cTexture* i_textureToBind, const Math::sVector& i_position, const Math::cQuaternion& i_orientation)
{
    EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
    EAE6320_ASSERT(i_meshToDraw && i_effectToBind && i_textureToBind);

    sDataRequiredToRenderAMesh meshRenderData;
    {
        meshRenderData.constantData_texture = i_textureToBind;
        meshRenderData.constantData_texture->IncrementReferenceCount();
        meshRenderData.constantData_mesh = i_meshToDraw;
        meshRenderData.constantData_mesh->IncrementReferenceCount();
        meshRenderData.constantData_effect = i_effectToBind;
        meshRenderData.constantData_effect->IncrementReferenceCount();
        meshRenderData.constantData_position = i_position;
        meshRenderData.constantData_orientation = i_orientation;
    }

    s_dataBeingSubmittedByApplicationThread->meshRenderDataList.push_back(meshRenderData);
}

void eae6320::Graphics::SubmitSpriteToBeRendered(cSprite* i_spriteToDraw, cEffect* i_effectToBind, cTexture* i_textureToBind)
{
    EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
    EAE6320_ASSERT(i_spriteToDraw && i_effectToBind && i_textureToBind);

    sDataRequiredToRenderASprite spriteRenderData;    
    {
        spriteRenderData.constantData_texture = i_textureToBind;
        spriteRenderData.constantData_texture->IncrementReferenceCount();
        spriteRenderData.constantData_effect = i_effectToBind;
        spriteRenderData.constantData_effect->IncrementReferenceCount();
        spriteRenderData.constantData_sprite = i_spriteToDraw;
        spriteRenderData.constantData_sprite->IncrementReferenceCount();
    }

    s_dataBeingSubmittedByApplicationThread->spriteRenderDataList.push_back(spriteRenderData);
}

eae6320::cResult eae6320::Graphics::WaitUntilDataForANewFrameCanBeSubmitted(const unsigned int i_timeToWait_inMilliseconds)
{
    return Concurrency::WaitForEvent(s_whenDataForANewFrameCanBeSubmittedFromApplicationThread, i_timeToWait_inMilliseconds);
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
        const auto result = Concurrency::WaitForEvent(s_whenAllDataHasBeenSubmittedFromApplicationThread);
        if (result)
        {
            // Switch the render data pointers so that
            // the data that the application just submitted becomes the data that will now be rendered
            std::swap(s_dataBeingSubmittedByApplicationThread, s_dataBeingRenderedByRenderThread);
            // Once the pointers have been swapped the application loop can submit new data
            const auto result = s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Signal();
            if (!result)
            {
                EAE6320_ASSERTF(false, "Couldn't signal that new graphics data can be submitted");
                Logging::OutputError("Failed to signal that new render data can be submitted");
                UserOutput::Print("The renderer failed to signal to the application that new graphics data can be submitted."
                    " The application is probably in a bad state and should be exited");
                return;
            }
        }
        else
        {
            EAE6320_ASSERTF(false, "Waiting for the graphics data to be submitted failed");
            Logging::OutputError("Waiting for the application loop to submit data to be rendered failed");
            UserOutput::Print("The renderer failed to wait for the application to submit data to be rendered."
                " The application is probably in a bad state and should be exited");
            return;
        }
    }

    EAE6320_ASSERT(s_dataBeingRenderedByRenderThread);

    // Every frame an entirely new image will be created.
    // Before drawing anything, then, the previous image will be erased
    // by "clearing" the image buffer (filling it with a solid color)
    {
        // Black is usually used
        s_view.ClearRenderTarget(s_dataBeingRenderedByRenderThread->backgroundColor);
    }

    s_view.ClearDepthBuffer(s_dataBeingRenderedByRenderThread->depthBufferClearDepth);

    // Update the per-frame constant buffer
    {
        // Copy the data from the system memory that the application owns to GPU memory
        auto& constantData_perFrame = s_dataBeingRenderedByRenderThread->constantData_perFrame;
        s_constantBuffer_perFrame.Update(&constantData_perFrame);
    }

    // Scratch-pad for transparent meshes
    struct sMeshIndexTransformPair
    {
        uint16_t indexIntoMeshRenderDataList = 0;
        float cameraSpaceZ = 0.0f;
    };
    uint16_t meshCounter = 0;
    std::vector<sMeshIndexTransformPair> transparentMeshData;

    // Draw the opaque meshes & filter out transparent mehses
    {
        for (const auto& meshRenderData : s_dataBeingRenderedByRenderThread->meshRenderDataList)
        {
            Math::cMatrix_transformation transform_localToWorld = Math::cMatrix_transformation(meshRenderData.constantData_orientation, meshRenderData.constantData_position);

            if (meshRenderData.constantData_effect->GetRenderState().IsAlphaTransparencyEnabled())
            {
                const Math::sVector translation_localToCamera = s_dataBeingRenderedByRenderThread->constantData_perFrame.g_transform_worldToCamera * transform_localToWorld.GetTranslation();
                transparentMeshData.push_back({ meshCounter, translation_localToCamera.z });
            }
            else
            {
                auto& constantData_perDrawCall = s_dataBeingRenderedByRenderThread->constantData_perDrawCall;
                {
                    constantData_perDrawCall.g_transform_localToWorld = transform_localToWorld;
                }
                s_constantBuffer_perDrawCall.Update(&constantData_perDrawCall);

                meshRenderData.constantData_effect->Bind();
                {
                    constexpr unsigned int id = 0;
                    meshRenderData.constantData_texture->Bind(id);
                }
                meshRenderData.constantData_mesh->Draw();
            }

            ++meshCounter;
        }
    }

    // Sort the transparent meshes from far-to-near in camera space and draw them
    {
        std::sort(transparentMeshData.begin(), transparentMeshData.end(), [](const sMeshIndexTransformPair& a, const sMeshIndexTransformPair& b) { return a.cameraSpaceZ < b.cameraSpaceZ; });

        for (const auto& meshIndexTransformPair : transparentMeshData)
        {
            const auto& meshRenderData = s_dataBeingRenderedByRenderThread->meshRenderDataList[meshIndexTransformPair.indexIntoMeshRenderDataList];
            auto& constantData_perDrawCall = s_dataBeingRenderedByRenderThread->constantData_perDrawCall;
            {
                constantData_perDrawCall.g_transform_localToWorld = Math::cMatrix_transformation(meshRenderData.constantData_orientation, meshRenderData.constantData_position);
            }
            s_constantBuffer_perDrawCall.Update(&constantData_perDrawCall);

            meshRenderData.constantData_effect->Bind();
            {
                constexpr unsigned int id = 0;
                meshRenderData.constantData_texture->Bind(id);
            }
            meshRenderData.constantData_mesh->Draw();
        }
    }

    // Draw the sprites
    {
        for (const auto& spriteRenderData : s_dataBeingRenderedByRenderThread->spriteRenderDataList)
        {
            spriteRenderData.constantData_effect->Bind();
            {
                constexpr unsigned int id = 0;
                spriteRenderData.constantData_texture->Bind(id);
            }
            spriteRenderData.constantData_sprite->Draw();
        }
    }

    // Everything has been drawn to the "back buffer", which is just an image in memory.
    // In order to display it the contents of the back buffer must be "presented"
    // (or "swapped" with the "front buffer")
    {
        s_view.Swap();
    }

    // Once everything has been drawn the data that was submitted for this frame
    // should be cleaned up and cleared.
    // so that the struct can be re-used (i.e. so that data for a new frame can be submitted to it)
    {
        for (auto& meshRenderData : s_dataBeingRenderedByRenderThread->meshRenderDataList)
        {
            meshRenderData.constantData_texture->DecrementReferenceCount();
            meshRenderData.constantData_effect->DecrementReferenceCount();
            meshRenderData.constantData_mesh->DecrementReferenceCount();
        }
        s_dataBeingRenderedByRenderThread->meshRenderDataList.clear();

        for (auto& spriteRenderData : s_dataBeingRenderedByRenderThread->spriteRenderDataList)
        {
            spriteRenderData.constantData_texture->DecrementReferenceCount();
            spriteRenderData.constantData_effect->DecrementReferenceCount();
            spriteRenderData.constantData_sprite->DecrementReferenceCount();
        }
        s_dataBeingRenderedByRenderThread->spriteRenderDataList.clear();
    }
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::Initialize(const sInitializationParameters& i_initializationParameters)
{
    auto result = Results::Success;

    // Initialize the platform-specific context
    if (!(result = sContext::g_context.Initialize(i_initializationParameters)))
    {
        EAE6320_ASSERT(false);
        goto OnExit;
    }

    // Initialize the asset managers
    {
        if (!(result = cShader::s_manager.Initialize()))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
    }

    // Initialize the platform-independent graphics objects
    {
        // Initialize per-frame constant buffer
        if (result = s_constantBuffer_perFrame.Initialize())
        {
            // There is only a single per-frame constant buffer that is re-used
            // and so it can be bound at initialization time and never unbound
            s_constantBuffer_perFrame.Bind(
                // In our class both vertex and fragment shaders use per-frame constant data
                ShaderTypes::Vertex | ShaderTypes::Fragment);
        }
        else
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }

        // Initialize per-draw-call constant buffer
        if (result = s_constantBuffer_perDrawCall.Initialize())
        {
            // There is only a single per-frame constant buffer that is re-used
            // and so it can be bound at initialization time and never unbound
            s_constantBuffer_perDrawCall.Bind(
                // In our class both vertex and fragment shaders use per-frame constant data
                ShaderTypes::Vertex | ShaderTypes::Fragment);
        }
        else
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }

        if (result = s_samplerState.Initialize())
        {
            // There is only a single sampler state that is re-used
            // and so it can be bound at initialization time and never unbound
            s_samplerState.Bind();
        }
        else
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
    }
    // Initialize the events
    {
        if (!(result = s_whenAllDataHasBeenSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        if (!(result = s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled,
            Concurrency::EventState::Signaled)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
    }
    // Initialize the views
    {
        if (!(result = s_view.Initialize(i_initializationParameters)))
        {
            EAE6320_ASSERT(false);
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
        const auto localResult = s_view.CleanUp();
        if (!localResult)
        {
            EAE6320_ASSERT(false);
            if (result)
            {
                result = localResult;
            }
        }
    }

    if (!s_dataBeingSubmittedByApplicationThread->meshRenderDataList.empty())
    {
        for (auto& meshRenderData : s_dataBeingSubmittedByApplicationThread->meshRenderDataList)
        {
            meshRenderData.constantData_texture->DecrementReferenceCount();
            meshRenderData.constantData_effect->DecrementReferenceCount();
            meshRenderData.constantData_mesh->DecrementReferenceCount();
        }
        s_dataBeingSubmittedByApplicationThread->meshRenderDataList.clear();
    }

    if (!s_dataBeingSubmittedByApplicationThread->spriteRenderDataList.empty())
    {
        for (auto& spriteRenderData : s_dataBeingSubmittedByApplicationThread->spriteRenderDataList)
        {
            spriteRenderData.constantData_texture->DecrementReferenceCount();
            spriteRenderData.constantData_effect->DecrementReferenceCount();
            spriteRenderData.constantData_sprite->DecrementReferenceCount();
        }
        s_dataBeingSubmittedByApplicationThread->spriteRenderDataList.clear();
    }

    if (!s_dataBeingRenderedByRenderThread->meshRenderDataList.empty())
    {
        for (auto& meshRenderData : s_dataBeingRenderedByRenderThread->meshRenderDataList)
        {
            meshRenderData.constantData_texture->DecrementReferenceCount();
            meshRenderData.constantData_effect->DecrementReferenceCount();
            meshRenderData.constantData_mesh->DecrementReferenceCount();
        }
        s_dataBeingRenderedByRenderThread->meshRenderDataList.clear();
    }

    if (!s_dataBeingRenderedByRenderThread->spriteRenderDataList.empty())
    {
        for (auto& spriteRenderData : s_dataBeingRenderedByRenderThread->spriteRenderDataList)
        {
            spriteRenderData.constantData_texture->DecrementReferenceCount();
            spriteRenderData.constantData_effect->DecrementReferenceCount();
            spriteRenderData.constantData_sprite->DecrementReferenceCount();
        }
        s_dataBeingRenderedByRenderThread->spriteRenderDataList.clear();
    }

    {
        const auto localResult = s_constantBuffer_perFrame.CleanUp();
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
        const auto localResult = s_constantBuffer_perDrawCall.CleanUp();
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
        const auto localResult = s_samplerState.CleanUp();
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
        const auto localResult = cShader::s_manager.CleanUp();
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
        const auto localResult = sContext::g_context.CleanUp();
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
