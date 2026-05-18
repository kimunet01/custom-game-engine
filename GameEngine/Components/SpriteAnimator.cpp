#include "SpriteAnimator.h"

#include "MovementState.h"
#include "GameObject.h"
#include "Logger.h"

SpriteAnimator::SpriteAnimator(Mesh* mesh)
    : mesh(mesh)
{
    Logger::Info("SpriteAnimator created. hasMesh=%d", mesh != nullptr);
}

void SpriteAnimator::AddClip(const std::string& name, int columns, int rows, int startFrame, int frameCount, float frameDuration, bool loop)
{
    if (columns <= 0 || rows <= 0 || frameCount <= 0 || frameDuration <= 0.0f) {
        Logger::Warning("SpriteAnimator ignored invalid clip. name=%s columns=%d rows=%d frameCount=%d frameDuration=%.3f", name.c_str(), columns, rows, frameCount, frameDuration);
        return;
    }

    AnimationClip clip;
    clip.name = name;
    clip.frameDuration = frameDuration;
    clip.loop = loop;

    for (int i = 0; i < frameCount; ++i) {
        const int frame = startFrame + i;
        const int column = frame % columns;
        const int row = frame / columns;

        if (row >= rows) {
            break;
        }

        SpriteFrame spriteFrame;
        spriteFrame.u0 = static_cast<float>(column) / static_cast<float>(columns);
        spriteFrame.v0 = static_cast<float>(row) / static_cast<float>(rows);
        spriteFrame.u1 = static_cast<float>(column + 1) / static_cast<float>(columns);
        spriteFrame.v1 = static_cast<float>(row + 1) / static_cast<float>(rows);
        clip.frames.push_back(spriteFrame);
    }

    if (!clip.frames.empty()) {
        clips[name] = clip;
        Logger::Info("SpriteAnimator clip added. name=%s frameCount=%zu", name.c_str(), clip.frames.size());
    }
    else {
        Logger::Warning("SpriteAnimator clip has no frames. name=%s", name.c_str());
    }
}

void SpriteAnimator::Start()
{
    if (pOwner != nullptr) {
        movementState = pOwner->GetComponent<MovementState>();
    }

    if (movementState == nullptr) {
        Logger::Warning("SpriteAnimator started without MovementState. owner=%s", pOwner ? pOwner->name.c_str() : "null");
    }

    SelectClipForState();
    isStarted = true;
    Logger::Info("SpriteAnimator started. owner=%s clipCount=%zu", pOwner ? pOwner->name.c_str() : "null", clips.size());
}

void SpriteAnimator::Update(float dt)
{
    SelectClipForState();

    if (currentClip == nullptr || currentClip->frames.empty()) {
        return;
    }

    elapsedTime += dt;
    if (elapsedTime < currentClip->frameDuration) {
        return;
    }

    elapsedTime = 0.0f;
    currentFrameIndex++;

    if (currentFrameIndex >= static_cast<int>(currentClip->frames.size())) {
        currentFrameIndex = currentClip->loop ? 0 : static_cast<int>(currentClip->frames.size()) - 1;
    }

    ApplyCurrentFrame();
}

void SpriteAnimator::SelectClipForState()
{
    if (movementState == nullptr) {
        return;
    }

    const std::string nextClipName = movementState->GetStateName();
    if (currentClipName == nextClipName) {
        return;
    }

    auto it = clips.find(nextClipName);
    if (it == clips.end()) {
        Logger::Warning("SpriteAnimator missing clip for animation state. state=%s", nextClipName.c_str());
        currentClip = nullptr;
        currentClipName = nextClipName;
        return;
    }

    currentClip = &it->second;
    currentClipName = nextClipName;
    currentFrameIndex = 0;
    elapsedTime = 0.0f;
    ApplyCurrentFrame();
    Logger::Info("SpriteAnimator selected clip. name=%s", currentClipName.c_str());
}

void SpriteAnimator::ApplyCurrentFrame()
{
    if (mesh == nullptr || currentClip == nullptr || currentClip->frames.empty()) {
        return;
    }

    const SpriteFrame& frame = currentClip->frames[currentFrameIndex];
    mesh->SetUVRect(frame.u0, frame.v0, frame.u1, frame.v1);
}
