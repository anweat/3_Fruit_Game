#include "IAnimationRenderer.h"
#include <QOpenGLTexture>

void IAnimationRenderer::drawFruit(
    int row, int col,
    const Fruit& fruit,
    float offsetX, float offsetY,
    float alpha,
    float scale,
    float gridStartX, float gridStartY, float cellSize,
    const std::vector<QOpenGLTexture*>& textures)
{
    if (fruit.type == FruitType::EMPTY) {
        return;
    }
    
    // 计算位置
    float x = gridStartX + col * cellSize + offsetX;
    float y = gridStartY + row * cellSize + offsetY;
    
    // 如果有缩放，从中心缩放
    if (scale != 1.0f) {
        float centerX = x + cellSize * 0.5f;
        float centerY = y + cellSize * 0.5f;
        float scaledSize = cellSize * scale;
        x = centerX - scaledSize * 0.5f;
        y = centerY - scaledSize * 0.5f;
        cellSize = scaledSize;
    }
    
    // 获取纹理索引
    int textureIndex = static_cast<int>(fruit.type);
    if (fruit.type == FruitType::CANDY) {
        textureIndex = 6;
    }
    
    if (textureIndex < 0 || textureIndex >= static_cast<int>(textures.size())) {
        return;
    }
    
    QOpenGLTexture* texture = textures[textureIndex];
    if (!texture) {
        return;
    }
    
    // 绘制水果纹理
    glEnable(GL_TEXTURE_2D);
    texture->bind();
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    
    float padding = cellSize * 0.1f;
    float fruitSize = cellSize - padding * 2;
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
                glColor4f(1.0f, 0.8f, 0.0f, 0.8f * alpha); // 金色
                break;
            case SpecialType::DIAMOND:
                glColor4f(0.0f, 0.8f, 1.0f, 0.8f * alpha); // 青色
                break;
            case SpecialType::RAINBOW:
                glColor4f(1.0f, 0.0f, 1.0f, 0.8f * alpha); // 紫色
                break;
            default:
                break;
        }
        
        float borderSize = 4.0f;
        glLineWidth(borderSize);
        glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + cellSize, y);
            glVertex2f(x + cellSize, y + cellSize);
            glVertex2f(x, y + cellSize);
        glEnd();
    }
}

void IAnimationRenderer::drawQuad(
    float x, float y, float width, float height)
{
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
    glEnd();
}
