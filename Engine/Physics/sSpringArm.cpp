// Include Files
//==============

#include "sSpringArm.h"

#include <Engine/Math/cMatrix_transformation.h>
#include <Engine/Math/sVector.h>

// Interface
//==========

void eae6320::Physics::sSpringArm::UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
    const Math::cMatrix_transformation transform_targetToWorld = Math::cMatrix_transformation(target->orientation, target->position);

    // Interpolate camera position to target position
    const Math::sVector targetBackVector = transform_targetToWorld.GetBackDirection();
    const Math::sVector positionBehindTarget = target->position + (targetBackVector * armLength);
    camera->position += (positionBehindTarget - camera->position) * i_elapsedSecondCount_sinceLastUpdate * 0.25f;

    // Interpolate camera orientation to target orientation
    camera->orientation += (target->orientation - camera->orientation) * i_elapsedSecondCount_sinceLastUpdate * 0.05f;
}
