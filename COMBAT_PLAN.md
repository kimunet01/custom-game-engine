# 전투/보스전 구현 계획 (Harness)

## Goal
1. 플레이어 공격 시 캐릭터 **전방의 적**에게 피해.
2. 피해 입은 적: **빨갛게 깜빡 + 살짝 흔들림**.
3. HP 0 → LifeState=Dead → dead UV 표시 → **잠시 후 사라짐**.
4. 보스 = 플레이어와 **같은 텍스처**, **scale만 키움**.

---

## 현재 코드 진단 (인프라 격차)

| 필요 | 현재 상태 | 격차 |
|---|---|---|
| 오브젝트별 크기 조정 | GameObject에 scale 없음. MeshRenderer는 rotation+translation만 | **추가 필요** |
| 텍스처 틴트(빨강) | TextureShader는 단순 텍스처 샘플만, tint 입력 없음 | **셰이더+머티리얼 확장** |
| HP 개념 | LifeState는 Alive/Dead enum만, 수치 없음 | **HealthState/Controller 신설** |
| 팀 구분 | CollisionSystem이 `name == "Player"`로 하드코딩 | **TeamId 도입** |
| 공격 판정 | AttackController는 자기 State만 조작, 외부 영향 없음 | **CombatSystem 또는 콜백 신설** |
| 오브젝트 제거 | GameLoop는 gameWorld에서 절대 제거 안 함 | **제거 메커니즘 필요** |
| 피격 이펙트 | 없음 | **HitReactionController 신설** |

---

## 설계 원칙 (이전 리팩토링과 일관)

- **데이터 vs 로직 분리**: `HealthState`(데이터) + `HealthController`(로직)
- **콜백 중심**: 피격, 사망은 State 변경 → 콜백이 외부 컴포넌트 반응
- **단방향 흐름**: PlayerControl → AttackController → CombatSystem → HealthController → State 변경 → Callbacks
- **System은 GameLoop가 보유** (CollisionSystem 패턴 그대로)

---

## 핵심 데이터 흐름 (가장 중요한 그림)

```
[Space 입력]
  └─ PlayerControl::Update
      └─ AttackController::TriggerSword(0.4f)
          └─ AttackState::Set(SwordAttack)
              ├─ OnAnimAttack    → 검 공격 클립 재생
              ├─ OnControlAttack → isAttackLocked=true
              └─ OnCombatAttack  ← (NEW) CombatSystem이 hitbox 한번 발사 요청

[CombatSystem::Update]
  ├─ pendingHitbox 있으면:
  │   ├─ attacker.position + facingDirection 기반 영역 계산
  │   ├─ gameWorld 검색 → 다른 팀 + 살아있는 GameObject
  │   └─ 검출된 각 대상 → HealthController::TakeDamage(N)
  │
  └─ pendingHitbox 소비/clear

[HealthController::TakeDamage(N)]
  ├─ HealthState::SetCurrent(현재HP - N)  ← ObservableState로 통보
  └─ HP <= 0 이면 LifeState::SetDead()

[HealthState 변경 콜백]
  └─ OnHitReaction → HitReactionController가 빨간 tint + position offset 시작

[LifeState=Dead 콜백]
  ├─ OnAnimLife → SpriteAnimator: "dead" 클립
  ├─ OnControlLife → isMovementLocked=true
  └─ OnDeathTimer ← (NEW) DeathTimer가 N초 카운트 시작

[DeathTimer Update]
  └─ N초 후 owner->pendingDestroy = true
      └─ GameLoop가 다음 프레임에 gameWorld에서 제거 + delete
```

---

## Task 분해 (총 7개)

### Task 1. 인프라: scale + TeamId
**파일**:
- `EngineTypes.h`: `Vec3 scale = {1,1,1}` 위해 Vec3에 기본값 검토 / `enum class TeamId { Player, Enemy, Neutral }`
- `GameObject.h/.cpp`: `Vec3 scale`, `TeamId teamId = Neutral`, `bool pendingDestroy = false` 멤버 추가
- `MeshRenderer.cpp`: world matrix에 `XMMatrixScaling(scale.x, scale.y, scale.z)` 첫번째 곱

**검증**: 기존 플레이어가 정상 표시되어야 함 (scale=1).

---

### Task 2. 셰이더 + TextureMaterial: tint 컬러
**파일**:
- `TextureShader.hlsl`: `cbuffer TintBuffer : register(b1) { float4 tint; }` 추가, PS에서 `sample * tint`
- `TextureMaterial.h/.cpp`: `ID3D11Buffer* pTintBuffer`, `void SetTint(float r,g,b,a)`. Bind 시 b1 슬롯에 업로드. 기본 (1,1,1,1).

**검증**: 기본 색상 변화 없음. SetTint(1,0,0,1) 호출하면 빨갛게 표시.

---

### Task 3. HealthState + HealthController
**파일**:
- `States/HealthState.h/.cpp`: `ObservableState<int>` 상속 (currentHP), `maxHP` 별도 보관, `SetCurrent`, `GetCurrent`, `GetMax`, `GetRatio`
- `Components/HealthController.h/.cpp`: `Start` (HealthState/LifeState 캐싱), `TakeDamage(int dmg)`, `Heal(int amt)`. HP<=0 시 LifeState::SetDead() 호출.

**리뷰 포인트**: HP를 ObservableState로 둘지 / 단순 멤버로 둘지. 콜백으로 피격 이펙트 발화하려면 Observable이 자연스러움.

---

### Task 4. CombatSystem (공격 판정)
**파일**:
- `FrameWork/CombatSystem.h/.cpp` 신설
  - `struct PendingHit { GameObject* attacker; AttackStateType type; int damage; }`
  - `void RequestHit(GameObject* attacker, AttackStateType, int damage)`
  - `void Update(std::vector<GameObject*>&)`: 큐 처리 → 전방 hitbox 영역 검사 → TakeDamage 호출
- `GameLoop.h/.cpp`: `CombatSystem combatSystem;` 멤버, Update에서 호출
- `Callbacks/StateCallbacks`: `OnCombatAttack(GameObject* attacker, AttackStateType prev, AttackStateType next)` 추가
  - prev=NoAttack && next≠NoAttack 일 때 CombatSystem::RequestHit 호출
  - 콜백은 CombatSystem 포인터 필요 → 전역 또는 GameLoop가 노출

**전방 hitbox**: MovementState::GetDirectionName() 기반으로 +/- x, y offset 적용. 단순 사각형/원형으로 시작.

**리뷰 포인트**:
- 콜백이 시스템 포인터를 알아야 하는데, 현재 콜백은 (component, prev, next) 시그니처. 어디서 CombatSystem을 잡을지.
- 옵션 A: CombatSystem을 전역 싱글톤 (GraphicsContext와 동일 패턴) — 가장 단순
- 옵션 B: AttackController가 CombatSystem 포인터를 보유하고, AttackState.Set 대신 자기가 RequestHit 함께 호출 — 의존 명시적
- **현재 선호**: **옵션 B**. AttackController가 이미 "공격 발동"의 단일 진입점이므로 자연스러움. AttackController::TriggerSword 안에서 attackState.Set + combat->RequestHit. CombatSystem은 GameLoop가 제공하는 정적 접근자 또는 main이 wire.

---

### Task 5. HitReactionController (빨간 깜빡 + 흔들림)
**파일**:
- `Components/HitReactionController.h/.cpp` 신설
  - `Start`: HealthState 콜백 등록 (HP 감소 시 트리거), MeshRenderer 캐싱(tint 접근용)
  - `Update`: 남은 시간 동안 tint를 빨강 → 흰색 보간, position에 sin 기반 작은 흔들림 offset
  - 종료 시 tint 복원, offset 0
- `Callbacks/StateCallbacks`: `OnHealthChanged(HitReactionController*, int prev, int next)` → prev>next면 hitFlash 트리거

**리뷰 포인트**:
- 흔들림 offset을 position에 직접 가산하면 VelocityController가 다음 프레임에 덮어쓸 가능성. → renderOffset 별도 멤버를 GameObject에 두고 MeshRenderer가 world matrix에서 가산.

---

### Task 6. DeathTimer + GameLoop 제거
**파일**:
- `Components/DeathTimer.h/.cpp` 신설
  - `Start`: LifeState 콜백 등록
  - LifeState=Dead 콜백 받으면 내부 타이머 시작 (e.g. 1.5s)
  - `Update`: 남은 시간 0 이하면 `pOwner->pendingDestroy = true`
- `GameLoop.cpp`: Update 끝에 gameWorld 순회 → pendingDestroy true인 것 delete + erase

**검증**: 사망 후 dead 클립 잠깐 보이다가 오브젝트가 사라짐.

---

### Task 7. main.cpp 배선 (Enemy + Boss)
**파일**:
- `main.cpp`:
  - Player에 `HealthState`, `HealthController`, `HitReactionController`, `DeathTimer` 추가, teamId=Player
  - Enemy GameObject 생성 (같은 mesh/material 공유, teamId=Enemy, HP=2)
  - Boss GameObject 생성 (같은 mesh/material, teamId=Enemy, scale=(2.0, 2.0, 1.0), HP=8)
  - 위치/등록 순서 정리

**리뷰 포인트**:
- mesh/material 공유 시 UV 변경(SpriteAnimator)이 모든 인스턴스에 적용됨 (Mesh는 vertex buffer 공유). Enemy/Boss마다 별도 SpriteAnimator → 별도 Mesh 인스턴스 필요할 수 있음.

---

## 결정 사항 (서브에이전트 리뷰 반영, 확정)

1. **CombatSystem 접근 방식** → **AttackController 직접 호출**. AttackController가 `combatSystem` 포인터를 보유, `TriggerSword`에서 `attackState.Set(...) + combat->RequestHit(...)` 함께 호출. 콜백은 "반응" 전용.
2. **Mesh 공유** → **불가**. SpriteAnimator가 mesh vertex buffer를 수정하므로 캐릭터마다 별도 Mesh 인스턴스. **Material(텍스처)은 공유 가능**.
3. **Tint per-instance** → Material에 두면 모든 인스턴스가 함께 빨개짐. **MeshRenderer/GameObject 측 멤버**로 두고 Bind 후 매 프레임 별도 버퍼 업데이트.
4. **HealthState = `ObservableState<int>`** → 일관성. maxHP는 별도 멤버(통보 불필요).
5. **흔들림** → `GameObject::renderOffset` 도입, MeshRenderer가 world matrix 합성 시만 가산. position/velocity 오염 없음.
6. **TeamId** → `GameObject` 직접 멤버. enum 단순값.
7. **사망 후 Update 가드** → HealthController/HitReactionController/AttackController에 `IsDead()` 가드.
8. **1공격 1히트** → CombatSystem `RequestHit`은 AttackController에서 `prev=NoAttack && next≠NoAttack` 진입 1회만 호출. CombatSystem은 큐 1회 소비.
9. **자기 자신 hitbox 스킵** → CombatSystem에서 attacker == target 검사.
10. **pendingDestroy 스윕** → **GameLoop Update 끝(Render 전)에 단 한 곳**에서만 처리. Task 1에 포함.
11. **collisionRadius per-instance** → GameObject에 `float collisionRadius = 0.1f`. CollisionSystem `IsColliding`은 `r1 + r2` 기반으로 변경. 보스는 scale에 맞춰 더 큰 radius.
12. **CollisionSystem의 name 하드코딩** → 이번 범위에서는 그대로 두되 lose 메시지만 비활성화. 정식 정리는 별도 작업.

---

## Task 분해 (재구성, 총 7개 → 8개로 분리)

### Task 1. 인프라: scale + renderOffset + TeamId + pendingDestroy 스윕
- `Vec3 scale = {1,1,1}`, `Vec3 renderOffset = {0,0,0}`, `TeamId teamId = Neutral`, `bool pendingDestroy = false`, `float collisionRadius = 0.1f` 멤버 추가
- `EngineTypes.h`에 `enum class TeamId { Player, Enemy, Neutral }`
- `MeshRenderer`: world matrix = scale × rotation × translation(pos + renderOffset)
- `CollisionSystem::IsColliding`: 거리 ≤ r1+r2
- `GameLoop::Update` 끝에서 `pendingDestroy` 스윕

### Task 2a. 셰이더 tint cbuffer (b1) + TextureMaterial Bind에 빈 슬롯 보장
- HLSL에 `cbuffer TintBuffer : register(b1) { float4 tint; }`, PS에서 `sample * tint`
- TextureMaterial은 변동 없음 (per-instance라 Material이 모름)

### Task 2b. MeshRenderer가 per-instance tint 버퍼 보유/업로드
- `MeshRenderer`: `ID3D11Buffer* pTintBuffer`, `Vec4 tint = {1,1,1,1}`
- Bind 후 매 프레임 b1 슬롯에 업로드

### Task 3. HealthState (Observable) + HealthController
- `States/HealthState.h/.cpp`: `ObservableState<int>` 상속, maxHP 멤버 별도
- `Components/HealthController.h/.cpp`: TakeDamage(int), HP≤0 시 LifeState.SetDead(). IsDead 가드.

### Task 4. CombatSystem + AttackController wiring
- `FrameWork/CombatSystem.h/.cpp`: PendingHit 큐, RequestHit, Update (전방 hitbox 검사, attacker 스킵, 다른 팀 살아있는 적에 TakeDamage)
- `GameLoop`: 멤버로 보유, Update에서 호출
- `AttackController`: `combatSystem` 포인터, `Trigger*` 안에서 RequestHit 동반 호출. main에서 wire.

### Task 5. HitReactionController (tint + 흔들림)
- `Components/HitReactionController.h/.cpp`: HealthState Subscribe (prev > next 시 트리거). Update에서 남은 시간 동안 tint 보간, renderOffset에 sin 흔들림. 종료 시 복원. IsDead 가드.

### Task 6. DeathTimer
- `Components/DeathTimer.h/.cpp`: LifeState=Dead 콜백 → 타이머 시작. Update에서 만료 시 pendingDestroy=true.

### Task 7. main.cpp 배선 (Player + Enemy + Boss)
- 각 캐릭터별 **별도 Mesh** 생성 (Material/텍스처는 공유)
- 등록 순서 명세: AddState 모두 → HealthController/AttackController/PlayerControl/VelocityController/SpriteAnimator/HitReactionController/DeathTimer/MeshRenderer
- Enemy(HP=2, scale=1.0, teamId=Enemy), Boss(HP=8, scale=2.0, teamId=Enemy, collisionRadius=0.2)

---

## 작업 순서
**1 → 2a → 2b → 3 → 4 → 5 → 6 → 7**
중간 빌드 게이트: 1, 2b, 3, 4, 5, 6, 7 끝마다 컴파일 성공해야 함.

---

## 완료 정의
- 플레이어가 적 앞에서 Space → 적 HP 감소 + 빨간 깜빡임 + 흔들림
- HP 0 → 적이 dead 텍스처로 정지 → 1.5초 후 사라짐
- 보스는 같은 텍스처가 2배 크기로 표시되고 동일한 피격 반응
- 기존 플레이어 이동/공격 동작 회귀 없음
