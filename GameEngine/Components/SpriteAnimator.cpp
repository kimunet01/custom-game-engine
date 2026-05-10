#include "SpriteAnimator.h"

SpriteAnimator::SpriteAnimator(Mesh* mesh)
    : mesh(mesh)
{
}

void SpriteAnimator::AddClip(const std::string& name, int columns, int rows, int startFrame, int frameCount, float frameDuration, bool loop)
{
    if (columns <= 0 || rows <= 0 || frameCount <= 0) {
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
    }
}

void SpriteAnimator::Play(const std::string& name)
{
    auto it = clips.find(name);
    if (it == clips.end()) {
        return;
    }

    if (currentClip == &it->second) {
        return;
    }

    currentClip = &it->second;
    currentFrameIndex = 0;
    elapsedTime = 0.0f;
    ApplyCurrentFrame();
}

void SpriteAnimator::Update(float dt)
{
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

void SpriteAnimator::ApplyCurrentFrame()
{
    if (mesh == nullptr || currentClip == nullptr || currentClip->frames.empty()) {
        return;
    }

    const SpriteFrame& frame = currentClip->frames[currentFrameIndex];
    mesh->SetUVRect(frame.u0, frame.v0, frame.u1, frame.v1);
}
