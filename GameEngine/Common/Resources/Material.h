#pragma once

#include "ShaderSet.h"

class Material {
public:
    ShaderSet shaders; // 모든 머티리얼은 셰이더를 가짐


    explicit Material(const ShaderSet& s) : shaders(s) {}
    virtual ~Material() {}

    // 이 머티리얼이 가진 셰이더와 파라미터를 GPU 슬롯에 꽂는 함수
    virtual void Bind() = 0;
};
