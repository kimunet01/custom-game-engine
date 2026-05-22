#pragma once

/*
 * MovementState.h
 * 캐릭터의 이동/정지 방향 8가지를 표현하는 관측 가능한 State.
 *
 * 데이터(현재 방향 enum)만 보유하며, 값 변경 시 베이스 ObservableState가
 * 등록된 구독자에게 (prev, next)를 통보한다. 이동 로직(velocity 계산 등)은
 * PlayerControl 같은 Component가 담당한다.
 */

#include "State.h"

enum class MovementStateType
{
    StandRight,
    StandLeft,
    StandUp,
    StandDown,
    WalkRight,
    WalkLeft,
    WalkUp,
    WalkDown
};

class MovementState : public ObservableState<MovementStateType>
{
public:
    MovementState();

    // 4방향 입력 플래그를 받아 적절한 Walk/Stand state로 베이스 Set을 호출한다.
    // 입력이 모두 false면 현재 방향 유지하면서 Stand로 전환한다.
    void SetFromDirectionInput(bool moveUp, bool moveDown, bool moveLeft, bool moveRight);

    // 현재 state의 사람이 읽을 수 있는 이름 ("walk_right" 등).
    const char* GetStateName() const;
    // 현재 state의 방향 부분만 ("right"/"left"/"up"/"down"). 애니메이션 클립 이름 조합에 사용.
    const char* GetDirectionName() const;

    // enum 값을 문자열로 변환하는 정적 헬퍼. 콜백 본체에서 named 로그 출력에 사용.
    static const char* ToString(MovementStateType state);

private:
    // 현재 방향을 유지한 채 Stand 상태로 매핑한다.
    MovementStateType GetStandStateForCurrentDirection() const;
};
