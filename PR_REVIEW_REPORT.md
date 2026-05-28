# PR #3 (TerrainController / 보스전 셰이더) 통합 점검 보고서

`feat_combat` 브랜치에서 main(PR #3 머지본)을 받아 통합한 뒤, 우리 프로젝트 구조(데이터/로직/콜백 분리 + TeamId 기반)와 충돌하거나 버그 위험이 있는 지점을 점검하고 정리한 결과.

빌드 상태: ✅ Debug|x64 Rebuild 성공, 동료 PR 파일군의 `C4819` 인코딩 경고 0건.

---

## 1. PR #3이 추가한 것

| 분류 | 파일 | 역할 |
|---|---|---|
| 신규 시스템 | `FrameWork/LevelLayout.{h,cpp}` | 정적 스테이지 데이터(벽 사각형 32개 + 경계) + 클램프/박스 충돌 보정 헬퍼 |
| 신규 컴포넌트 | `Components/EnvironmentRenderer.{h,cpp}` | 스테이지 바닥 메시 렌더 + PS b1 `EnvironmentBuffer`(시간/보스플래그/히트좌표) 업로드 |
| 신규 컴포넌트 | `Components/TerrainStateController.{h,cpp}` | 5초 후 자동 보스 스테이지 진입, EnvironmentRenderer에 시간/플래시 신호 전달 |
| 신규 자원 | `assets/Dungeon2.png` | 던전 바닥 텍스처 |
| 셰이더 변경 | `Common/Resources/Shaders/TextureShader.hlsl` | b1에 `EnvironmentBuffer` 추가, 보스 스테이지에서 어두운 + 핏빛 톤 + 초반 깜빡임 |
| 충돌 시스템 확장 | `FrameWork/CollisionSystem.cpp` | `Update`에서 LevelLayout 찾아 캐릭터 위치 클램프 + 박스 충돌 적용 |
| (기타) | `PlayerControl.cpp`, `main.cpp`, `.vcxproj/.filters` | StageTerrain 배선과 잡다 |

---

## 2. 발견된 문제와 처리

### 🟢 수정 완료

| # | 문제 | 영향 | 조치 |
|---|---|---|---|
| **A1** | `LevelLayout/EnvironmentRenderer/TerrainStateController` 6개 파일이 **CP949 한글 주석**으로 저장 → 빌드 시 `C4819` 경고 다수, 다른 OS에서 깨짐 | 가독성·이식성·로그 노이즈 | 6개 파일 전체를 **영어 주석으로 재작성** + **UTF-8 BOM 추가** |
| **A2** | `CollisionSystem::Update`가 벽 충돌 대상을 **`obj->name == "Player1"/"Player2"/"Player"/"Monster"`** 로 하드코딩 — 우리 Enemy/Boss는 이름이 다름 | **Enemy/Boss가 벽을 그대로 통과** (게임 논리 오류) | `TeamId == Player || Enemy` 기반으로 교체. 적/보스도 자동 포함 |
| **A3** | `CollisionSystem`이 **매 프레임 gameWorld 전체를 순회**하여 LevelLayout 컴포넌트 검색 (O(N)) | 성능, 코드 명료성 | `cachedLevelLayout` 멤버에 1회 캐싱. 첫 번째로 발견 시 저장 |
| **A4** | `CollisionSystem.bounds` (`-0.85, 0.85, -0.65, 0.65`)와 `LevelLayout` 영역 (`-0.85, 0.95, -1.6, 0.8`) **불일치**. 매 프레임 두 다른 범위가 캐릭터를 동시에 클램프 → 좁은 쪽이 항상 이김 | LevelLayout이 의도한 영역(아래쪽 1.6까지)을 못 씀 | `main.cpp`의 `SetBounds`를 LevelLayout과 동일하게 `(-0.85, 0.95, -1.6, 0.8)`로 통일 |
| **A5** | `LevelLayout::Update`가 자기 owner(StageTerrain — 안 움직임)에 `Clamp/Pillar/Box` 호출 — 무의미 | dead code | 메서드 본체를 비우고 설명 주석. 진짜 호출은 CollisionSystem이 수행 |
| **A6** | `LevelLayout::ResolveBoxCollision`이 **하드코딩 반경 0.06f** 사용 → `collisionRadius=0.09f`인 Boss가 벽을 일부 관통 | 보스가 벽에 끼이거나 통과 가능 | `obj->collisionRadius`로 교체 |
| **A7** | `PlayerControl::Update`에 빈 `if (moveUp || moveDown || moveLeft || moveRight) { }` 블록 | dead code, 혼란 | 삭제 |
| **A8** | `StageTerrain` GameObject의 `teamId`/`collisionRadius` **미설정** (기본값 의존). 운 좋게 `z=1.0`이라 안 부딪힐 뿐 | 명시성/안전성 | `teamId = Neutral`, `collisionRadius = 0.0f` 명시 |
| **A9** | **`EnvironmentBuffer(b1)`** 가 StageTerrain 렌더 시 채워진 채 → 캐릭터 MeshRenderer가 같은 셰이더를 쓰면서 b1을 갱신 안 함 → **보스 스테이지 진입 후 캐릭터까지 어두운 핏빛 톤이 적용** | 시각적 버그 (캐릭터 색 누수) | MeshRenderer에 `pEnvNeutralBuffer` 추가. 캐릭터 렌더 시 PS b1에 항상 `(time=0, isBossStage=0, ...)` 바인딩하여 환경 효과 누수 차단 |

### 🟡 보고만 / 보류 (작업 범위 외)

| # | 문제 | 권장 후속 조치 |
|---|---|---|
| **B1** | `LevelLayout` 박스 좌표 중 **8건이 폭 0 또는 음수** (`wallObstacle2`, `wallObstacle3`, `waterBox5`, `waterBox6`, `boxObstacle2`, `boxObstacle3`, `smallRoom`, `waterBox4` 등). `pX+r > minX && pX-r < maxX` 조건이 만족 불가 → silent noop | 동료(@zkfkel123)와 좌표 검토. 의도라면 OK, 오타라면 수정 |
| **B2** | `TerrainStateController`가 **5초 경과만으로 보스 스테이지 진입** — 우리 Boss GameObject 등장/사망과 무관 | Boss GameObject의 `LifeState`/스폰 이벤트에 콜백을 걸어 게임 로직과 동기화 |
| **B3** | `LevelLayout::m_pillars`는 항상 빈 벡터, `ResolvePillarCollision` 영원히 noop | 미사용이면 제거, 사용 예정이면 초기화 추가 |
| **B4** | `TerrainStateController::ReportWallCollision`을 **호출하는 코드 없음** → `m_isFlashActive`는 항상 false | 향후 Bullet/투사체가 벽에 부딪힐 때 호출하도록 wiring. 또는 제거 |
| **B5** | `ResolvePillarCollision`의 반사 공식 `velocity.x *= -0.5f; velocity.y *= -0.5f` — 법선 기준이 아니라 양 축 동시 부호 반전 | pillar 활성화 시 `CollisionSystem::ReflectVelocity`와 동일한 법선 반사 공식으로 교체 |

---

## 3. 변경된 파일 목록

```
M  GameEngine/Components/MeshRenderer.h      (pEnvNeutralBuffer 추가)
M  GameEngine/Components/MeshRenderer.cpp    (PS b1 zero-buffer 바인딩)
M  GameEngine/Components/PlayerControl.cpp   (빈 if 삭제)
M  GameEngine/Components/EnvironmentRenderer.h    (영어 주석 + BOM)
M  GameEngine/Components/EnvironmentRenderer.cpp  (영어 주석 + BOM)
M  GameEngine/Components/TerrainStateController.h    (영어 주석 + BOM)
M  GameEngine/Components/TerrainStateController.cpp  (영어 주석 + BOM)
M  GameEngine/FrameWork/CollisionSystem.h    (cachedLevelLayout, fwd decl)
M  GameEngine/FrameWork/CollisionSystem.cpp  (TeamId 필터 + 캐싱)
M  GameEngine/FrameWork/LevelLayout.h        (영어 주석 + BOM, ResolveBox docstring)
M  GameEngine/FrameWork/LevelLayout.cpp      (영어 주석 + BOM, Update 비움, radius 사용)
M  GameEngine/Core/main.cpp                  (bounds 통일, StageTerrain 명시)
```

---

## 4. 검증 가이드

1. **빌드**: Rebuild 성공, 동료 PR 파일군 `C4819` 0건.
2. **벽 충돌 회귀**: Player가 LevelLayout의 사각형에 부딪히면 막힘 (이전과 동일).
3. **Enemy/Boss 벽 충돌 신규 동작**: Enemy/Boss가 벽 박스에 부딪히면 막혀야 함 (이전엔 통과). 적이 좌표 (0.3, 0.0), 보스가 (-0.5, 0.2)에서 시작 → `room1`/`room2`/`waterBox1` 등으로 이동시 막히는지 확인.
4. **보스 톤 누수 차단**: 5초 후 스테이지 톤이 어두운 핏빛으로 바뀔 때, **캐릭터(Player/Enemy/Boss) 스프라이트는 평상시 색 그대로 유지**.
5. **공격/HP/사망 회귀**: 이전 검증 시나리오(공격, 피격 깜빡임, 사망 후 소멸)가 그대로 동작.

---

## 5. 한 줄 요약

PR #3는 보스전 시각 효과와 지형 충돌 기반을 가져왔지만 (a) **TeamId 미반영**, (b) **bounds 불일치**, (c) **EnvironmentBuffer 누수**, (d) **인코딩 깨짐** 등이 있었음. 이번 정리로 우리 구조와 정합시키고 게임 논리 오류와 시각 누수를 차단했음. 보류한 5개 항목(B1~B5)은 별도 작업으로 분리 권장.
