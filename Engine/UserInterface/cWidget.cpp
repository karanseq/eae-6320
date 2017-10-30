// Include Files
//==============

#include "cWidget.h"

#include <Engine/Application/cbApplication.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Graphics/cSprite.h>
#include <Engine/Graphics/cTexture.h>
#include <Engine/Graphics/Graphics.h>
#include <Engine/Logging/Logging.h>
#include <Engine/UserSettings/UserSettings.h>

#include <new>

// Interface
//==========

// Render
//-------

void eae6320::UserInterface::cWidget::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
    eae6320::Graphics::cTexture* texture = eae6320::Graphics::cTexture::s_manager.Get(m_texture);
    eae6320::Graphics::SubmitSpriteToBeRendered(m_sprite, m_effect, texture);
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::UserInterface::cWidget::Create(cWidget*& o_widget, const sInitializationParameters& i_params)
{
    auto result = eae6320::Results::Success;

    cWidget* newWidget = nullptr;

    // Allocate a new widget
    {
        newWidget = new (std::nothrow) cWidget();
        if (newWidget == nullptr)
        {
            result = eae6320::Results::OutOfMemory;
            EAE6320_ASSERTF(false, "Failed to allocate memory for the new widget!");
            Logging::OutputError("Failed to allocate memory for the new widget!");
            goto OnExit;
        }
    }

    // Initialize the new widget
    if (!(result = newWidget->Initialize(i_params)))
    {
        EAE6320_ASSERTF(false, "Failed to initialize the new widget!");
        goto OnExit;
    }

OnExit:

    if (result)
    {
        EAE6320_ASSERT(newWidget);
        o_widget = newWidget;
    }
    else
    {
        if (newWidget)
        {
            newWidget->DecrementReferenceCount();
            newWidget = nullptr;
        }
        o_widget = nullptr;
    }

    return result;
}

eae6320::cResult eae6320::UserInterface::cWidget::Initialize(const sInitializationParameters& i_params)
{
    auto result = eae6320::Results::Success;

    // create the effect
    if (i_params.vertexShaderName && i_params.fragmentShaderName)
    {
        eae6320::Graphics::cEffect* effect = nullptr;
        if (!(result = eae6320::Graphics::cEffect::Create(effect, i_params.vertexShaderName, i_params.fragmentShaderName, Graphics::RenderStates::AlphaTransparency)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_effect = effect;
        }
    }
    else
    {
        result = eae6320::Results::Failure;
        EAE6320_ASSERT(false);
        goto OnExit;
    }

    // load the texture
    if (i_params.textureName)
    {
        eae6320::Graphics::cTexture::Handle textureHandle;
        if (!(result = eae6320::Graphics::cTexture::s_manager.Load(i_params.textureName, textureHandle)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_texture = textureHandle;
        }
    }
    else
    {
        result = eae6320::Results::Failure;
        EAE6320_ASSERT(false);
        goto OnExit;
    }

    // create the sprite
    {
        eae6320::Graphics::cSprite* sprite = nullptr;

        eae6320::Math::sVector2d spriteOrigin;
        eae6320::Math::sVector2d spriteExtents;
        GetSpriteOriginAndExtents(spriteOrigin, spriteExtents, i_params);

        if (!(result = eae6320::Graphics::cSprite::Create(sprite, spriteOrigin, spriteExtents)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_sprite = sprite;
        }
    }

OnExit:

    return result;
}

eae6320::cResult eae6320::UserInterface::cWidget::CleanUp()
{
    auto result = eae6320::Results::Success;

    // Release the effect
    {
        m_effect->DecrementReferenceCount();
        m_effect = nullptr;
    }

    // Release the texture
    {
        const auto localResult = eae6320::Graphics::cTexture::s_manager.Release(m_texture);
        if (!localResult)
        {
            EAE6320_ASSERT(false);
            if (result)
            {
                result = localResult;
            }
        }
    }

    // Release the sprite
    {
        m_sprite->DecrementReferenceCount();
        m_sprite = nullptr;
    }

    return result;
}

eae6320::UserInterface::cWidget::~cWidget()
{
    CleanUp();
}

// Implementation
//===============

void eae6320::UserInterface::cWidget::GetSpriteOriginAndExtents(Math::sVector2d& o_origin, Math::sVector2d& o_extents, const sInitializationParameters& i_params) const
{
    eae6320::Math::sVector2d resolution;
    GetCurrentResolution(resolution);

    eae6320::Graphics::cTexture* texture = eae6320::Graphics::cTexture::s_manager.Get(m_texture);

    o_extents.x = 0.5f * texture->GetWidth() / resolution.x * i_params.scale.x;
    o_extents.y = 0.5f * texture->GetHeight() / resolution.y * i_params.scale.y;

    o_origin.x = i_params.position.x + (0.5f - i_params.anchor.x) * o_extents.x * 2.0f;
    o_origin.y = i_params.position.y + (0.5f - i_params.anchor.y) * o_extents.y * 2.0f;
}

void eae6320::UserInterface::cWidget::GetCurrentResolution(Math::sVector2d& o_resolution) const
{
    uint16_t desiredResolutionWidth = eae6320::Application::cbApplication::s_defaultInitialResolution_width;
    uint16_t desiredResolutionHeight = eae6320::Application::cbApplication::s_defaultInitialResolution_height;
    eae6320::UserSettings::GetDesiredInitialResolutionWidth(desiredResolutionWidth);
    eae6320::UserSettings::GetDesiredInitialResolutionHeight(desiredResolutionHeight);
    o_resolution.x = float(desiredResolutionWidth);
    o_resolution.y = float(desiredResolutionHeight);
}
