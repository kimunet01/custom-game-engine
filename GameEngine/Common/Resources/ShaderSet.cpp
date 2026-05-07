#include "ShaderSet.h"

ShaderSet::ShaderSet(ID3D11VertexShader* v, ID3D11PixelShader* p, ID3D11InputLayout* l) 
    : vs(v), ps(p), layout(l) {

}
ShaderSet::ShaderSet()
    : vs(nullptr), ps(nullptr), layout(nullptr) {

}

ShaderSet::ShaderSet(const ShaderSet& other)
    : vs(other.vs), ps(other.ps), layout(other.layout) {
    AddRefResources();
}

ShaderSet& ShaderSet::operator=(const ShaderSet& other) {
    if (this != &other) {
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
    other.vs = nullptr;
    other.ps = nullptr;
    other.layout = nullptr;
}

ShaderSet& ShaderSet::operator=(ShaderSet&& other) noexcept {
    if (this != &other) {
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
    ReleaseResources();
}

void ShaderSet::AddRefResources() {
    if (vs) vs->AddRef();
    if (ps) ps->AddRef();
    if (layout) layout->AddRef();
}

void ShaderSet::ReleaseResources() {
    if (vs) { vs->Release(); vs = nullptr; }
    if (ps) { ps->Release(); ps = nullptr; }
    if (layout) { layout->Release(); layout = nullptr; }
}
