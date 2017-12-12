// Include Files
//==============

#include "sSpringArm.h"

#include <Engine/Math/cMatrix_transformation.h>

// Interface
//==========

void eae6320::Physics::sSpringArm::Update(const float i_secondCountToIntegrate)
{
    camera->position = PredictCameraFuturePosition(i_secondCountToIntegrate);
    camera->orientation = PredictCameraFutureOrientation(i_secondCountToIntegrate);
}

eae6320::Math::sVector eae6320::Physics::sSpringArm::PredictCameraFuturePosition(const float i_secondCountToExtrapolate) const
{
    const Math::cMatrix_transformation transform_targetToWorld = Math::cMatrix_transformation(target->orientation, target->position);

    const Math::sVector targetBackVector = transform_targetToWorld.GetBackDirection();
    const Math::sVector positionBehindTarget = target->position + (targetBackVector * armLength);
    return camera->position + (positionBehindTarget - camera->position) * i_secondCountToExtrapolate * lerpRate;
}

eae6320::Math::cQuaternion eae6320::Physics::sSpringArm::PredictCameraFutureOrientation(const float i_secondCountToExtrapolate) const
{
    Math::cQuaternion extrapolatedOrientation = camera->orientation + (target->orientation - camera->orientation) * i_secondCountToExtrapolate * lerpRate;
    extrapolatedOrientation.Normalize();
    return extrapolatedOrientation;
}
