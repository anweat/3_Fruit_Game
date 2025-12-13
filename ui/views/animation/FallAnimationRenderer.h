#ifndef FALLANIMATIONRENDERER_H
#define FALLANIMATIONRENDERER_H

#include "IAnimationRenderer.h"

class QOpenGLTexture;

/**
 * @brief 下落动画渲染器
 * 
 * 职责：
 * - 绘制旧水果下落动画
 * - 绘制新水果从顶部入场动画
 * 
 * 继承关系：IAnimationRenderer → FallAnimationRenderer
 */
class FallAnimationRenderer : public IAnimationRenderer
{
public:
    FallAnimationRenderer();
    ~FallAnimationRenderer() override;
    
    /**
     * @brief 实现基类接口：渲染下落动画
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
        const std::vector<QOpenGLTexture*>& textures
    ) override;
    
private:
    /**
     * @brief 绘制下落的旧水果
     */
    void renderFallingFruits(
        const FallStep& step,
        float progress,
        const std::vector<std::vector<Fruit>>& engineMap,
        float gridStartX, float gridStartY, float cellSize,
        const std::vector<QOpenGLTexture*>& textures
    );
    
    /**
     * @brief 绘制新生成的水果（从顶部入场）
     */
    void renderNewFruits(
        const FallStep& step,
        float progress,
        const std::vector<std::vector<Fruit>>& engineMap,
        float gridStartX, float gridStartY, float cellSize,
        const std::vector<QOpenGLTexture*>& textures
    );
};

#endif // FALLANIMATIONRENDERER_H
