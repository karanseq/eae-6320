#ifndef EAE6320_PHYSICS_SSPRINGARM_H
#define EAE6320_PHYSICS_SSPRINGARM_H

// Include Files
//==============

#include "sRigidBodyState.h"

#include <Engine/Math/cQuaternion.h>
#include <Engine/Math/sVector.h>

// Class Declaration
//==================

namespace eae6320
{
    namespace Physics
    {
        struct sSpringArm
        {
            // Data
            //=====

            const sRigidBodyState*          target = nullptr;
            sRigidBodyState*                camera = nullptr;
            float                           armLength = 0.0f;
            float                           lerpRate = 1.0f;//0.01f;

            // Interface
            //==========

        public:
            void Update(const float i_secondCountToIntegrate);
            Math::sVector PredictCameraFuturePosition(const float i_secondCountToExtrapolate) const;
            Math::cQuaternion PredictCameraFutureOrientation(const float i_secondCountToExtrapolate) const;

        };
    }
}

#endif // EAE6320_PHYSICS_SSPRINGARM_H
