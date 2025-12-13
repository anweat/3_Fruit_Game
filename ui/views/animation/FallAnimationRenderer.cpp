#include "FallAnimationRenderer.h"
#include <QOpenGLTexture>

FallAnimationRenderer::FallAnimationRenderer()
{
}

FallAnimationRenderer::~FallAnimationRenderer()
{
}

void FallAnimationRenderer::render(
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
    if (roundIndex < 0 || roundIndex >= static_cast<int>(animSeq.rounds.size())) {
        return;
    }
    
    const FallStep& step = animSeq.rounds[roundIndex].fall;
    
    // 绘制下落的旧水果
    renderFallingFruits(step, progress, engineMap, gridStartX, gridStartY, cellSize, textures);
    
    // 绘制新生成的水果
    renderNewFruits(step, progress, engineMap, gridStartX, gridStartY, cellSize, textures);
}

void FallAnimationRenderer::renderFallingFruits(
    const FallStep& step,
    float progress,
    const std::vector<std::vector<Fruit>>& engineMap,
    float gridStartX, float gridStartY, float cellSize,
    const std::vector<QOpenGLTexture*>& textures)
{
    // 绘制下落的老元素（from→to 插值）
    for (const auto& move : step.moves) {
        int toRow = move.toRow;
        int toCol = move.toCol;
        if (toRow < 0 || toRow >= MAP_SIZE || toCol < 0 || toCol >= MAP_SIZE) {
            continue;
        }
        
        const Fruit& fruit = engineMap[toRow][toCol];
        if (fruit.type == FruitType::EMPTY) {
            continue;
        }
        
        // 计算起始和目标Y坐标
        float fromY = gridStartY + move.fromRow * cellSize;
        float toY   = gridStartY + move.toRow   * cellSize;
        float curY  = fromY + (toY - fromY) * progress;
        float offsetY = curY - toY;
        
        // 绘制水果（使用引擎的最终数据，因为需要知道最终水果类型）
        drawFruit(toRow, toCol, fruit, 0.0f, offsetY, 1.0f, 1.0f,
                  gridStartX, gridStartY, cellSize, textures);
    }
}

void FallAnimationRenderer::renderNewFruits(
    const FallStep& step,
    float progress,
    const std::vector<std::vector<Fruit>>& engineMap,
    float gridStartX, float gridStartY, float cellSize,
    const std::vector<QOpenGLTexture*>& textures)
{
    // 绘制新生成的水果（从网格上方掉入）
    for (const auto& nf : step.newFruits) {
        int row = nf.row;
        int col = nf.col;
        if (row < 0 || row >= MAP_SIZE || col < 0 || col >= MAP_SIZE) {
            continue;
        }
        
        const Fruit& fruit = engineMap[row][col];
        if (fruit.type == FruitType::EMPTY) {
            continue;
        }
        
        // 新水果从网格上方落下
        float startY = gridStartY - cellSize * 1.5f;
        float endY   = gridStartY + row * cellSize;
        float curY   = startY + (endY - startY) * progress;
        float offsetY = curY - endY;
        
        drawFruit(row, col, fruit, 0.0f, offsetY, 1.0f, 1.0f,
                  gridStartX, gridStartY, cellSize, textures);
    }
}
