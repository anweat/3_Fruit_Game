#include "SwapEffect.h"
#include <QOpenGLFunctions>
#include <cmath>

SwapEffect::SwapEffect(int row1, int col1, int row2, int col2,
                       const Fruit& fruit1, const Fruit& fruit2, bool success)
    : row1_(row1), col1_(col1), row2_(row2), col2_(col2)
    , fruit1_(fruit1), fruit2_(fruit2), success_(success)
{
}

void SwapEffect::render(const AnimationContext& ctx, float progress)
{
    float t = progress;
    if (!success_) {
        // 回弹：前半程出去，后半程回来
        t = (t <= 0.5f) ? (t * 2.0f) : ((1.0f - t) * 2.0f);
    }
    
    // 计算位移方向
    int dirRow = row2_ - row1_;
    int dirCol = col2_ - col1_;
    float dx = dirCol * ctx.cellSize * t;
    float dy = dirRow * ctx.cellSize * t;
    
    // 绘制第一个水果（从位置1向位置2移动）
    if (fruit1_.type != FruitType::EMPTY) {
        drawFruitWithOffset(ctx, row1_, col1_, fruit1_, dx, dy);
    }
    
    // 绘制第二个水果（从位置2向位置1移动）
    if (fruit2_.type != FruitType::EMPTY) {
        drawFruitWithOffset(ctx, row2_, col2_, fruit2_, -dx, -dy);
    }
}

void SwapEffect::drawFruitWithOffset(const AnimationContext& ctx,
                                     int row, int col, const Fruit& fruit,
                                     float offsetX, float offsetY)
{
    if (fruit.type == FruitType::EMPTY) return;
    if (!ctx.textures) return;
    
    float x = ctx.cellX(col) + offsetX;
    float y = ctx.cellY(row) + offsetY;
    
    // 获取纹理索引
    int textureIndex = static_cast<int>(fruit.type);
    if (fruit.type == FruitType::CANDY) {
        textureIndex = 6;
    }
    
    if (textureIndex < 0 || textureIndex >= static_cast<int>(ctx.textures->size())) {
        return;
    }
    
    QOpenGLTexture* texture = (*ctx.textures)[textureIndex];
    if (!texture) return;
    
    // 绘制水果纹理
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
    
    // 如果有特殊属性，绘制标记
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
