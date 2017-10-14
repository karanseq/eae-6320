/*
    TODO
*/

#ifndef EAE6320_USERINTERFACE_CWIDGET_H
#define EAE6320_USERINTERFACE_CWIDGET_H

// Include Files
//==============

#include <Engine/Assets/ReferenceCountedAssets.h>
#include <Engine/Graphics/cTexture.h>
#include <Engine/Math/sVector2d.h>
#include <Engine/Results/Results.h>

// Forward Declarations
//=====================

namespace eae6320
{
    namespace Graphics
    {
        class cEffect;
        class cSprite;
    }
}

// Class Declaration
//==================

namespace eae6320
{
    namespace UserInterface
    {
        class cWidget
        {
            // Interface
            //==========

        public:

            // Render
            //-------

            void SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate);

            // Initialization / Clean Up
            //--------------------------

            struct sInitializationParameters
            {
                // The name of the vertex shader to initialize the underlying effect with.
                const char*                             vertexShaderName = nullptr;

                // The name of the fragment shader to initialize the underlying effect with.
                const char*                             fragmentShaderName = nullptr;
                
                // The name of the texture to use with this widget.
                const char*                             textureName = nullptr;
                
                // The position in screen-coordinates.
                // Bottom-Left                          => -1.0f, -1.0f
                // Top-Right                            => 1.0f, 1.0f
                // Center                               => 0.5f, 0.5f
                Math::sVector2d                         position;
                
                // The anchor around which the position is calculated.
                // Bottom-Left                          => 0.0f, 0.0f
                // Top-Right                            => 1.0f, 1.0f
                // Center                               => 0.5f, 0.5f
                Math::sVector2d                         anchor = { 0.5f, 0.5f };
                
                // The amount this widget is scaled relative to its texture size.
                Math::sVector2d                         scale = { 1.0f, 1.0f };
            };

            static cResult Create(cWidget*& o_widget, const sInitializationParameters& i_params);

        public:

            // Reference Counting
            //-------------------

            EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

            EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cWidget);

        private:

            // Initialization / Clean Up
            //--------------------------

            cResult Initialize(const sInitializationParameters& i_params);
            cResult CleanUp();

            cWidget() = default;
            ~cWidget();

            // Implementation
            //===============

            void GetSpriteOriginAndExtents(Math::sVector2d& o_origin, Math::sVector2d& o_extents, const sInitializationParameters& i_params) const;
            void GetCurrentResolution(Math::sVector2d& o_resolution) const;

            // Data
            //=====

        private:
            Graphics::cEffect*                         m_effect = nullptr;
            Graphics::cTexture::Handle                 m_texture;
            Graphics::cSprite*                         m_sprite = nullptr;

            EAE6320_ASSETS_DECLAREREFERENCECOUNT();

        }; // class cWidget

    } // namespace UserInterface

} // namespace eae6320

#endif // EAE6320_USERINTERFACE_CWIDGET_H
