/*
    This class is your specific game
*/

#ifndef EAE6320_CEXAMPLEGAME_H
#define EAE6320_CEXAMPLEGAME_H

// Include Files
//==============

#include <Engine/Application/cbApplication.h>
#include <Engine/Graphics/cTexture.h>
#include <Engine/Graphics/sColor.h>
#include <Engine/Results/Results.h>
#include <vector>

#if defined( EAE6320_PLATFORM_WINDOWS )
    #include "Resource Files/Resource.h"
#endif

// Forward Declarations
//=====================

namespace eae6320
{
    namespace Graphics
    {
        class cEffect;
        class cMesh;
        class cSprite;
    }

    namespace Math
    {
        struct sVector2d;
    }

    namespace UserInterface
    {
        class cWidget;
    }
}

// Class Declaration
//==================

namespace eae6320
{
    class cExampleGame : public Application::cbApplication
    {
        // Inherited Implementation
        //=========================

    private:

        // Configuration
        //--------------

#if defined( EAE6320_PLATFORM_WINDOWS )
        // The main window's name will be displayed as its caption (the text that is displayed in the title bar).
        // You can make it anything that you want, but please keep the platform name and debug configuration at the end
        // so that it's easy to tell at a glance what kind of build is running.
        virtual const char* GetMainWindowName() const override
        {
            return "Karan's EAE6320 Example Game"
                " -- "
#if defined( EAE6320_PLATFORM_D3D )
                "Direct3D"
#elif defined( EAE6320_PLATFORM_GL )
                "OpenGL"
#endif
#ifdef _DEBUG
                " -- Debug"
#endif
                ;
        }
        // Window classes are almost always identified by name;
        // there is a unique "ATOM" associated with them,
        // but in practice Windows expects to use the class name as an identifier.
        // If you don't change the name below
        // your program could conceivably have problems if it were run at the same time on the same computer
        // as one of your classmate's.
        // You don't need to worry about this for our class,
        // but if you ever ship a real project using this code as a base you should set this to something unique
        // (a generated GUID would be fine since this string is never seen)
        virtual const char* GetMainWindowClassName() const override { return "Karan's EAE6320 Example Main Window Class"; }
        // The following three icons are provided:
        //	* IDI_EAEGAMEPAD
        //	* IDI_EAEALIEN
        //	* IDI_VSDEFAULT_LARGE / IDI_VSDEFAULT_SMALL
        // If you want to try creating your own a convenient website that will help is: http://icoconvert.com/
        virtual const WORD* GetLargeIconId() const override { static constexpr WORD iconId_large = IDI_EAEALIEN; return &iconId_large; }
        virtual const WORD* GetSmallIconId() const override { static constexpr WORD iconId_small = IDI_EAEALIEN; return &iconId_small; }
#endif

        // Run
        //----

        virtual void UpdateBasedOnInput() override;
        virtual void UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate) override;

        virtual void SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate) override;

        void UpdateSpriteRenderData(const float i_elapsedSecondCount_sinceLastUpdate);

        // Initialization / Clean Up
        //--------------------------

        virtual cResult Initialize() override;
        virtual cResult CleanUp() override;

        cResult InitializeEffects();
        cResult InitializeTextures();
        cResult InitializeMeshes();
        cResult InitializeSprites();
        void InitializeSpriteRenderDataList();
        cResult InitializeWidgets();

        void GetRandomOriginForSprite(eae6320::Math::sVector2d& o_origin) const;
        void GetRandomExtentsForSprite(eae6320::Math::sVector2d& o_extents) const;

        // Data
        //=====

    private:

        struct sSpriteRenderData
        {
            uint8_t m_firstFrameIndex = 0;
            uint8_t m_currentFrameIndex = 0;
            float m_frameRate = 0.0f;
            float m_waitUntilNextFrame = 0.0f;
            eae6320::Graphics::cEffect* m_effect = nullptr;
            eae6320::Graphics::cSprite* m_sprite = nullptr;
        };

        eae6320::Graphics::sColor m_backgroundColor = eae6320::Graphics::sColor::ORANGE;
        std::vector<eae6320::Graphics::cEffect*> m_effectList;
        std::vector<eae6320::Graphics::cTexture::Handle> m_textureList;
        std::vector<eae6320::Graphics::cSprite*> m_spriteList;
        std::vector<eae6320::Graphics::cMesh*> m_meshList;
        std::vector<sSpriteRenderData> m_spriteRenderDataList;
        std::vector<eae6320::UserInterface::cWidget*> m_widgetList;


        bool m_swapSpritesBasedOnInput = false;

        static constexpr uint8_t s_numTextureFolders = 3;
        static constexpr uint8_t s_numFrames = 6;
        static const std::string s_meshVertexShaderFilePath;
        static const std::string s_meshFragmentShaderFilePath;
        static const std::string s_spriteVertexShaderFilePath;
        static const std::string s_spriteFragmentShaderFilePath;
        static const std::string s_animatedSpriteFragmentShaderFilePath;
        static const std::string s_textureFolderList[s_numTextureFolders];

    };
}

// Result Definitions
//===================

namespace eae6320
{
    namespace Results
    {
        namespace Application
        {
            // You can add specific results for your game here:
            //	* The System should always be Application
            //	* The __LINE__ macro is used to make sure that every result has a unique ID.
            //		That means, however, that all results _must_ be defined in this single file
            //		or else you could have two different ones with equal IDs.
            //	* Note that you can define multiple Success codes.
            //		This can be used if the caller may want to know more about how a function succeeded.
            constexpr cResult ExampleResult( IsFailure, System::Application, __LINE__, Severity::Default );
        }
    }
}

#endif	// EAE6320_CEXAMPLEGAME_H
