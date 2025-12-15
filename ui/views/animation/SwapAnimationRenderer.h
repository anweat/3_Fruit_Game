#ifndef SWAPANIMATIONRENDERER_H
#define SWAPANIMATIONRENDERER_H

#include "IAnimationRenderer.h"

class QOpenGLTexture;

/**
 * @brief 交换动画渲染器
 * 
 * 职责：
 * - 绘制交换动画（两个水果的移动）
 * - 支持成功交换和失败回弹
 * 
 * 继承关系：IAnimationRenderer → SwapAnimationRenderer
 */
class SwapAnimationRenderer : public IAnimationRenderer
{
public:
    SwapAnimationRenderer();
    ~SwapAnimationRenderer() override;
    
    /**
     * @brief 实现基类接口：渲染交换动画
     */
    void render(
        const GameAnimationSequence& animSeq,
        int roundIndex,
        float progress,
        const std::vector<std::vector<Fruit>>& snapshot,
        const std::vector<std::vector<Fruit>>& engineMap,
        float gridStartX,
        float gridStartY,
        float cellSize,
        int mapSize,
        const std::vector<QOpenGLTexture*>& textures
    ) override;
    
private:
    // 无需额外成员变量，所有信息从 animSeq 获取
};

#endif // SWAPANIMATIONRENDERER_H
