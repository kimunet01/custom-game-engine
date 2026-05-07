#include "VelocityController.h"

#include "GameObject.h"
#include "Utils.h"

VelocityController::VelocityController(float maxDelta)
    : maxDelta(maxDelta)
{
}

void VelocityController::Update(float dt)
{
    if (pOwner == nullptr) {
        return;
    }

    pOwner->position.x += ClampFloat(pOwner->velocity.x * dt, -maxDelta, maxDelta);
    pOwner->position.y += ClampFloat(pOwner->velocity.y * dt, -maxDelta, maxDelta);
    pOwner->position.z += ClampFloat(pOwner->velocity.z * dt, -maxDelta, maxDelta);
}
