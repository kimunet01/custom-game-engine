#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Component.h"
#include "Resources/Mesh.h"

class MovementState;

struct SpriteFrame {
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
};

struct AnimationClip {
    std::string name;
    std::vector<SpriteFrame> frames;
    float frameDuration = 0.1f;
    bool loop = true;
};

class SpriteAnimator : public Component {
public:
    explicit SpriteAnimator(Mesh* mesh);

    void AddClip(const std::string& name, int columns, int rows, int startFrame, int frameCount, float frameDuration, bool loop = true);
    void Start() override;
    void Update(float dt) override;

private:
    Mesh* mesh = nullptr;
    MovementState* movementState = nullptr;
    std::unordered_map<std::string, AnimationClip> clips;
    AnimationClip* currentClip = nullptr;
    std::string currentClipName;
    int currentFrameIndex = 0;
    float elapsedTime = 0.0f;

    void SelectClipForState();
    void ApplyCurrentFrame();
};
