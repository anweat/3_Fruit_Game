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
    const std::vector<std::vector<Fruit>>& /*engineMap*/,  // 不再使用engineMap
    float gridStartX,
    float gridStartY,
    float cellSize,
    int mapSize,
    const std::vector<QOpenGLTexture*>& textures)
{
    if (roundIndex < 0 || roundIndex >= static_cast<int>(animSeq.rounds.size())) {
        return;
    }
    
    const auto& round = animSeq.rounds[roundIndex];
    
    // 🔧 关键修复：使用动画序列中记录的精确移动数据，而不是比较snapshot和engineMap
    // 这样可以正确处理多轮消除，因为每轮的移动数据都是独立记录的
    
    // 1. 渲染移动中的水果（从FallMove获取类型）
    for (const auto& move : round.fall.moves) {
        int fromRow = move.fromRow;
        int fromCol = move.fromCol;
        int toRow = move.toRow;
        int toCol = move.toCol;
        
        // 跳过无效移动
        if (fromRow < 0 || fromRow >= mapSize || fromCol < 0 || fromCol >= mapSize) {
            continue;
        }
        if (toRow < 0 || toRow >= mapSize || toCol < 0 || toCol >= mapSize) {
            continue;
        }
        
        // 🔧 关键修复：从FallMove直接获取水果类型（不再依赖snapshot）
        if (move.type == FruitType::EMPTY) {
            continue; // 空水果，跳过
        }
        
        Fruit fruit;
        fruit.type = move.type;
        fruit.special = move.special;
        
        // 计算插值位置
        float startY = gridStartY + fromRow * cellSize;
        float endY = gridStartY + toRow * cellSize;
        float curY = startY + (endY - startY) * progress;
        float offsetY = curY - endY;
        
        // 在目标位置绘制（带偏移）
        drawFruit(toRow, toCol, fruit, 0.0f, offsetY, 1.0f, 1.0f,
                  gridStartX, gridStartY, cellSize, textures);
    }
    
    // 2. 渲染新生成的水果（从动画数据获取类型）
    // 关键修复：所有新水果从**同一位置**开始下落（gridStartY - cellSize * mapSize）
    float newFruitStartY = gridStartY - cellSize * mapSize;  // 统一起始位置
    
    for (const auto& nf : round.fall.newFruits) {
        int row = nf.row;
        int col = nf.col;
        
        if (row < 0 || row >= mapSize || col < 0 || col >= mapSize) {
            continue;
        }
        
        // 构造水果数据
        Fruit fruit;
        fruit.type = nf.type;
        fruit.special = nf.special;
        
        // 新水果从 newFruitStartY 统一下落到各自的目标行
        float startY = newFruitStartY;
        float endY = gridStartY + nf.row * cellSize;
        float curY = startY + (endY - startY) * progress;
        float offsetY = curY - endY;
        
        drawFruit(row, col, fruit, 0.0f, offsetY, 1.0f, 1.0f,
                  gridStartX, gridStartY, cellSize, textures);
    }
} 