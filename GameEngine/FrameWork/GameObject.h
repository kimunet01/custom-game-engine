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
#include <type_traits>
#include <vector>

#include "Component.h"
#include "State.h"
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
    std::string name;
    Vec3 position;
    Vec3 velocity;
    float rotation;
    bool isCollided;
    std::vector<Component*> components;
    // 이 오브젝트에 부착된 데이터 State 목록. Component와 달리 lifecycle에 참여하지 않고
    // 값 보유 + 변경 통보(콜백)만 담당한다.
    std::vector<State*> states;

    explicit GameObject(const std::string& n);
    ~GameObject();

    // 컴포넌트의 owner를 연결하고 목록에 등록한다.
    void AddComponent(Component* pComp);
    template <typename T>
    T* GetComponent();
    // State의 owner를 연결하고 목록에 등록한다. 소유권은 GameObject가 갖는다.
    void AddState(State* pState);
    template <typename T>
    T* GetState();
    // 자식 오브젝트를 등록한다. 현재는 소멸 시 함께 삭제하는 ownership 역할이 중심이다.
    void AddChildObject(GameObject* pObject);
};

template <typename T>
T* GameObject::GetComponent()
{
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

    for (Component* component : components) {
        T* matched = dynamic_cast<T*>(component);
        if (matched != nullptr) {
            return matched;
        }
    }

    return nullptr;
}

template <typename T>
T* GameObject::GetState()
{
    static_assert(std::is_base_of<State, T>::value, "T must derive from State");

    for (State* state : states) {
        T* matched = dynamic_cast<T*>(state);
        if (matched != nullptr) {
            return matched;
        }
    }

    return nullptr;
}
