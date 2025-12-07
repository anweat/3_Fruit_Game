#include "EliminateEffect.h"
#include <QOpenGLFunctions>

EliminateEffect::EliminateEffect(const std::vector<Position>& positions)
    : positions_(positions)
{
}

void EliminateEffect::render(const AnimationContext& ctx, float progress)
{
    float scale = 1.0f - progress;
    float alpha = 1.0f - progress;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (const auto& pos : positions_) {
        if (pos.fruit.type == FruitType::EMPTY) continue;
        if (!ctx.textures) continue;
        
        float cellX = ctx.cellX(pos.col);
        float cellY = ctx.cellY(pos.row);
        float centerX = cellX + ctx.cellSize * 0.5f;
        float centerY = cellY + ctx.cellSize * 0.5f;
        
        float size = ctx.cellSize * scale;
        float x = centerX - size * 0.5f;
        float y = centerY - size * 0.5f;
        
        // 获取纹理
        int textureIndex = static_cast<int>(pos.fruit.type);
        if (pos.fruit.type == FruitType::CANDY) {
            textureIndex = 6;
        }
        
        if (textureIndex >= 0 && textureIndex < static_cast<int>(ctx.textures->size())) {
            QOpenGLTexture* texture = (*ctx.textures)[textureIndex];
            if (texture) {
                glEnable(GL_TEXTURE_2D);
                texture->bind();
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                
                float padding = size * 0.1f;
                float fruitSize = size - padding * 2;
                float fruitX = x + padding;
                float fruitY = y + padding;
                
                glBegin(GL_QUADS);
                    glTexCoord2f(0.0f, 0.0f); glVertex2f(fruitX, fruitY);
                    glTexCoord2f(1.0f, 0.0f); glVertex2f(fruitX + fruitSize, fruitY);
                    glTexCoord2f(1.0f, 1.0f); glVertex2f(fruitX + fruitSize, fruitY + fruitSize);
                    glTexCoord2f(0.0f, 1.0f); glVertex2f(fruitX, fruitY + fruitSize);
                glEnd();
                
                texture->release();
            }
        }
    }
}
