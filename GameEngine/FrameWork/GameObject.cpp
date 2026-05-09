#include "GameObject.h"

/*
 * GameObject.cpp
 * GameObject의 기본 상태 초기화와 컴포넌트/자식 오브젝트 소유권 관리를 구현한다.
 *
 * 이 파일은 오브젝트 자체의 게임 로직을 넣는 곳이 아니라, Component 기반 구조가
 * 동작하기 위한 연결 작업을 담당한다.
 */

GameObject::GameObject(const std::string& n)
    : name(n), parentObject(nullptr), rotation(0.0f), isCollided(false) {
    velocity.x = 0.f;
    velocity.y = 0.f;
    velocity.z = 0.f;
}

// 부착된 컴포넌트와 자식 오브젝트는 GameObject가 소유한다.
// 따라서 GameLoop가 GameObject를 delete하면 하위 구성도 함께 정리된다.
GameObject::~GameObject() {
    for (Component* component : components) {
        delete component;
    }
    for (GameObject* object : childObjects) {
        delete object;
    }
}

// 컴포넌트를 부착할 때 owner를 연결한다.
// GameLoop는 이후 pOwner를 통해 컴포넌트가 자신이 붙은 오브젝트 상태에 접근한다고 가정한다.
void GameObject::AddComponent(Component* pComp)
{
    pComp->pOwner = this;
    pComp->isStarted = false;
    components.push_back(pComp);
}

void GameObject::AddChildObject(GameObject* pObject) {
    // 현재 parentObject 연결은 하지 않고 childObjects에만 보관한다.
    // transform 계층 구조가 필요해지면 parentObject 설정과 월드 변환 상속을 함께 추가해야 한다.
    childObjects.push_back(pObject);
}
