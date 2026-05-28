#pragma once

/*
 * CombatSystem.h
 * 공격 hitbox 판정과 데미지 전달을 담당하는 시스템.
 *
 * AttackController가 공격을 발동할 때 RequestHit으로 1회성 PendingHit을 큐에 등록한다.
 * GameLoop는 매 프레임 Update를 호출해 큐에 쌓인 공격을 처리한다.
 *
 * Update는 attacker의 전방 영역(MovementState의 facing 방향 기준)에 있는
 * 다른 팀의 살아있는 GameObject를 찾아 HealthController::TakeDamage를 호출한다.
 *
 * 한 PendingHit은 1회만 처리된다 — 같은 공격 동안 동일 적에게 매 프레임 데미지가 들어가지 않는다.
 * 자기 자신, 같은 팀, 이미 사망한 대상은 hitbox에서 제외한다.
 */

#include <vector>

#include "EngineTypes.h"

class GameObject;
enum class AttackStateType;

struct PendingHit {
    // 공격을 시작한 오브젝트. attacker == target은 검사 단계에서 스킵한다.
    GameObject* attacker;
    // 공격 종류. 현재는 SwordAttack/MagicAttack 모두 동일한 hitbox로 처리.
    AttackStateType type;
    // 적중 시 적용할 데미지 양.
    int damage;
};

class CombatSystem {
public:
    CombatSystem();

    // AttackController 등이 공격 발동 시 1회 호출한다.
    // attacker가 살아있고, AttackState가 NoAttack에서 진입한 첫 프레임이라는 보장은 호출자 책임.
    void RequestHit(GameObject* attacker, AttackStateType type, int damage);

    // 큐에 쌓인 모든 PendingHit을 처리한다. 처리 후 큐는 비워진다.
    void Update(const std::vector<GameObject*>& gameObjects);

private:
    // 1회성 큐. Update가 비운다.
    std::vector<PendingHit> pendingHits;

    // attacker의 전방 hitbox에 target이 들어오는지 검사한다.
    // 전방 방향은 attacker가 보유한 MovementState의 facing direction에서 가져온다.
    bool IsInFrontalHitbox(const GameObject* attacker, const GameObject* target) const;
};
