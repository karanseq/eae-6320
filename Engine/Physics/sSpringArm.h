#ifndef EAE6320_PHYSICS_SSPRINGARM_H
#define EAE6320_PHYSICS_SSPRINGARM_H

// Include Files
//==============

#include "sRigidBodyState.h"

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
            float                           lerpRate = 0.01f;

            // Interface
            //==========

        public:
            void UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate);

        };
    }
}

#endif // EAE6320_PHYSICS_SSPRINGARM_H
