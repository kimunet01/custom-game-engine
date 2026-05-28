# PR #3 (Terrain/보스전 셰이더) 점검 및 정리 계획

머지된 동료 작업에서 우리 구조와 맞지 않는 부분 및 잠재 버그를 정리한다.

## A. 동료 PR이 가져온 변경 (요약)
- 신규: `FrameWork/LevelLayout`, `Components/EnvironmentRenderer`, `Components/TerrainStateController`, `assets/Dungeon2.png`
- 수정: `TextureShader.hlsl` (b1에 EnvironmentBuffer), `CollisionSystem.cpp` (Update에 LevelLayout 호출), `PlayerControl.cpp`, `main.cpp`

---

## B. 발견 이슈

### B-1. 인코딩 깨짐 (CP949 → 보이지 않는 글자) — **수정 대상**
- `FrameWork/LevelLayout.h`, `LevelLayout.cpp`
- `Components/EnvironmentRenderer.h`, `EnvironmentRenderer.cpp`
- `Components/TerrainStateController.h`, `TerrainStateController.cpp`

모두 한글 주석이 CP949 인코딩. 빌드 경고 C4819 다수 발생, 다른 환경(UTF-8)에서 읽으면 깨짐. 우리는 `TextureShader.hlsl` 충돌 해결 시 같은 패턴(영어로 재작성)을 이미 적용했음.

### B-2. CollisionSystem의 벽 충돌이 Enemy/Boss에 적용 안 됨 — **수정 대상**
`CollisionSystem::Update`에 동료가 추가한 코드:
```cpp
if (obj->name == "Player1" || obj->name == "Player2" || obj->name == "Player" || obj->name == "Monster")
```
- 우리는 `Player`, `Enemy`, `Boss`를 사용 → **Enemy/Boss가 벽을 통과한다**.
- name 하드코딩 자체가 TeamId 도입 의도와 어긋남.
- LevelLayout 검색이 매 프레임 O(N): 1회 캐싱이 자연스러움.

**수정**: `obj->teamId == Player || obj->teamId == Enemy` 기반으로 교체. LevelLayout 포인터를 CollisionSystem 멤버로 캐싱.

### B-3. bounds 불일치 — **수정 대상**
- `LevelLayout`: minX=-0.85, maxX=0.95, minY=-1.6, maxY=0.8
- `CollisionSystem` (main.cpp의 `SetBounds`): -0.85, 0.85, -0.65, 0.65
- 매 프레임 `ClampGameObjectToBounds`로 LevelLayout 범위에 가뒀다가 곧바로 `ResolveBounds`가 다시 좁은 범위로 가둠 → 사실상 좁은 쪽이 이김. LevelLayout이 의도한 영역을 못 씀.

**수정**: main.cpp의 `SetBounds`를 LevelLayout과 일치(-0.85, 0.95, -1.6, 0.8)시킴.

### B-4. LevelLayout::Update의 self-call dead code — **수정 대상**
```cpp
void LevelLayout::Update(float dt) {
    if (pOwner == nullptr) return;
    ClampGameObjectToBounds(pOwner);  // pOwner = StageTerrain (안 움직임)
    ResolvePillarCollision(pOwner);
    ResolveBoxCollision(pOwner);
}
```
- StageTerrain은 정적이라 클램프/충돌 처리 의미 없음.
- 실제로 의미 있는 호출은 CollisionSystem이 외부에서 함.

**수정**: `LevelLayout::Update` 본체를 비우거나 메서드 삭제.

### B-5. PlayerControl의 빈 if 블록 — **수정 대상**
```cpp
if (moveUp || moveDown || moveLeft || moveRight) {
}
```
의미 없는 코드. **삭제**.

### B-6. StageTerrain의 collisionRadius 미설정 — **수정 대상**
StageTerrain GameObject가 기본 `collisionRadius = 0.1f`로 충돌 검사에 들어감. 우연히 position.z=1.0이라 거리 계산상 안 부딪히지만 명시적이지 않음.

**수정**: `stageTerrain->collisionRadius = 0.0f` (사실상 충돌 제외).

---

## C. 보고서에만 기록 (수정 보류)

### C-1. LevelLayout의 invalid box 좌표
`wallObstacle2 (minX=0.246, maxX=0.220)`, `wallObstacle3 (0.246 vs 0.240)`, `boxObstacle2 (-0.380 vs -0.380)` 등 폭이 0이거나 음수인 사각형이 다수. 조건문이 모두 만족되지 않아 충돌 안 일어남 → silent noop. 동료의 의도(좌표 오타)인지 확인 필요.

### C-2. EnvironmentRenderer가 PS b1에 EnvironmentBuffer 바인딩 후 잔류
캐릭터 MeshRenderer는 PS b1을 갱신하지 않음 → 보스 스테이지 진입 후 캐릭터에도 어두운 톤 적용. **의도라면 OK, 아니면 zero-buffer 바인딩 필요**.

### C-3. TerrainStateController의 5초 자동 보스 진입
실제 Boss GameObject 등장과 무관하게 시간만으로 톤 변경. 게임 로직 연동 필요 시 LifeState 콜백으로 트리거 권장.

### C-4. LevelLayout::m_pillars 미사용
초기화 안 됨, 항상 빈 벡터. `ResolvePillarCollision`은 항상 noop.

### C-5. TerrainStateController::ReportWallCollision 호출 경로 없음
어디서도 호출하지 않음 → `m_isFlashActive` 항상 false. dead code path.

---

## D. 작업 순서
1. **인코딩 정리** (6개 파일, 영어 주석으로 재작성)
2. **CollisionSystem 개선** (TeamId 기반 + LevelLayout 캐싱)
3. **bounds 통일** (main.cpp `SetBounds`)
4. **LevelLayout::Update 비우기**
5. **PlayerControl 빈 if 삭제**
6. **StageTerrain.collisionRadius = 0.0f**

각 단계 후 빌드 게이트.

---

## E. 완료 정의
- 빌드 경고 C4819 (인코딩) 모두 사라짐
- Enemy/Boss가 벽에 부딪히면 막힘
- LevelLayout 영역(-0.85~0.95, -1.6~0.8) 안에서 캐릭터가 자유롭게 이동
- 회귀 없음 (기존 공격/HP/사망/보스 톤 동작 유지)
