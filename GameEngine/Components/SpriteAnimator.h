#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Component.h"
#include "Resources/Mesh.h"

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
    void Play(const std::string& name);
    void Update(float dt) override;

private:
    Mesh* mesh = nullptr;
    std::unordered_map<std::string, AnimationClip> clips;
    AnimationClip* currentClip = nullptr;
    int currentFrameIndex = 0;
    float elapsedTime = 0.0f;

    void ApplyCurrentFrame();
};
