#ifndef SHUFFLEANIMATIONRENDERER_H
#define SHUFFLEANIMATIONRENDERER_H

#include "IAnimationRenderer.h"

class QOpenGLTexture;

/**
 * @brief 重排动画渲染器
 * 
 * 职责：
 * - 绘制死局重排动画
 * - 旧元素淡出缩小 → 新元素淡入放大
 * 
 * 继承关系：IAnimationRenderer → ShuffleAnimationRenderer
 */
class ShuffleAnimationRenderer : public IAnimationRenderer
{
public:
    ShuffleAnimationRenderer();
    ~ShuffleAnimationRenderer() override;
    
    /**
     * @brief 实现基类接口：渲染重排动画
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
    /**
     * @brief 绘制淡出阶段（旧地图）
     */
    void renderFadeOut(
        float phase,
        const std::vector<std::vector<Fruit>>& snapshot,
        float gridStartX, float gridStartY, float cellSize,
        int mapSize,
        const std::vector<QOpenGLTexture*>& textures
    );
    
    /**
     * @brief 绘制淡入阶段（新地图）
     */
    void renderFadeIn(
        float phase,
        const std::vector<std::vector<Fruit>>& newMap,
        float gridStartX, float gridStartY, float cellSize,
        int mapSize,
        const std::vector<QOpenGLTexture*>& textures
    );
};

#endif // SHUFFLEANIMATIONRENDERER_H
