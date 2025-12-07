#ifndef ANIMATIONRENDERER_H
#define ANIMATIONRENDERER_H

#include "IAnimationEffect.h"
#include "SwapEffect.h"
#include "EliminateEffect.h"
#include "FallEffect.h"
#include "BombEffect.h"
#include <memory>
#include <queue>

/**
 * @brief 动画渲染器
 * 
 * 统一管理动画效果的渲染，支持：
 * - 动画效果队列
 * - 动画状态机控制
 * - 动画进度更新
 */
class AnimationRenderer {
public:
    AnimationRenderer();
    ~AnimationRenderer();
    
    /**
     * @brief 设置渲染上下文
     */
    void setContext(const AnimationContext& ctx);
    
    /**
     * @brief 添加动画效果到队列
     */
    void pushEffect(std::unique_ptr<IAnimationEffect> effect);
    
    /**
     * @brief 清空所有动画
     */
    void clear();
    
    /**
     * @brief 检查是否有正在播放的动画
     */
    bool isPlaying() const;
    
    /**
     * @brief 更新动画进度（每帧调用）
     * @param deltaMs 时间增量（毫秒）
     * @return 是否有动画完成
     */
    bool update(float deltaMs);
    
    /**
     * @brief 渲染当前动画
     */
    void render();
    
    /**
     * @brief 获取当前动画名称（用于调试）
     */
    const char* getCurrentEffectName() const;
    
    /**
     * @brief 获取当前动画进度
     */
    float getProgress() const { return progress_; }

private:
    AnimationContext context_;
    std::queue<std::unique_ptr<IAnimationEffect>> effectQueue_;
    std::unique_ptr<IAnimationEffect> currentEffect_;
    float progress_;
    float elapsed_;
};

#endif // ANIMATIONRENDERER_H
