#include "FallEffect.h"
#include <QOpenGLFunctions>

FallEffect::FallEffect(const std::vector<FallMove>& moves, const std::vector<NewFruit>& newFruits)
    : moves_(moves), newFruits_(newFruits)
{
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
    for (const auto& nf : newFruits_) {
        float startY = ctx.gridStartY - ctx.cellSize * 1.5f;
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
