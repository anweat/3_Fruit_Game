#include "AnimationRenderer.h"

AnimationRenderer::AnimationRenderer()
    : progress_(0.0f)
    , elapsed_(0.0f)
{
}

AnimationRenderer::~AnimationRenderer()
{
    clear();
}

void AnimationRenderer::setContext(const AnimationContext& ctx)
{
    context_ = ctx;
}

void AnimationRenderer::pushEffect(std::unique_ptr<IAnimationEffect> effect)
{
    effectQueue_.push(std::move(effect));
    
    // 如果当前没有动画在播放，立即开始
    if (!currentEffect_ && !effectQueue_.empty()) {
        currentEffect_ = std::move(effectQueue_.front());
        effectQueue_.pop();
        progress_ = 0.0f;
        elapsed_ = 0.0f;
    }
}

void AnimationRenderer::clear()
{
    while (!effectQueue_.empty()) {
        effectQueue_.pop();
    }
    currentEffect_.reset();
    progress_ = 0.0f;
    elapsed_ = 0.0f;
}

bool AnimationRenderer::isPlaying() const
{
    return currentEffect_ != nullptr;
}

bool AnimationRenderer::update(float deltaMs)
{
    if (!currentEffect_) {
        return false;
    }
    
    elapsed_ += deltaMs;
    float duration = currentEffect_->getDuration();
    progress_ = elapsed_ / duration;
    
    if (progress_ >= 1.0f) {
        progress_ = 1.0f;
        
        // 当前动画完成
        currentEffect_.reset();
        elapsed_ = 0.0f;
        
        // 切换到下一个动画
        if (!effectQueue_.empty()) {
            currentEffect_ = std::move(effectQueue_.front());
            effectQueue_.pop();
            progress_ = 0.0f;
        }
        
        return true;  // 有动画完成
    }
    
    return false;
}

void AnimationRenderer::render()
{
    if (currentEffect_) {
        currentEffect_->render(context_, progress_);
    }
}

const char* AnimationRenderer::getCurrentEffectName() const
{
    if (currentEffect_) {
        return currentEffect_->getName();
    }
    return "None";
}
