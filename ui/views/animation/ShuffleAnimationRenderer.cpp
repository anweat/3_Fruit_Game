#include "ShuffleAnimationRenderer.h"
#include <QOpenGLTexture>

ShuffleAnimationRenderer::ShuffleAnimationRenderer()
{
}

ShuffleAnimationRenderer::~ShuffleAnimationRenderer()
{
}

void ShuffleAnimationRenderer::render(
    const GameAnimationSequence& animSeq,
    int roundIndex,
    float progress,
    const std::vector<std::vector<Fruit>>& snapshot,
    const std::vector<std::vector<Fruit>>& engineMap,
    float gridStartX,
    float gridStartY,
    float cellSize,
    const std::vector<QOpenGLTexture*>& textures)
{
    if (!animSeq.shuffled || animSeq.newMapAfterShuffle.empty()) {
        return;
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 第一阶段(0-0.5): 旧元素淡出并缩小
    // 第二阶段(0.5-1.0): 新元素淡入并放大
    
    if (progress < 0.5f) {
        // 第一阶段：绘制快照中的旧元素（淡出）
        float phase = progress / 0.5f;  // 0 → 1
        renderFadeOut(phase, snapshot, gridStartX, gridStartY, cellSize, textures);
    } else {
        // 第二阶段：绘制新地图中的元素（淡入）
        float phase = (progress - 0.5f) / 0.5f;  // 0 → 1
        renderFadeIn(phase, animSeq.newMapAfterShuffle, gridStartX, gridStartY, cellSize, textures);
    }
}

void ShuffleAnimationRenderer::renderFadeOut(
    float phase,
    const std::vector<std::vector<Fruit>>& snapshot,
    float gridStartX, float gridStartY, float cellSize,
    const std::vector<QOpenGLTexture*>& textures)
{
    float alpha = 1.0f - phase;
    float scale = 1.0f - phase * 0.3f;
    
    for (int row = 0; row < MAP_SIZE; ++row) {
        for (int col = 0; col < MAP_SIZE; ++col) {
            const Fruit& fruit = snapshot[row][col];
            if (fruit.type == FruitType::EMPTY) {
                continue;
            }
            
            drawFruit(row, col, fruit, 0.0f, 0.0f, alpha, scale,
                      gridStartX, gridStartY, cellSize, textures);
        }
    }
}

void ShuffleAnimationRenderer::renderFadeIn(
    float phase,
    const std::vector<std::vector<Fruit>>& newMap,
    float gridStartX, float gridStartY, float cellSize,
    const std::vector<QOpenGLTexture*>& textures)
{
    float alpha = phase;
    float scale = 0.7f + phase * 0.3f;
    
    for (int row = 0; row < MAP_SIZE; ++row) {
        for (int col = 0; col < MAP_SIZE; ++col) {
            const Fruit& fruit = newMap[row][col];
            if (fruit.type == FruitType::EMPTY) {
                continue;
            }
            
            drawFruit(row, col, fruit, 0.0f, 0.0f, alpha, scale,
                      gridStartX, gridStartY, cellSize, textures);
        }
    }
}
