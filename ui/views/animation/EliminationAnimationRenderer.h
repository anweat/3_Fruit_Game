#ifndef ELIMINATIONANIMATIONRENDERER_H
#define ELIMINATIONANIMATIONRENDERER_H

#include "IAnimationRenderer.h"

class QOpenGLTexture;

/**
 * @brief 消除动画渲染器
 * 
 * 职责：
 * - 绘制消除动画（水果缩小消失）
 * - 绘制炸弹特效（横排、竖排、菱形、彩虹）
 */
class EliminationAnimationRenderer : public IAnimationRenderer
{
public:
    EliminationAnimationRenderer();
    ~EliminationAnimationRenderer();
    
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
     * @brief 绘制水果消除效果（缩小淡出）
     */
    void renderElimination(
        const EliminationStep& step,
        float progress,
        const std::vector<std::vector<Fruit>>& snapshot,
        float gridStartX, float gridStartY, float cellSize,
        int mapSize,
        const std::vector<QOpenGLTexture*>& textures
    );
    
    /**
     * @brief 绘制炸弹特效
     */
    void renderBombEffects(
        const EliminationStep& step,
        float progress,
        float gridStartX, float gridStartY, float cellSize,
        int mapSize
    );
};

#endif // ELIMINATIONANIMATIONRENDERER_H
