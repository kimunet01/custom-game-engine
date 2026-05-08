#pragma once

/*
 * GameObject.h
 * 게임 월드에 존재하는 하나의 객체를 표현한다.
 *
 * GameObject는 이름, 위치, 속도, 회전, 충돌 상태 같은 공통 상태를 들고 있으며,
 * 실제 행동은 부착된 Component들에게 위임한다. 현재 구조에서는 GameObject가
 * Component와 자식 GameObject의 소유권을 갖고 소멸자에서 delete한다.
 */

#include <string>
#include <vector>

#include "Component.h"
#include "EngineTypes.h"

// 게임 월드에 배치되는 기본 오브젝트.
// 자체 로직을 크게 갖기보다 Component 조합으로 기능을 구성한다.
class GameObject {
private:
    // 계층 구조 확장을 위한 부모/자식 포인터.
    // 현재 transform 상속 처리는 없고, 소멸 시 자식 삭제 용도로만 사용된다.
    GameObject* parentObject;
    std::vector<GameObject*> childObjects;
public:
    // 충돌 판정이나 디버그 출력에서 사용할 식별 이름.
    std::string name;
    // 렌더링과 충돌의 기준이 되는 월드 위치.
    Vec3 position;
    // VelocityController가 position에 반영할 이동 속도.
    Vec3 velocity;
    // MeshRenderer가 world matrix를 만들 때 사용하는 Z축 회전값.
    float rotation;
    // CollisionSystem이 이번 프레임 충돌 여부를 표시하는 플래그.
    bool isCollided;
    // 이 오브젝트에 부착된 행동 컴포넌트 목록.
    std::vector<Component*> components;

    explicit GameObject(const std::string& n);
    ~GameObject();

    // 컴포넌트의 owner를 연결하고 목록에 등록한다.
    void AddComponent(Component* pComp);
    // 자식 오브젝트를 등록한다. 현재는 소멸 시 함께 삭제하는 ownership 역할이 중심이다.
    void AddChildObject(GameObject* pObject);
};
