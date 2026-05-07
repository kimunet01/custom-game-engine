#pragma once

#include "Component.h"

class VelocityController : public Component {
public:
    explicit VelocityController(float maxDelta = 0.03f);

    void Update(float dt) override;

private:
    float maxDelta;
};
