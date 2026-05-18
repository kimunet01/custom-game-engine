# Task Plan

이 문서는 현재 엔진 개발 과정에서 진행할 주요 작업을 정리한다.

## 1. Object State And Animation Flow

### Goal

오브젝트의 상태를 애니메이션, 입력, 이동, 충돌 같은 여러 시스템이 함께 사용할 수 있게 만든다.

단, `GameObject`에 고정된 `enum state`를 직접 추가하지 않는다. 상태 종류는 계속 늘어날 수 있으므로, 확장 가능한 상태 저장소를 둔다.

또한 컴포넌트끼리 직접 의존하지 않는다. 예를 들어 `SpriteAnimator`가 `PlayerControl`이나 `VelocityController`를 직접 참조하지 않게 한다.

### Design Direction

`GameObject`에 `StateContext`를 추가한다.

```cpp
class GameObject {
public:
    StateContext state;
};
```

`StateContext`는 key-value 기반 상태 저장소다.

```cpp
state.SetString("animation.clip", "idle");
state.SetBool("movement.isMoving", true);
state.SetString("facing.direction", "right");
```

각 컴포넌트는 다른 컴포넌트를 직접 보지 않고, 필요한 상태만 `GameObject::state`를 통해 읽거나 쓴다.

```text
PlayerControl
 -> input을 읽는다
 -> velocity를 설정한다
 -> animation.clip 같은 의도 상태를 기록한다

VelocityController
 -> velocity만 position에 반영한다

SpriteAnimator
 -> animation.clip을 읽는다
 -> 현재 clip의 UV frame을 재생한다

MeshRenderer
 -> 현재 Mesh와 Material만 렌더링한다
```

### Rules

- `GameObject`에 `enum State` 같은 단일 상태값을 추가하지 않는다.
- `SpriteAnimator`는 `velocity`를 직접 읽지 않는다.
- `VelocityController`는 animation 상태를 알지 않는다.
- `PlayerControl`은 입력을 해석하고, 필요한 상태를 `StateContext`에 기록한다.
- 상태 key는 문자열을 직접 흩뿌리지 않고 `StateKeys`로 모은다.
- 나중에 `StateMachineComponent`가 필요해져도 다른 컴포넌트가 그것을 직접 참조하지 않게 한다.

### Planned Tasks

#### 1.1 Add StateContext

- [ ] `FrameWork/StateContext.h` 추가
- [ ] `FrameWork/StateContext.cpp` 추가
- [ ] `SetBool`, `GetBool` 구현
- [ ] `SetFloat`, `GetFloat` 구현
- [ ] `SetString`, `GetString` 구현
- [ ] 기본값 반환 API 제공

#### 1.2 Add StateKeys

- [ ] `FrameWork/StateKeys.h` 추가
- [ ] animation 관련 key 추가
  - `animation.clip`
- [ ] movement 관련 key 추가
  - `movement.isMoving`
- [ ] facing 관련 key 추가
  - `facing.direction`

#### 1.3 Connect GameObject

- [ ] `GameObject`에 `StateContext state` 추가
- [ ] 기존 생성/소멸 흐름에 영향 없는지 확인
- [ ] include dependency가 꼬이지 않도록 `StateContext`는 가벼운 타입으로 유지

#### 1.4 Update PlayerControl

- [ ] 입력 여부를 기준으로 `movement.isMoving` 기록
- [ ] 방향키 입력을 기준으로 `facing.direction` 기록
- [ ] 움직이는 중이면 `animation.clip = walk`
- [ ] 멈춰 있으면 `animation.clip = idle`
- [ ] `SpriteAnimator`를 직접 참조하지 않기

#### 1.5 Update SpriteAnimator

- [ ] `animation.clip` 상태를 읽어 현재 clip과 다르면 `Play()` 호출
- [ ] 같은 clip이면 재시작하지 않기
- [ ] clip이 없으면 현재 frame 유지
- [ ] idle clip은 1 frame으로 유지 가능하게 설계
- [ ] walk clip은 여러 frame을 반복 재생

#### 1.6 Update Sample Scene

- [ ] `main.cpp`에서 `idle` clip 추가
- [ ] `main.cpp`에서 `walk` clip 추가
- [ ] 처음 상태는 `animation.clip = idle`
- [ ] 방향키 입력 시 walk animation이 재생되는지 확인
- [ ] 입력이 없으면 idle frame에 멈추는지 확인

#### 1.7 Verify

- [ ] Debug x64 build
- [ ] 캐릭터가 방향키로 이동하는지 확인
- [ ] 정지 상태에서 UV frame이 계속 바뀌지 않는지 확인
- [ ] 이동 중에만 UV frame이 바뀌는지 확인
- [ ] 컴포넌트 간 직접 참조가 생기지 않았는지 확인

### Later

- [ ] `AnimationClip`을 코드가 아니라 JSON 또는 txt로 정의
- [ ] clip별 loop 여부 지원 강화
- [ ] animation end event 추가
- [ ] facing direction에 따른 좌우 반전 또는 방향별 clip 선택
- [ ] collision event도 `StateContext` 또는 event queue로 연결

---

## 2. Logger

### Goal

개발 과정에 필요한 로그 기록을 위한 `Logger` 기능을 구현한다.

로그는 로컬 디버깅뿐 아니라 팀의 Firebase 저장소와 연동할 수 있게 만든다. 단, 로그 기록과 Firebase 저장소 정보가 public하게 노출되지 않도록 관리한다.

### Branch

```text
feat_Logger
```

### PR Link

추가 예정

### Implementation Details

#### 2.1 Add Logger Class

- [ ] `Common/Logger.h` 추가
- [ ] `Common/Logger.cpp` 추가
- [ ] 기본 로그 레벨 정의
  - `Debug`
  - `Info`
  - `Warning`
  - `Error`
- [ ] 콘솔 출력 지원
- [ ] 파일 출력 또는 외부 저장소 전송을 고려한 인터페이스 설계
- [ ] Logger 사용 방식은 전역 접근 또는 싱글톤 중 프로젝트 스타일에 맞춰 결정

예상 API 예시:

```cpp
Logger::Info("GameLoop started");
Logger::Warning("Texture load failed");
Logger::Error("D3D device creation failed");
```

#### 2.2 Apply Logger To Engine Flow

- [ ] `GameLoop` 주요 lifecycle에 로그 추가
  - initialize
  - update loop start/end
  - shutdown
- [ ] `GameObject` 생성/소멸 또는 component attach 시 로그 추가
- [ ] `Component` lifecycle에 로그 적용 검토
  - `Start`
  - `Update`
  - `Render`
- [ ] `D3D11ResourceHandler` 주요 실패 지점에 로그 추가
  - window creation failure
  - device/swap chain creation failure
  - shader compile failure
  - texture load failure
- [ ] 로그가 너무 과도하게 찍히지 않도록 frame 단위 반복 로그는 제한

#### 2.3 Document Usage

- [ ] `Logger` 적용 방법 문서화
- [ ] 팀원이 새 클래스/함수에 Logger를 적용하는 기준 정리
- [ ] 로그 레벨 사용 기준 작성
- [ ] 예제 코드 작성

문서에 포함할 예시:

```cpp
void Mesh::createVertexBuffer()
{
    Logger::Info("Creating vertex buffer");

    if (mesh.empty()) {
        Logger::Warning("Cannot create vertex buffer from empty mesh");
        return;
    }
}
```

#### 2.4 Connect Firebase Storage

- [ ] 팀 Firebase 저장소 연동 방식 결정
- [ ] Firebase SDK 또는 REST API 사용 여부 결정
- [ ] 인증 정보 관리 방식 결정
- [ ] 로그 업로드 실패 시 엔진 실행에 영향 없도록 설계
- [ ] 네트워크 비활성 환경에서 fallback 동작 정의
- [ ] Firebase 전송 기능은 `Logger` 내부 구현으로 숨기고, 호출부 API는 유지

예상 구조:

```text
Logger
 -> ConsoleLogSink
 -> FileLogSink
 -> FirebaseLogSink
```

#### 2.5 Protect Private Log And Firebase Data

- [ ] `.gitignore`에 로컬 로그 파일 제외 규칙 추가
- [ ] Firebase 인증 파일 제외 규칙 추가
- [ ] `.env` 또는 config 파일 제외 규칙 추가
- [ ] 실제 secret이 커밋되지 않았는지 확인

예상 `.gitignore` 항목:

```gitignore
logs/
*.log
.env
firebase-config.json
firebase-service-account.json
```

#### 2.6 Verify

- [ ] Debug x64 build
- [ ] Logger 기본 출력 확인
- [ ] GameLoop/GameObject/Component 로그 출력 확인
- [ ] Firebase 전송 성공 확인
- [ ] Firebase 전송 실패 시 엔진이 중단되지 않는지 확인
- [ ] `.gitignore` 적용 후 민감 파일이 추적되지 않는지 확인

### Expected Modified Files

- `Common/Logger.h`
- `Common/Logger.cpp`
- `FrameWork/GameLoop.cpp`
- `FrameWork/GameObject.cpp`
- `FrameWork/Component.cpp`
- `Common/D3D11ResourceHandler.cpp`
- `.gitignore`
- Logger 사용 가이드 문서

### Notes

- Firebase 인증 정보는 절대 repository에 포함하지 않는다.
- 로그 호출이 엔진 흐름을 방해하지 않게 설계한다.
- frame마다 반복되는 로그는 성능과 가독성을 해칠 수 있으므로 필요한 경우 throttle 또는 once log를 제공한다.
