#pragma once

/*
 * Material.h
 * 렌더링에 필요한 셰이더와 머티리얼 파라미터를 하나의 바인딩 단위로 추상화한다.
 *
 * Mesh는 모양을, Material은 그 모양을 어떤 셰이더/색상/텍스처로 그릴지를 담당한다.
 * 구체적인 색상, 텍스처, 조명 파라미터는 Material을 상속한 클래스가 Bind에서 GPU에 전달한다.
 */

#include "ShaderSet.h"

class Material {
public:
    // 모든 머티리얼은 렌더링 파이프라인에 올릴 셰이더 묶음을 가진다.
    ShaderSet shaders;


    explicit Material(const ShaderSet& s) : shaders(s) {}
    virtual ~Material() {}

    // 이 머티리얼이 가진 셰이더와 파라미터를 GPU 슬롯에 꽂는 함수.
    // MeshRenderer는 구체 타입을 모르고 이 인터페이스만 호출한다.
    virtual void Bind() = 0;
};
