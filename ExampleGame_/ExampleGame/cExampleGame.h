/*
    This class is your specific game
*/

#ifndef EAE6320_CEXAMPLEGAME_H
#define EAE6320_CEXAMPLEGAME_H

// Include Files
//==============

#include <Engine/Application/cbApplication.h>
#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/cTexture.h>
#include <Engine/Graphics/sCamera.h>
#include <Engine/Graphics/sColor.h>
#include <Engine/Physics/sSpringArm.h>
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

    class cGameObject;
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
        virtual void UpdateSimulationBasedOnInput() override;
        virtual void UpdateSimulationBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate) override;

        virtual void SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate) override;

        void UpdateGameObjects(const float i_elapsedSecondCount_sinceLastUpdate);
        void UpdateCoins(const float i_elapsedSecondCount_sinceLastUpdate);

        // Initialization / Clean Up
        //--------------------------

        virtual cResult Initialize() override;
        virtual cResult CleanUp() override;

        cResult InitializeCamera();
        cResult InitializeGameObjects();
        cResult InitializeRings();
        cResult InitializeSkyBox();

        void GetRandomOriginForSprite(Math::sVector2d& o_origin) const;
        void GetRandomExtentsForSprite(Math::sVector2d& o_extents) const;

        // Data
        //=====

    public:

        static const std::string                            s_meshVertexShaderFilePath;
        static const std::string                            s_meshFragmentShaderFilePath;

    private:

        Graphics::sCamera                                   m_camera;
        Graphics::sColor                                    m_backgroundColor = Graphics::sColor(0.05882f, 0.05882f, 0.26275f, 1.0f);
        std::vector<cGameObject*>                           m_gameObjectList;

        Physics::sSpringArm                                 m_springArm;

        bool                                                m_skyBoxEnabled = true;
        Graphics::cMesh::Handle                             m_skyBoxMesh;
        Graphics::cEffect*                                  m_skyBoxEffect = nullptr;
        Graphics::cTexture::Handle                          m_skyBoxTexture;

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
