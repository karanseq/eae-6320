/*
TODO
*/

#ifndef EAE6320_GRAPHICS_CCAMERA_H
#define EAE6320_GRAPHICS_CCAMERA_H

// Include Files
//==============

#include <Engine/Math/Constants.h>
#include <Engine/Math/Functions.h>
#include <Engine/Physics/sRigidBodyState.h>

// Struct Declaration
//===================

namespace eae6320
{
    namespace Graphics
    {
        struct sCamera
        {
            Physics::sRigidBodyState            m_rigidBodyState;
            float                               m_verticalFieldOfView_inRadians = Math::Pi * 0.25f;
            float                               m_aspectRatio = 1.0f;
            float                               m_z_nearPlane = 0.1f;
            float                               m_z_farPlane = 1000.0f;

        }; // struct sCamera

    } // namespace Graphics

} // namespace eae6320

#endif // EAE6320_GRAPHICS_CCAMERA_H
