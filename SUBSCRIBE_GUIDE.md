# Subscribe 구조 가이드

> **State가 바뀌면 자동으로 어떤 일이 일어나도록** 미리 등록해두는 방법. 우리 엔진의 게임 로직 흐름은 거의 전부 이 한 가지 패턴 위에 돌아갑니다.

이 문서 한 번 읽으면 우리 코드의 거의 모든 흐름을 따라갈 수 있습니다.

---

## 0. 가장 먼저: 한 줄 요약

> **`Subscribe`는 "이 일이 일어나면 나를 깨워줘"라고 명부에 이름 적어두기.**
> **`Set`은 그 일이 일어나서 "명부에 적힌 모두를 한 번에 깨우기".**
> **State는 누가 명부에 적혔는지만 알고, 그들이 뭐 할지는 모름.**

---

## 1. 왜 이게 필요한가 — Subscribe가 없는 세상

플레이어가 데미지를 입었을 때 일어나야 하는 일들:

1. HP 숫자 감소
2. 빨갛게 깜빡임 시각 효과
3. HP가 0이면 사망 처리
4. 사망 시 "dead" 애니메이션 클립 전환
5. 사망 시 이동 입력 차단
6. 사망 1.5초 후 게임에서 제거

**Subscribe 없는 세상에서는**, 데미지를 일으키는 코드(예: `PlayerControl::Update`)가 이 6가지를 다 알아야 합니다:

```cpp
// 가상 코드 — 이런 세상이 되면 안 됨
void PlayerControl::Update(float dt) {
    if (pOwner->isCollided) {
        healthState->current -= 1;
        hitReact->Trigger();                      // 깜빡임을 직접 호출
        if (healthState->current <= 0) {
            lifeState->state = Dead;              // 사망 처리 직접
            spriteAnimator->SwitchToClip("dead"); // 클립 전환 직접
            isMovementLocked = true;              // 자기 잠금 직접
            deathTimer->Start(1.5f);              // 타이머 직접 시작
        }
    }
}
```

`PlayerControl`이 `HitReactionController`, `SpriteAnimator`, `DeathTimer`를 **다 알아야** 합니다. 컴포넌트가 늘면 늘수록 이 코드도 늘어남. 새로 "사망 시 사운드 재생" 컴포넌트가 추가되면 또 한 줄 추가. 컴포넌트 수만 개에 함수 수만 개 — 추적 불가능.

---

## 2. Subscribe가 있는 세상 — 우리 코드

같은 시나리오를 우리 코드로 처리하면 `PlayerControl::Update`는 이것만 합니다:

```cpp
if (pOwner->isCollided) {
    HealthState* hs = pOwner->GetState<HealthState>();
    hs->SetCurrent(hs->GetCurrent() - 1);   // ← HP를 1 깎았다. 끝.
}
```

**한 줄 호출이 위 6가지 일을 줄줄이 일으킵니다.** PlayerControl은 다른 컴포넌트의 존재를 모릅니다. 어떻게? 각 컴포넌트가 **자기 Start에서 미리 "이런 변화에 반응하겠다"고 등록(Subscribe)** 해뒀기 때문.

---

## 3. 가장 핵심: 함수를 저장한다는 개념

C++에서 함수는 보통 **즉시 실행**됩니다:

```cpp
doSomething();        // 지금 즉시 실행
```

근데 함수를 **나중에 실행할 수 있게 저장**해두는 것도 가능합니다:

```cpp
std::vector<std::function<void()>> 보관함;
보관함.push_back([]() { doSomething(); });   // 저장만 함. 아직 실행 X

// ... 시간이 흐른 후 ...
for (auto& f : 보관함) f();                  // 이 시점에 비로소 실행됨
```

**`Subscribe`는 위의 `push_back`이고, `Set`은 위의 `for` 루프입니다.** 이게 본질의 전부.

---

## 4. 우리 State의 실제 모습

```cpp
// FrameWork/State.h (요약)
template<typename TEnum>
class ObservableState : public State {
public:
    using Callback = std::function<void(TEnum prev, TEnum next)>;

    TEnum Get() const { return current; }

    void Set(TEnum next) {
        if (current == next) return;       // 같은 값이면 무시
        TEnum prev = current;
        current = next;
        for (auto& cb : subscribers) {     // 보관함을 순회하며
            cb(prev, next);                //  하나씩 실행
        }
    }

    void Subscribe(Callback cb) {
        subscribers.push_back(std::move(cb));   // 그냥 벡터에 추가
    }

protected:
    TEnum current{};
    std::vector<Callback> subscribers;     // 함수 보관함
};
```

`subscribers`는 **함수가 줄지어 들어가는 `std::vector`** 일 뿐입니다. 마법은 없습니다.

---

## 5. 실제 한 흐름 따라가기

플레이어가 데미지를 입고 죽는 순간을 처음부터 끝까지 따라갑니다.

### Step A. 게임 시작 — 명부 작성 (Subscribe)

`main.cpp`에서 GameObject를 만들면, 각 컴포넌트의 `Start()`가 차례로 호출됩니다.

#### Player의 `HealthController::Start()`

```cpp
if (HealthState* hs = pOwner->GetState<HealthState>()) {
    hs->Subscribe([this](int p, int n) {                    // ← 람다를 만들어서
        StateCallbacks::OnHealthAutoDeath(this, p, n);
    });                                                      // ← Subscribe로 hs.subscribers에 push
}
```

이 순간:

```
Player의 HealthState.subscribers:
  ┌─────────────────────────────────────┐
  │ [0] λ (this=HealthController 인스턴스) │   ← 방금 추가됨
  └─────────────────────────────────────┘
```

**아직 `OnHealthAutoDeath`는 한 번도 실행되지 않았습니다.** 저장만 됨.

#### Player의 `HitReactionController::Start()`

```cpp
if (HealthState* hs = pOwner->GetState<HealthState>()) {
    hs->Subscribe([this](int p, int n) {
        StateCallbacks::OnHitReaction(this, p, n);
    });
}
```

```
Player의 HealthState.subscribers:
  ┌─────────────────────────────────────────┐
  │ [0] λ (this=HealthController 인스턴스)     │
  │ [1] λ (this=HitReactionController 인스턴스) │   ← 추가됨
  └─────────────────────────────────────────┘
```

다른 State들에도 비슷하게 람다가 쌓입니다:

```
Player의 LifeState.subscribers:
  [0] λ (SpriteAnimator)        → 클립 전환용
  [1] λ (PlayerControl)         → 이동 잠금용
  [2] λ (DeathTimer)            → 카운트다운 시작용

Player의 MovementState.subscribers:
  [0] λ (SpriteAnimator)        → walk/stand 클립 전환

Player의 AttackState.subscribers:
  [0] λ (SpriteAnimator)        → sword_attack 클립
  [1] λ (PlayerControl)         → 공격 중 잠금
```

이 시점에서는 **단 한 줄의 게임 로직도 실행되지 않음.** 명부에 이름만 적힘.

---

### Step B. 게임 중 — Set 호출 → 명부 줄줄이 깨우기

플레이어가 적과 부딪힘. `PlayerControl::Update`가 자기 `isCollided`를 보고 HP를 깎으려 합니다:

```cpp
// PlayerControl::Update 안 (HP가 3 → 2가 되는 순간)
HealthState* hs = pOwner->GetState<HealthState>();
hs->SetCurrent(2);   // 핵심 한 줄
```

`SetCurrent(2)` 안에서 무슨 일이 일어나는가:

```cpp
// HealthState (= ObservableState<int>)::Set
void Set(int next /* =2 */) {
    if (current == next) return;        // 3 != 2 → 통과
    int prev = current;                  // prev = 3
    current = next;                      // current = 2
    for (auto& cb : subscribers) {       // ← 명부 순회!
        cb(3, 2);                         // 람다 호출
    }
}
```

이 `for` 루프가 **명부에 적힌 2개 람다를 차례로 호출**합니다:

```
반복 1: subscribers[0] (HealthController 람다) 실행
  → 람다 본체: StateCallbacks::OnHealthAutoDeath(HealthController*, 3, 2)
       → 조건 prev(3) > 0 && next(2) <= 0 ? → 거짓
       → 아무것도 안 함 (아직 죽지 않음)

반복 2: subscribers[1] (HitReactionController 람다) 실행
  → 람다 본체: StateCallbacks::OnHitReaction(HitReactionController*, 3, 2)
       → 조건 next(2) >= prev(3) ? → 거짓이므로 통과
       → HitReact->remainingTime = 0.25  ← 빨간 깜빡임 시작!
```

`hs->SetCurrent(2)` **한 줄**의 결과로 두 가지 일이 자동 발화됐습니다. PlayerControl은 자기가 무슨 일이 일어났는지 모릅니다.

---

### Step C. 죽음의 순간 — 콜백 체인

다시 데미지로 HP가 1 → 0이 되면:

```cpp
hs->SetCurrent(0);
```

```
HealthState.Set(0):
  for문 반복:
    [0] OnHealthAutoDeath(_, 1, 0)
         → prev(1) > 0 && next(0) <= 0 → 참!
         → life->SetDead()  ◀━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
    [1] OnHitReaction(_, 1, 0)                                  ┃
         → HitReact->remainingTime = 0.25 (또 깜빡임 시작)        ┃
                                                                 ▼
            LifeState.Set(Dead):
              for문 반복:
                [0] OnAnimLife(SpriteAnimator, Alive, Dead)
                     → SpriteAnimator가 "dead" 클립으로 전환
                [1] OnControlLife(PlayerControl, Alive, Dead)
                     → PlayerControl.isMovementLocked = true
                [2] OnLifeDeathTimer(DeathTimer, Alive, Dead)
                     → DeathTimer->remainingTime = 1.5
                       (1.5초 후 DeathTimer.Update가 pendingDestroy=true 표시)
```

**한 번의 `SetCurrent(0)` 호출**이 줄줄이 5가지 일을 깨웠습니다. 누가 누구를 부르거나 하지 않음 — **각 State의 subscribers에 미리 등록된 람다들이 자동 발화**된 것뿐.

---

## 6. 그림으로 보기

```
[Step A — 게임 시작]
  컴포넌트들이 자기 관심 State에 람다를 push_back
  (실행 X, 저장만)

      각 State는 이런 모습이 됨
      ┌─────────────────────────┐
      │  State.current = ...     │
      │  State.subscribers:      │
      │    [0] λ                 │
      │    [1] λ                 │
      │    [2] λ                 │
      └─────────────────────────┘


[Step B — 게임 중 어디서든]
  누군가 state.Set(newValue) 호출

      ┌─────────────────────────┐
      │  current 갱신             │
      │  for cb in subscribers:  │
      │     cb(prev, next)        │
      │       ├─ 람다 1 실행       │
      │       ├─ 람다 2 실행       │
      │       └─ 람다 3 실행       │
      └─────────────────────────┘
              │
              ▼
       각 람다가 StateCallbacks의
       자유 함수를 호출하여 자기 책임 처리
```

---

## 7. 우리 코드의 규칙 (지키면 일관성 유지)

### 규칙 1. 컴포넌트는 lifecycle(Start/Input/Update/Render) + 데이터만

```cpp
class HealthController : public Component {
public:
    void Start() override;
    void Update(float dt) override;
    // public 데이터
    float invincibilityRemaining = 0.0f;
    float invincibilityDuration = 0.4f;
};
```

`OnHealthChanged`, `StartReaction` 같은 사적 메서드를 컴포넌트 안에 두지 않습니다. 그런 로직은 모두 `StateCallbacks`로.

### 규칙 2. Start의 본체 = "Subscribe 한 줄들"

```cpp
void HealthController::Start() {
    if (HealthState* hs = pOwner->GetState<HealthState>()) {
        hs->Subscribe([this](int p, int n) {
            StateCallbacks::OnHealthAutoDeath(this, p, n);
        });
    }
}
```

람다 본체는 한 줄 — `StateCallbacks::On*(this, p, n)` 호출만. 실제 로직은 `StateCallbacks` 안에.

### 규칙 3. State 포인터 캐싱 금지

GameObject가 이미 `pOwner->GetState<T>()`로 가져갈 수 있는 것을 컴포넌트 멤버로 또 보관하지 않습니다. 필요할 때 즉시 조회.

### 규칙 4. 콜백 = "State 변화에 대한 반응"만

호출자가 1곳뿐인 동작(예: 검 공격 발동)은 그 호출자에 인라인. `StateCallbacks::TriggerSword` 같은 의도적 액션 함수를 함부로 만들지 않습니다. 단, **여러 곳에서 같은 효과를 일으켜야 하는** 함수만 자유 함수로.

---

## 8. 새 기능 추가 시 따라하는 절차

예를 들어 "플레이어 사망 시 효과음 재생" 기능을 추가한다고 합시다.

1. 새 컴포넌트 `DeathSoundPlayer` 만들기 — 데이터(사운드 파일 경로 등) + Start + Update.
2. `DeathSoundPlayer::Start`에 한 줄:
   ```cpp
   if (LifeState* life = pOwner->GetState<LifeState>()) {
       life->Subscribe([this](LifeStateType p, LifeStateType n) {
           StateCallbacks::OnLifeDeathSound(this, p, n);
       });
   }
   ```
3. `StateCallbacks`에 `OnLifeDeathSound(DeathSoundPlayer*, LifeStateType, LifeStateType)` 추가 — 사망 진입 엣지에서 사운드 재생.
4. `main.cpp`에서 Player에 `AddComponent(new DeathSoundPlayer())`.

**기존 코드 한 줄도 안 바뀝니다.** PlayerControl도, HealthController도, LifeState도 모름. 그냥 자기 명부에 청취자 한 명이 더 들어갔을 뿐.

---

## 9. 흔히 헷갈리는 점

### Q. `Set`이 어떤 콜백을 호출할지 어떻게 매칭하나?
**A. 매칭 없음.** `subscribers` 벡터의 **모든** 람다가 다 호출됩니다. "어떤 변화면 누구 호출"같은 라우팅은 없음. 각 람다가 받은 `(prev, next)`를 보고 자기가 알아서 분기.

### Q. 람다 안의 `[this]`는 뭐?
**A. 캡처.** 람다가 자기를 등록한 컴포넌트 인스턴스의 포인터(`this`)를 같이 들고 다닙니다. 나중에 람다가 실행될 때 "어느 인스턴스에 대해 처리해야 하는지" 알 수 있습니다. `std::function`이 이걸 담을 수 있어서 함수 포인터 대신 사용합니다.

### Q. Start 시점에는 콜백 발화 안 되나?
**A. 안 됨.** 콜백은 "**변화**" 시에만 발화. 등록 직후 현재 상태에 맞춰 초기 동기화가 필요하면 명시적으로 한 번 호출. SpriteAnimator가 Start 끝에 `StateCallbacks::ReevaluateAnimClip(this)`를 한 번 부르는 게 그 예.

### Q. 동일값으로 Set하면?
**A. 무시.** `if (current == next) return;` 가드. 중복 통보 안 일어남.

### Q. 콜백 안에서 다른 State.Set을 호출해도 되나?
**A. 됨.** 자연스러운 사용. 예: `OnHealthAutoDeath`가 `life->SetDead()`를 호출 → LifeState 콜백 체인이 또 발화. 다만 자기 자신의 Set을 콜백 안에서 재호출하는 무한 루프는 책임지지 않음 (개발자가 조심).

### Q. 콜백 등록을 해제하는 방법은?
**A. 없음(의도).** State와 컴포넌트는 GameObject가 같이 소유하므로 수명이 함께 끝남. 람다가 캡처한 `this`는 owner가 죽으면 같이 죽음. 한 GameObject의 람다가 다른 GameObject를 참조하지 않는 한 안전.

---

## 10. 우리 코드의 모든 콜백 한눈에

```cpp
// Callbacks/StateCallbacks.h
namespace StateCallbacks
{
    // 애니메이션 클립 재평가 (3개 State 어느 것이 바뀌어도 호출)
    void OnAnimMovement(SpriteAnimator*, MovementStateType, MovementStateType);
    void OnAnimAttack  (SpriteAnimator*, AttackStateType,   AttackStateType);
    void OnAnimLife    (SpriteAnimator*, LifeStateType,     LifeStateType);
    void ReevaluateAnimClip(SpriteAnimator*);  // Start 초기 동기화용

    // PlayerControl 입력 잠금
    void OnControlLife  (PlayerControl*, LifeStateType,   LifeStateType);
    void OnControlAttack(PlayerControl*, AttackStateType, AttackStateType);

    // 환경(셰이더) 토글
    void OnEnvTerrain(EnvironmentRenderer*, TerrainStateType, TerrainStateType);

    // 자동 사망 처리
    void OnHealthAutoDeath(HealthController*, int prev, int next);

    // 피격 시각 반응
    void OnHitReaction(HitReactionController*, int prev, int next);

    // 사망 후 소멸 카운트다운
    void OnLifeDeathTimer(DeathTimer*, LifeStateType, LifeStateType);
}
```

이 한 헤더만 보면 **이 게임에서 State 변화가 일으키는 모든 반응이 무엇인지** 다 보입니다.

---

## 11. 마지막 정리

| 개념 | 의미 |
|---|---|
| **State** | 값 + 함수 보관함 (`subscribers`) |
| **Subscribe** | 보관함에 함수(람다) push_back |
| **Set** | 값 갱신 + 보관함 모든 함수 호출 |
| **StateCallbacks** | 람다 안에서 호출되는 자유 함수들. 진짜 반응 로직은 여기 |
| **Component** | lifecycle + 데이터만. 사적 메서드 금지 |
| **`[this]` 캡처** | 람다가 등록 컴포넌트 인스턴스를 들고 다님 |

**이 6개 개념만 알면 우리 코드의 모든 게임 로직 흐름을 따라갈 수 있습니다.** 새 기능은 새 콜백 + Subscribe 한 줄로 추가. 기존 코드는 영영 안 바뀜.
