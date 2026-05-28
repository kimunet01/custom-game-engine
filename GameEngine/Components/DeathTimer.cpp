#include "DeathTimer.h"

#include "GameObject.h"
#include "LifeState.h"
#include "Logger.h"

DeathTimer::DeathTimer()
{
    Logger::Info("DeathTimer created");
}

void DeathTimer::Start()
{
    if (pOwner == nullptr) {
        Logger::Warning("DeathTimer started without owner");
        isStarted = true;
        return;
    }

    lifeState = pOwner->GetState<LifeState>();
    if (lifeState == nullptr) {
        Logger::Warning("DeathTimer started without LifeState. owner=%s", pOwner->name.c_str());
    }
    else {
        // LifeState 변경 시 Dead 진입을 감지해 카운트다운을 시작한다.
        // 캡처된 prev/next를 직접 보지 않고 OnLifeChanged에서 현재 상태를 한 번 더 확인한다 — 중복 트리거 방지.
        lifeState->Subscribe([this](LifeStateType, LifeStateType) {
            this->OnLifeChanged();
        });
    }

    isStarted = true;
    Logger::Info("DeathTimer started. owner=%s delay=%.3f", pOwner->name.c_str(), delay);
}

void DeathTimer::Update(float dt)
{
    // 카운트다운이 시작되지 않았으면 아무것도 하지 않는다.
    if (remainingTime < 0.0f) {
        return;
    }
    if (pOwner == nullptr) {
        return;
    }
    // 이미 제거 표시가 된 경우 더 이상 작업 없음.
    if (pOwner->pendingDestroy) {
        return;
    }

    remainingTime -= dt;
    if (remainingTime <= 0.0f) {
        Logger::Info("DeathTimer expired. owner=%s — marking pendingDestroy", pOwner->name.c_str());
        pOwner->pendingDestroy = true;
        // remainingTime을 다시 음수로 두어 추후 Update에서 재진입하지 않도록 한다.
        remainingTime = -1.0f;
    }
}

void DeathTimer::SetDelay(float seconds)
{
    delay = (seconds > 0.0f) ? seconds : 1.5f;
}

void DeathTimer::OnLifeChanged()
{
    if (lifeState == nullptr) {
        return;
    }
    if (!lifeState->IsDead()) {
        return;
    }
    // 이미 카운트다운 중이거나 만료 후이면 재시작하지 않는다.
    if (remainingTime >= 0.0f) {
        return;
    }
    remainingTime = delay;
    Logger::Info("DeathTimer countdown started. owner=%s delay=%.3f",
                 pOwner ? pOwner->name.c_str() : "(null)", delay);
}
