#pragma once

/*
 * VelocityController.h
 * GameObject의 velocity를 실제 position 변화로 적용하는 이동 컴포넌트다.
 *
 * PlayerControl이나 초기 spawn 코드가 velocity를 설정하면, 이 컴포넌트가 매 Update마다
 * deltaTime을 곱해 위치를 갱신한다. maxDelta는 한 프레임에 너무 큰 이동이 발생하는 것을
 * 제한해 충돌 처리와 화면 경계 처리를 안정적으로 만든다.
 */

#include "Component.h"

class VelocityController : public Component {
public:
    explicit VelocityController(float maxDelta = 0.03f);

    // pOwner->velocity를 읽어 pOwner->position에 반영한다.
    void Update(float dt) override;

private:
    // 프레임 지연이 커졌을 때 이동량이 과도하게 튀는 것을 막는 최대 이동량.
    float maxDelta;
};
