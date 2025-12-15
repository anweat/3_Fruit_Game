#include "FallEffect.h"
#include <QOpenGLFunctions>
#include <algorithm>

FallEffect::FallEffect(const std::vector<FallMove>& moves, const std::vector<NewFruit>& newFruits,
                       float cellSize, int mapSize)
    : moves_(moves), newFruits_(newFruits), mapSize_(mapSize), cellSize_(cellSize), 
      totalFallDistance_(0.0f)
{
    // 关键设计：所有元素在同一时间完成动画
    // 总下落距离 = 从网格顶部上方(-mapSize 格) 到网格底部(第 mapSize-1 行) 的距离
    // 即：mapSize * cellSize（上方距离）+ mapSize * cellSize（网格高度）= 2 * mapSize * cellSize
    // 但实际上，最远的下落是从 row=-mapSize 到 row=mapSize-1
    // 对于新水果：起始 y = gridStartY - cellSize * mapSize
    // 对于第一行新水果：目标 y = gridStartY + 0 * cellSize
    // 距离 = cellSize * mapSize
    
    // 对于旧元素，最坏情况是从 row=0 到 row=mapSize-1，距离 = cellSize * (mapSize-1)
    
    // 因此总下落距离应该是：cellSize * mapSize（这是新水果的最大下落距离）
    totalFallDistance_ = cellSize * mapSize;
    
    // 根据恒定速度计算动画时长
    if (totalFallDistance_ > 0.0f) {
        duration_ = std::max(150.0f, totalFallDistance_ / FALL_SPEED);
    } else {
        duration_ = 250.0f;  // 默认250ms
    }
}

void FallEffect::render(const AnimationContext& ctx, float progress)
{
    // 1. 绘制下落的老元素（from→to 插值）
    for (const auto& move : moves_) {
        float fromY = ctx.cellY(move.fromRow);
        float toY = ctx.cellY(move.toRow);
        float curY = fromY + (toY - fromY) * progress;
        float offsetY = curY - toY;
        
        drawFruitAtOffset(ctx, move.toRow, move.toCol, move.fruit, offsetY);
    }
    
    // 2. 绘制新生成的水果（从网格上方掉入）
    // 关键修复：所有新水果从**同一位置**开始下落（gridStartY - cellSize * mapSize）
    // 这样所有新水果都在同一时间完成下落到各自的目标行
    float newFruitStartY = ctx.gridStartY - ctx.cellSize * mapSize_;  // 统一起始位置
    
    for (const auto& nf : newFruits_) {
        float startY = newFruitStartY;
        float endY = ctx.cellY(nf.row);
        float curY = startY + (endY - startY) * progress;
        float offsetY = curY - endY;
        
        drawFruitAtOffset(ctx, nf.row, nf.col, nf.fruit, offsetY);
    }
}

void FallEffect::drawFruitAtOffset(const AnimationContext& ctx,
                                   int row, int col, const Fruit& fruit, float offsetY)
{
    if (fruit.type == FruitType::EMPTY) return;
    if (!ctx.textures) return;
    
    float x = ctx.cellX(col);
    float y = ctx.cellY(row) + offsetY;
    
    int textureIndex = static_cast<int>(fruit.type);
    if (fruit.type == FruitType::CANDY) {
        textureIndex = 6;
    }
    
    if (textureIndex < 0 || textureIndex >= static_cast<int>(ctx.textures->size())) {
        return;
    }
    
    QOpenGLTexture* texture = (*ctx.textures)[textureIndex];
    if (!texture) return;
    
    glEnable(GL_TEXTURE_2D);
    texture->bind();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    float padding = ctx.cellSize * 0.1f;
    float fruitSize = ctx.cellSize - padding * 2;
    float fruitX = x + padding;
    float fruitY = y + padding;
    
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(fruitX, fruitY);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(fruitX + fruitSize, fruitY);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(fruitX + fruitSize, fruitY + fruitSize);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(fruitX, fruitY + fruitSize);
    glEnd();
    
    texture->release();
    
    // 特殊元素标记
    if (fruit.special != SpecialType::NONE) {
        glDisable(GL_TEXTURE_2D);
        
        switch (fruit.special) {
            case SpecialType::LINE_H:
            case SpecialType::LINE_V:
                glColor4f(1.0f, 0.8f, 0.0f, 0.8f);
                break;
            case SpecialType::DIAMOND:
                glColor4f(0.0f, 0.8f, 1.0f, 0.8f);
                break;
            case SpecialType::RAINBOW:
                glColor4f(1.0f, 0.0f, 1.0f, 0.8f);
                break;
            default:
                break;
        }
        
        glLineWidth(4.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + ctx.cellSize, y);
            glVertex2f(x + ctx.cellSize, y + ctx.cellSize);
            glVertex2f(x, y + ctx.cellSize);
        glEnd();
    }
}
