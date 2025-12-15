#ifndef FALLANIMATIONRENDERER_H
#define FALLANIMATIONRENDERER_H

#include "IAnimationRenderer.h"

class QOpenGLTexture;

/**
 * @brief 下落动画渲染器
 * 
 * 职责：
 * - 使用动画序列中记录的FallMove数据绘制下落动画
 * - 绘制新水果从顶部入场动画
 * 
 * 关键设计：
 * - 使用 round.fall.moves 精确控制每个水果的移动
 * - 使用 round.fall.newFruits 精确控制新水果的生成
 * - 不依赖engineMap（最终状态），确保多轮消除时动画正确
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
        int mapSize,
        const std::vector<QOpenGLTexture*>& textures
    ) override;
};

#endif // FALLANIMATIONRENDERER_H
