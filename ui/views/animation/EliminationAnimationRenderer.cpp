#include "EliminationAnimationRenderer.h"
#include <QOpenGLTexture>

EliminationAnimationRenderer::EliminationAnimationRenderer()
{
}

EliminationAnimationRenderer::~EliminationAnimationRenderer()
{
}

void EliminationAnimationRenderer::render(
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
    
    const EliminationStep& step = animSeq.rounds[roundIndex].elimination;
    
    // 绘制消除效果
    renderElimination(step, progress, snapshot, gridStartX, gridStartY, cellSize, textures);
    
    // 绘制炸弹特效
    renderBombEffects(step, progress, gridStartX, gridStartY, cellSize);
}

void EliminationAnimationRenderer::renderElimination(
    const EliminationStep& step,
    float progress,
    const std::vector<std::vector<Fruit>>& snapshot,
    float gridStartX, float gridStartY, float cellSize,
    const std::vector<QOpenGLTexture*>& textures)
{
    if (step.positions.empty()) {
        return;
    }
    
    float scale = 1.0f - progress;
    float alpha = 1.0f - progress;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (const auto& pos : step.positions) {
        int row = pos.first;
        int col = pos.second;
        if (row < 0 || row >= MAP_SIZE || col < 0 || col >= MAP_SIZE) {
            continue;
        }
        
        const Fruit& fruit = snapshot[row][col];
        if (fruit.type == FruitType::EMPTY) {
            continue;
        }
        
        float cellX = gridStartX + col * cellSize;
        float cellY = gridStartY + row * cellSize;
        float centerX = cellX + cellSize * 0.5f;
        float centerY = cellY + cellSize * 0.5f;
        
        float size = cellSize * scale;
        float x = centerX - size * 0.5f;
        float y = centerY - size * 0.5f;
        
        // 绘制缩小的水果纹理
        int textureIndex = static_cast<int>(fruit.type);
        if (fruit.type == FruitType::CANDY) {
            textureIndex = 6;
        }
        
        if (textureIndex >= 0 && textureIndex < static_cast<int>(textures.size()) && textures[textureIndex]) {
            glEnable(GL_TEXTURE_2D);
            textures[textureIndex]->bind();
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
            
            textures[textureIndex]->release();
        }
    }
}

void EliminationAnimationRenderer::renderBombEffects(
    const EliminationStep& step,
    float progress,
    float gridStartX, float gridStartY, float cellSize)
{
    if (step.bombEffects.empty()) {
        return;
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    
    for (const auto& effect : step.bombEffects) {
        switch (effect.type) {
            case BombEffectType::LINE_H: {
                // 横排特效：白色长条覆盖整行，逐渐变窄变淡
                float rowY = gridStartY + effect.row * cellSize;
                float startX = gridStartX;
                float fullWidth = cellSize * MAP_SIZE;
                float centerY = rowY + cellSize * 0.5f;
                
                float height = cellSize * (1.0f - progress);
                float alpha = 0.8f * (1.0f - progress);
                
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                glBegin(GL_QUADS);
                    glVertex2f(startX, centerY - height * 0.5f);
                    glVertex2f(startX + fullWidth, centerY - height * 0.5f);
                    glVertex2f(startX + fullWidth, centerY + height * 0.5f);
                    glVertex2f(startX, centerY + height * 0.5f);
                glEnd();
                break;
            }
            
            case BombEffectType::LINE_V: {
                // 竖排特效：白色长条覆盖整列，逐渐变窄变淡
                float colX = gridStartX + effect.col * cellSize;
                float startY = gridStartY;
                float fullHeight = cellSize * MAP_SIZE;
                float centerX = colX + cellSize * 0.5f;
                
                float width = cellSize * (1.0f - progress);
                float alpha = 0.8f * (1.0f - progress);
                
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                glBegin(GL_QUADS);
                    glVertex2f(centerX - width * 0.5f, startY);
                    glVertex2f(centerX + width * 0.5f, startY);
                    glVertex2f(centerX + width * 0.5f, startY + fullHeight);
                    glVertex2f(centerX - width * 0.5f, startY + fullHeight);
                glEnd();
                break;
            }
            
            case BombEffectType::DIAMOND: {
                // 菱形特效：白色正方形从中心放大并变暗
                float centerX = gridStartX + effect.col * cellSize + cellSize * 0.5f;
                float centerY = gridStartY + effect.row * cellSize + cellSize * 0.5f;
                
                float maxSize = cellSize * (effect.range * 2 + 1);
                float size = cellSize + (maxSize - cellSize) * progress;
                float alpha = 0.6f * (1.0f - progress);
                
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                glBegin(GL_QUADS);
                    glVertex2f(centerX - size * 0.5f, centerY - size * 0.5f);
                    glVertex2f(centerX + size * 0.5f, centerY - size * 0.5f);
                    glVertex2f(centerX + size * 0.5f, centerY + size * 0.5f);
                    glVertex2f(centerX - size * 0.5f, centerY + size * 0.5f);
                glEnd();
                break;
            }
            
            case BombEffectType::RAINBOW: {
                // 彩虹特效：全屏白色闪光
                float startX = gridStartX;
                float startY = gridStartY;
                float fullSize = cellSize * MAP_SIZE;
                float alpha = 0.5f * (1.0f - progress);
                
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                glBegin(GL_QUADS);
                    glVertex2f(startX, startY);
                    glVertex2f(startX + fullSize, startY);
                    glVertex2f(startX + fullSize, startY + fullSize);
                    glVertex2f(startX, startY + fullSize);
                glEnd();
                break;
            }
            
            default:
                break;
        }
    }
}
