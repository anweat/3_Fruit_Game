#include "AnimationController.h"

AnimationController::AnimationController()
    : currentPhase_(AnimPhase::IDLE)
    , progress_(0.0f)
    , currentRoundIndex_(-1)
    , swapSuccess_(false)
{
}

AnimationController::~AnimationController()
{
}

void AnimationController::reset()
{
    currentPhase_ = AnimPhase::IDLE;
    progress_ = 0.0f;
    currentRoundIndex_ = -1;
    swapSuccess_ = false;
}

void AnimationController::beginSwap(bool success)
{
    currentPhase_ = AnimPhase::SWAPPING;
    progress_ = 0.0f;
    swapSuccess_ = success;
}

void AnimationController::beginElimination(int roundIndex)
{
    currentPhase_ = AnimPhase::ELIMINATING;
    progress_ = 0.0f;
    currentRoundIndex_ = roundIndex;
}

void AnimationController::beginFall(int roundIndex)
{
    currentPhase_ = AnimPhase::FALLING;
    progress_ = 0.0f;
    currentRoundIndex_ = roundIndex;
}

void AnimationController::beginShuffle()
{
    currentPhase_ = AnimPhase::SHUFFLING;
    progress_ = 0.0f;
}

bool AnimationController::updateProgress()
{
    if (currentPhase_ == AnimPhase::IDLE) {
        return false;
    }
    
    const float frameTime = 16.0f;  // ~60 FPS
    const float duration = getCurrentPhaseDuration();
    const float delta = frameTime / duration;
    
    progress_ += delta;
    
    if (progress_ >= 1.0f) {
        progress_ = 1.0f;
        
        // 通知阶段完成
        if (onPhaseComplete_) {
            onPhaseComplete_(currentPhase_);
        }
        
        return true;  // 阶段完成
    }
    
    return false;  // 阶段继续
}

float AnimationController::getCurrentPhaseDuration() const
{
    switch (currentPhase_) {
        case AnimPhase::SWAPPING:
            return 200.0f;  // 200ms
        case AnimPhase::ELIMINATING:
            return 220.0f;  // 220ms
        case AnimPhase::FALLING:
            return 180.0f;  // 180ms
        case AnimPhase::SHUFFLING:
            return 600.0f;  // 600ms
        case AnimPhase::IDLE:
        default:
            return 0.0f;
    }
}
