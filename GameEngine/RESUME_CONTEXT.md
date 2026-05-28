# 🗓️ Session Resume Context (2026-05-23 완료)

## 📌 현재 시스템 상태 (Architecture)

### 1. 적(Enemy) 시스템 개요
*   **Object Pooling**: `EnemySpawner`가 시작 시 30마리의 `Enemy_X` 객체를 선할당함. 런타임 중 `new/delete`가 없으므로 메모리 안정성이 확보됨.
*   **Simplified States**: `EnemyState`는 오직 `Move`, `Dead`, `Disabled` 3가지만 존재함.
*   **Centralized Callbacks**: 리팩토링 가이드에 따라 `EnemyCallbacks`는 삭제되었으며, 모든 반응 로직은 `StateCallbacks.cpp` 내에 통합됨.

### 2. 핵심 해결 기술 (Troubleshooting Knowledge)
*   **Z-Axis Hiding**: `CollisionSystem`의 경계 보정(Snapping)을 피하기 위해 비활성 객체는 `Z=10.0f`에 은닉함.
*   **Collision Reset**: 스폰 시 반드시 `Z=0.0f`로 초기화해야 플레이어(Z=0)와 충돌 판정이 발생함.
*   **Iterator Safety**: `GameLoop` 순회 중 리스트 수정을 방지하기 위해 풀이 비었을 경우 추가 생성을 제한함.

---

## 🚀 내일의 작업 목표 (Next Objective)

### 1. 선형 이동(Linear Movement) 적 구현
*   **목표**: 특정 지점(Waypoints) 사이를 선형적으로 왕복하거나 순회하는 적 로직 추가.
*   **구조**: `EnemyController` 내부에 목표 지점 리스트를 가질 수 있도록 확장.
*   **커스터마이징**: 아직 맵 데이터가 없으므로, `main.cpp`나 `Spawner` 수준에서 위치 좌표를 쉽게 주입할 수 있는 인터페이스 설계 필요.

### 2. 기술적 고려사항
*   현재의 "상시 추격 AI"와 "선형 이동 AI"를 모드(Mode)나 별도 컴포넌트로 분리할지 결정 필요.
*   오브젝트 풀링 구조를 유지하면서 각 객체마다 서로 다른 이동 경로를 부여하는 방식 고민.

---

## 📂 주요 관련 파일
*   `Components/EnemySpawner.cpp`: 풀링 및 스폰 제어
*   `Components/EnemyController.cpp`: 적 AI (추격 로직 포함)
*   `Callbacks/StateCallbacks.cpp`: 상태 변화 반응 로직 (중앙 센터)
*   `Core/main.cpp`: 적 자원 및 스포너 초기화
*   `PR_ENEMY_SYSTEM_REFACTOR.md`: 오늘 팀장님께 보고한 최종 작업서
