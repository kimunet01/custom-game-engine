#pragma once

/*
 * TerrainState.h
 * 스테이지(지형)의 현재 진행 상태를 표현하는 관측 가능한 State.
 *
 * 데이터(현재 어떤 스테이지 페이즈인지)만 보유하며, 변경 시 구독자(EnvironmentRenderer 등)에게
 * (prev, next)를 통보한다. "5초 후 보스 스테이지 진입" 같은 시간 기반 트리거는
 * TerrainStateController가 담당하고, "보스 스테이지가 되면 화면 톤을 바꿔라" 같은 반응은
 * Callbacks/StateCallbacks의 OnEnvTerrain이 담당한다.
 */

#include "State.h"

enum class TerrainStateType
{
    Normal,     // 일반 스테이지
    BossStage   // 보스 스테이지 (어두운 핏빛 톤 + 초반 깜빡임)
};

class TerrainState : public ObservableState<TerrainStateType>
{
public:
    TerrainState();

    // 의미를 명확히 하기 위한 편의 메서드. 내부적으로 베이스의 Set을 호출한다.
    void SetNormal();
    void SetBossStage();

    bool IsBossStage() const;

    const char* GetStateName() const;
    static const char* ToString(TerrainStateType state);
};
