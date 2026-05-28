## **Goal**

런타임에 새로운 적(Enemy)을 동적으로 생성(Spawn)할 때마다 게임이 터지는(Crash) 치명적인 버그를 해결한다.

원인 분석 결과, `GameLoop`가 오브젝트 리스트를 순회하며 업데이트를 도는 도중 `new GameObject`로 리스트에 요소를 강제 추가하면서 메모리(Iterator)가 무효화되어 발생하는 문제였다. 

프레임워크의 코어(`GameLoop.cpp`)를 건드리지 않고 이 문제를 안전하게 해결하기 위해, 런타임 동적 할당을 배제한 **오브젝트 풀링(Object Pooling)** 방식을 도입한다. 더불어 적의 행동은 불필요한 상태를 줄이고 상시 추격하는 직관적인 AI로 간소화한다.

## **Design Direction**

게임 도중 리스트 구조가 변하지 않도록 오브젝트 풀링을 설계한다.

`EnemySpawner
 -> 게임 시작(Start) 시 미리 넉넉한 수(30마리)의 적을 일괄 생성하여 풀(Pool)에 보관한다
 -> 런타임에는 절대 new를 호출하지 않고, 풀에서 대기 중인 적의 상태만 활성화(Move)한다
 -> 풀이 비었으면 크래시 방지를 위해 스폰을 쉰다
 -> 적이 죽으면 소멸(delete)시키지 않고 다시 풀로 회수(Disabled)한다`

`EnemyState`는 불필요한 `Idle`, `Attack`, `Hit`을 제거하고 3가지만 남긴다.

`EnemyStateType::Move (추격 중)
EnemyStateType::Dead (사망 연출 중)
EnemyStateType::Disabled (풀 대기 중)`

모든 State 변경에 대한 반응은 중앙에서 관리한다.

`StateCallbacks
 -> EnemyState 변경 이벤트를 구독한다
 -> 상태에 맞춰 SpriteAnimator의 클립을 전환한다
 -> 상태에 맞춰 EnemyController의 움직임을 잠근다`

## **Rules**

- 게임 루프 도중(런타임)에 `new GameObject`를 통해 요소를 강제 추가하여 Iterator 크래시를 유발하지 않는다.
- 비활성(Disabled) 상태의 적은 `CollisionSystem`의 X, Y 경계 보정을 피하기 위해 Z축(10.0f)으로 은닉한다.
- 활성화 시 반드시 Z축을 0.0f로 초기화하여 플레이어와의 3D 충돌 판정이 정상 작동하게 한다.
- 적은 플레이어용 더미 State(`MovementState` 등)를 갖지 않고 오직 `EnemyState`만 갖는다.
- 콜백 로직은 `EnemyCallbacks`로 파편화하지 않고 가이드에 따라 `StateCallbacks`에 통합한다.

## **Completed Tasks**

### **1. Object Pooling Implementation**

- [x]  `EnemySpawner`에 `inactivePool` 벡터 추가
- [x]  `PreAllocate()`를 통해 초기 30마리 적 선할당 구현
- [x]  `Spawn()` 시 `new` 대신 풀에서 객체 재사용 구현
- [x]  `ReturnToPool()` 구현 및 사망 후 타이머를 통한 회수 로직 작성

### **2. State System Simplification (State Diet)**

- [x]  `EnemyState.h`에서 `Idle`, `Attack`, `Hit` 상태 제거
- [x]  `EnemyStateType::Move`, `Dead`, `Disabled` 3가지 체계 확립
- [x]  `EnemyController`의 사거리 기반 행동을 상시 추격(Move)으로 변경

### **3. Centralize Callbacks (Guide Compliance)**

- [x]  `Callbacks/EnemyCallbacks.cpp/h` 삭제
- [x]  `StateCallbacks.cpp` 내부에 Enemy 관련 반응 콜백(`OnAnimEnemy`, `OnControlEnemy`) 이동 및 통합
- [x]  수정된 부분에 중앙 집중화 리팩토링 명시 주석(2026-05-23) 추가

### **4. Fix Bugs & Glitches**

- [x]  [Ghost Slime] 생성 직후 Z축을 10.0f로 설정하여 화면 우측 상단 잔상 완벽 제거
- [x]  [Collision Fail] 스폰 시 Z축을 0.0f로 되돌려 플레이어와 3D 충돌 감지 정상화
- [x]  [Log Spam] 적에게 불필요한 플레이어 더미 State를 제거하여 `SpriteAnimator`의 `stand_down` Warning 제거
- [x]  `CollisionSystem::NotifyCollision`에 "Enemy" 네임태그 감지 추가로 `lose!` 출력 정상화

## **Verify**

- [x]  Debug x64 build
- [x]  적이 스폰 즉시 지연 없이 플레이어를 추격하는지 확인
- [x]  30마리 도달 시 더 이상 생성되지 않으며 게임 크래시가 발생하지 않는지 확인
- [x]  적이 죽은 후 2초 뒤 화면에서 사라지고 풀로 정상 회수되는지 확인
- [x]  플레이어와 적 충돌 시 콘솔에 `lose!` 로그가 뜨는지 확인
- [x]  콘솔에 `SpriteAnimator missing clip` 경고가 발생하지 않는지 확인

## **Later**

- [ ]  풀 크기 초과 시 대기 큐(Queue) 시스템 또는 동적 안전 확장(Safe Expansion) 로직 도입
- [ ]  적(Enemy) 타입 다양화 시 팩토리 패턴을 결합한 스포너 확장
- [ ]  적 사망 시 파티클 이펙트나 사운드 이벤트를 `StateCallbacks`에 추가 연동