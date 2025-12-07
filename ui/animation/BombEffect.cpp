#include "BombEffect.h"
#include <QOpenGLFunctions>

BombAnimEffect::BombAnimEffect(const std::vector<BombAnimData>& effects)
    : effects_(effects)
{
}

void BombAnimEffect::render(const AnimationContext& ctx, float progress)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    
    for (const auto& effect : effects_) {
        switch (effect.type) {
            case BombEffectType::LINE_H:
                renderLineH(ctx, effect.row, progress);
                break;
            case BombEffectType::LINE_V:
                renderLineV(ctx, effect.col, progress);
                break;
            case BombEffectType::DIAMOND:
                renderDiamond(ctx, effect.row, effect.col, effect.range, progress);
                break;
            case BombEffectType::RAINBOW:
                renderRainbow(ctx, progress);
                break;
            default:
                break;
        }
    }
}

void BombAnimEffect::renderLineH(const AnimationContext& ctx, int row, float progress)
{
    float rowY = ctx.cellY(row);
    float startX = ctx.gridStartX;
    float fullWidth = ctx.cellSize * ctx.mapSize;
    float centerY = rowY + ctx.cellSize * 0.5f;
    
    // 高度从 cellSize 缩小到 0
    float height = ctx.cellSize * (1.0f - progress);
    // 透明度从 0.8 淡化到 0
    float alpha = 0.8f * (1.0f - progress);
    
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
        glVertex2f(startX, centerY - height * 0.5f);
        glVertex2f(startX + fullWidth, centerY - height * 0.5f);
        glVertex2f(startX + fullWidth, centerY + height * 0.5f);
        glVertex2f(startX, centerY + height * 0.5f);
    glEnd();
}

void BombAnimEffect::renderLineV(const AnimationContext& ctx, int col, float progress)
{
    float colX = ctx.cellX(col);
    float startY = ctx.gridStartY;
    float fullHeight = ctx.cellSize * ctx.mapSize;
    float centerX = colX + ctx.cellSize * 0.5f;
    
    // 宽度从 cellSize 缩小到 0
    float width = ctx.cellSize * (1.0f - progress);
    // 透明度从 0.8 淡化到 0
    float alpha = 0.8f * (1.0f - progress);
    
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
        glVertex2f(centerX - width * 0.5f, startY);
        glVertex2f(centerX + width * 0.5f, startY);
        glVertex2f(centerX + width * 0.5f, startY + fullHeight);
        glVertex2f(centerX - width * 0.5f, startY + fullHeight);
    glEnd();
}

void BombAnimEffect::renderDiamond(const AnimationContext& ctx, int row, int col, int range, float progress)
{
    float centerX = ctx.cellCenterX(col);
    float centerY = ctx.cellCenterY(row);
    
    // 大小从 1 格子放大到 5×5 范围
    float maxSize = ctx.cellSize * (range * 2 + 1);
    float size = ctx.cellSize + (maxSize - ctx.cellSize) * progress;
    // 透明度从 0.6 淡化到 0
    float alpha = 0.6f * (1.0f - progress);
    
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
        glVertex2f(centerX - size * 0.5f, centerY - size * 0.5f);
        glVertex2f(centerX + size * 0.5f, centerY - size * 0.5f);
        glVertex2f(centerX + size * 0.5f, centerY + size * 0.5f);
        glVertex2f(centerX - size * 0.5f, centerY + size * 0.5f);
    glEnd();
}

void BombAnimEffect::renderRainbow(const AnimationContext& ctx, float progress)
{
    float startX = ctx.gridStartX;
    float startY = ctx.gridStartY;
    float fullSize = ctx.cellSize * ctx.mapSize;
    // 透明度从 0.5 淡化到 0
    float alpha = 0.5f * (1.0f - progress);
    
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
        glVertex2f(startX, startY);
        glVertex2f(startX + fullSize, startY);
        glVertex2f(startX + fullSize, startY + fullSize);
        glVertex2f(startX, startY + fullSize);
    glEnd();
}
