#include "ShaderSet.h"

/*
 * ShaderSet.cpp
 * ShaderSet의 COM 리소스 수명 관리 규칙을 구현한다.
 *
 * ShaderSet은 포인터 세 개를 단순 보관하지만, 복사될 수 있는 값 타입처럼 사용되므로
 * 복사 생성/대입에서는 AddRef, 소멸과 덮어쓰기에서는 Release를 수행한다.
 */

ShaderSet::ShaderSet(ID3D11VertexShader* v, ID3D11PixelShader* p, ID3D11InputLayout* l) 
    : vs(v), ps(p), layout(l) {

}
ShaderSet::ShaderSet()
    : vs(nullptr), ps(nullptr), layout(nullptr) {

}

ShaderSet::ShaderSet(const ShaderSet& other)
    : vs(other.vs), ps(other.ps), layout(other.layout) {
    // 같은 D3D11 리소스를 공유하는 복사본이 생기므로 참조 카운트를 올린다.
    AddRefResources();
}

ShaderSet& ShaderSet::operator=(const ShaderSet& other) {
    if (this != &other) {
        // 기존 리소스를 먼저 놓고, 새 리소스를 공유한 뒤 AddRef한다.
        ReleaseResources();
        vs = other.vs;
        ps = other.ps;
        layout = other.layout;
        AddRefResources();
    }
    return *this;
}

ShaderSet::ShaderSet(ShaderSet&& other) noexcept
    : vs(other.vs), ps(other.ps), layout(other.layout) {
    // 이동은 참조 카운트를 늘리지 않고 포인터 소유권만 넘긴다.
    other.vs = nullptr;
    other.ps = nullptr;
    other.layout = nullptr;
}

ShaderSet& ShaderSet::operator=(ShaderSet&& other) noexcept {
    if (this != &other) {
        // 현재 보유 리소스를 정리한 뒤, other가 가진 포인터를 그대로 가져온다.
        ReleaseResources();
        vs = other.vs;
        ps = other.ps;
        layout = other.layout;
        other.vs = nullptr;
        other.ps = nullptr;
        other.layout = nullptr;
    }
    return *this;
}

ShaderSet::~ShaderSet() {
    // 마지막 ShaderSet이 사라질 때 실제 COM 리소스 참조가 감소한다.
    ReleaseResources();
}

void ShaderSet::AddRefResources() {
    // nullptr 검사를 거쳐 유효한 COM 객체만 AddRef한다.
    if (vs) vs->AddRef();
    if (ps) ps->AddRef();
    if (layout) layout->AddRef();
}

void ShaderSet::ReleaseResources() {
    // Release 후 nullptr로 비워 중복 Release 가능성을 낮춘다.
    if (vs) { vs->Release(); vs = nullptr; }
    if (ps) { ps->Release(); ps = nullptr; }
    if (layout) { layout->Release(); layout = nullptr; }
}
