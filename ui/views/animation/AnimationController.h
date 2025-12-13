#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <functional>
#include "GameEngine.h"

/**
 * @brief 视图动画阶段（严格按时序推进）
 */
enum class AnimPhase {
    IDLE,           ///< 空闲，等待玩家操作
    SWAPPING,       ///< 交换动画中（成功或失败）
    ELIMINATING,    ///< 消除动画中
    FALLING,        ///< 下落+新生成入场动画中
    SHUFFLING       ///< 死局重排动画中
};

/**
 * @brief 动画状态机控制器
 * 
 * 职责：
 * - 管理动画阶段流转（IDLE → SWAPPING → ELIMINATING → FALLING → ...）
 * - 管理动画进度更新
 * - 协调各个动画渲染器的启动和停止
 */
class AnimationController
{
public:
    AnimationController();
    ~AnimationController();
    
    /**
     * @brief 重置到空闲状态
     */
    void reset();
    
    /**
     * @brief 开始交换动画
     * @param success 交换是否成功
     */
    void beginSwap(bool success);
    
    /**
     * @brief 开始消除动画
     * @param roundIndex 轮次索引
     */
    void beginElimination(int roundIndex);
    
    /**
     * @brief 开始下落动画
     * @param roundIndex 轮次索引
     */
    void beginFall(int roundIndex);
    
    /**
     * @brief 开始重排动画
     */
    void beginShuffle();
    
    /**
     * @brief 更新动画进度（每帧调用）
     * @return 当前阶段是否完成
     */
    bool updateProgress();
    
    /**
     * @brief 获取当前动画阶段
     */
    AnimPhase getCurrentPhase() const { return currentPhase_; }
    
    /**
     * @brief 获取当前动画进度 (0.0~1.0)
     */
    float getProgress() const { return progress_; }
    
    /**
     * @brief 获取当前轮次索引
     */
    int getCurrentRoundIndex() const { return currentRoundIndex_; }
    
    /**
     * @brief 获取交换是否成功
     */
    bool isSwapSuccess() const { return swapSuccess_; }
    
    /**
     * @brief 设置阶段完成回调
     */
    void setPhaseCompleteCallback(std::function<void(AnimPhase)> callback) {
        onPhaseComplete_ = callback;
    }
    
private:
    AnimPhase currentPhase_;         ///< 当前动画阶段
    float progress_;                 ///< 当前阶段进度 0.0~1.0
    int currentRoundIndex_;          ///< 当前轮次索引
    bool swapSuccess_;               ///< 交换是否成功
    
    std::function<void(AnimPhase)> onPhaseComplete_;  ///< 阶段完成回调
    
    /**
     * @brief 获取当前阶段的持续时间（毫秒）
     */
    float getCurrentPhaseDuration() const;
};

#endif // ANIMATIONCONTROLLER_H
