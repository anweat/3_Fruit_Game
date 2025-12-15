#include "AnimationController.h"

AnimationController::AnimationController()
    : currentPhase_(AnimPhase::IDLE)
    , progress_(0.0f)
    , currentRoundIndex_(-1)
    , swapSuccess_(false)
    , phaseCompleted_(false)
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
    phaseCompleted_ = false;
}

void AnimationController::beginSwap(bool success)
{
    currentPhase_ = AnimPhase::SWAPPING;
    progress_ = 0.0f;
    swapSuccess_ = success;
    phaseCompleted_ = false;
}

void AnimationController::beginElimination(int roundIndex)
{
    currentPhase_ = AnimPhase::ELIMINATING;
    progress_ = 0.0f;
    currentRoundIndex_ = roundIndex;
    phaseCompleted_ = false;
}

void AnimationController::beginFall(int roundIndex)
{
    currentPhase_ = AnimPhase::FALLING;
    progress_ = 0.0f;
    currentRoundIndex_ = roundIndex;
    phaseCompleted_ = false;
}

void AnimationController::beginShuffle()
{
    currentPhase_ = AnimPhase::SHUFFLING;
    progress_ = 0.0f;
    phaseCompleted_ = false;
}

bool AnimationController::updateProgress()
{
    if (currentPhase_ == AnimPhase::IDLE) {
        return false;
    }
    
    // 如果上一帧已经完成，这一帧触发回调
    if (phaseCompleted_) {
        phaseCompleted_ = false;
        
        // 触发回调（这时最后一帧已经渲染完毕）
        if (onPhaseComplete_) {
            onPhaseComplete_(currentPhase_);
        }
        
        return true;  // 阶段完成
    }
    
    const float frameTime = 16.0f;  // ~60 FPS
    const float duration = getCurrentPhaseDuration();
    const float delta = frameTime / duration;
    
    progress_ += delta;
    
    if (progress_ >= 1.0f) {
        progress_ = 1.0f;
        phaseCompleted_ = true;  // 标记完成，但不立即回调
        return false;  // 返回false，让这一帧还是渲染progress=1.0的画面
    }
    
    return false;  // 阶段继续
}

float AnimationController::getCurrentPhaseDuration() const
{
    switch (currentPhase_) {
        case AnimPhase::SWAPPING:
            return 200.0f;  // 200ms
        case AnimPhase::ELIMINATING:
            return 250.0f;  // 250ms（稍微慢点）
        case AnimPhase::FALLING:
            return 500.0f;  // 500ms（让下落更流畅、清晰）
        case AnimPhase::SHUFFLING:
            return 600.0f;  // 600ms
        case AnimPhase::IDLE:
        default:
            return 0.0f;
    }
}
