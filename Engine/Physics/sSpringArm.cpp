// Include Files
//==============

#include "sSpringArm.h"

#include <Engine/Math/sVector.h>

// Interface
//==========

void eae6320::Physics::sSpringArm::UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
    const Math::sVector cameraToTarget = target->position - camera->position;
    const float targetDistanceFromCamera = cameraToTarget.GetLengthSquared();
    if (targetDistanceFromCamera > armLength * armLength)
    {
        const Math::sVector cameraToTargetNormalized = cameraToTarget.GetNormalized();
        camera->velocity = target->velocity.GetLengthSquared() > 0.0f ? target->velocity : cameraToTargetNormalized * 2.0f;
    }
}
