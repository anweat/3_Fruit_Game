#include "SwapAnimationRenderer.h"
#include <QOpenGLTexture>

SwapAnimationRenderer::SwapAnimationRenderer()
{
}

SwapAnimationRenderer::~SwapAnimationRenderer()
{
}

void SwapAnimationRenderer::render(
    const GameAnimationSequence& animSeq,
    int roundIndex,
    float progress,
    const std::vector<std::vector<Fruit>>& snapshot,
    const std::vector<std::vector<Fruit>>& engineMap,
    float gridStartX,
    float gridStartY,
    float cellSize,
    int mapSize,
    const std::vector<QOpenGLTexture*>& textures)
{
    // 从动画序列获取交换信息
    int row1 = animSeq.swap.row1;
    int col1 = animSeq.swap.col1;
    int row2 = animSeq.swap.row2;
    int col2 = animSeq.swap.col2;
    bool success = animSeq.swap.success;
    
    if (row1 < 0 || col1 < 0 || row2 < 0 || col2 < 0) {
        return;
    }
    
    // 从快照获取交换前的水果
    const Fruit& fruit1 = snapshot[row1][col1];
    const Fruit& fruit2 = snapshot[row2][col2];
    
    float t = progress;
    if (!success) {
        // 回弹：前半程出去，后半程回来
        t = (t <= 0.5f) ? (t * 2.0f) : ((1.0f - t) * 2.0f);
    }
    
    // 计算位移方向
    int dirRow = row2 - row1;
    int dirCol = col2 - col1;
    float dx = dirCol * cellSize * t;
    float dy = dirRow * cellSize * t;
    
    // 绘制第一个水果（从位置1向位置2移动）
    if (fruit1.type != FruitType::EMPTY) {
        drawFruit(row1, col1, fruit1, dx, dy, 1.0f, 1.0f,
                  gridStartX, gridStartY, cellSize, textures);
    }
    
    // 绘制第二个水果（从位置2向位置1移动）
    if (fruit2.type != FruitType::EMPTY) {
        drawFruit(row2, col2, fruit2, -dx, -dy, 1.0f, 1.0f,
                  gridStartX, gridStartY, cellSize, textures);
    }
}
