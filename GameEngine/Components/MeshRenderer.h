#pragma once

#include <vector>

#include "Component.h"
#include "D3D11ResourceHandler.h"
#include "EngineTypes.h"
#include "Resources/Mesh.h"
#include "Resources/Material.h"

class MeshRenderer : public Component {
public:
    std::vector<Mesh*> meshes;
    Material* pMaterial;
    ID3D11Buffer* pMatrixBuffer;

    explicit MeshRenderer(std::vector<Mesh*> meshes, Material* mat);
    void Start() override;
    void Render() override;
    ~MeshRenderer();
};
