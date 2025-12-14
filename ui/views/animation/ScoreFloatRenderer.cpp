#include "ScoreFloatRenderer.h"
#include <QFont>
#include <cmath>

ScoreFloatRenderer::ScoreFloatRenderer()
{
    floatingScores_.reserve(MAX_FLOATING_SCORES);
}

ScoreFloatRenderer::~ScoreFloatRenderer()
{
}

void ScoreFloatRenderer::addScore(int score, int combo, float centerX, float centerY)
{
    if (score <= 0) return;
    
    // 查找空闲槽位或覆盖最旧的
    int slot = -1;
    for (size_t i = 0; i < floatingScores_.size(); ++i) {
        if (!floatingScores_[i].active) {
            slot = static_cast<int>(i);
            break;
        }
    }
    
    if (slot < 0) {
        if (floatingScores_.size() < MAX_FLOATING_SCORES) {
            floatingScores_.push_back(FloatingScore());
            slot = static_cast<int>(floatingScores_.size() - 1);
        } else {
            // 找进度最大的（即将消失的）覆盖
            float maxProgress = -1.0f;
            for (size_t i = 0; i < floatingScores_.size(); ++i) {
                if (floatingScores_[i].progress > maxProgress) {
                    maxProgress = floatingScores_[i].progress;
                    slot = static_cast<int>(i);
                }
            }
        }
    }
    
    if (slot >= 0) {
        floatingScores_[slot].score = score;
        floatingScores_[slot].combo = combo;
        floatingScores_[slot].progress = 0.0f;
        floatingScores_[slot].centerX = centerX;
        floatingScores_[slot].centerY = centerY;
        floatingScores_[slot].active = true;
    }
}

void ScoreFloatRenderer::update(float deltaProgress)
{
    for (auto& fs : floatingScores_) {
        if (fs.active) {
            fs.progress += deltaProgress;
            if (fs.progress >= 1.0f) {
                fs.active = false;
            }
        }
    }
}

void ScoreFloatRenderer::render(QPainter& painter, int widgetWidth, int widgetHeight)
{
    Q_UNUSED(widgetWidth);
    Q_UNUSED(widgetHeight);
    
    for (const auto& fs : floatingScores_) {
        if (!fs.active) continue;
        
        // 计算位置：上浮效果
        float floatDistance = 60.0f;  // 上浮距离（像素）
        float offsetY = -floatDistance * fs.progress;
        
        // 计算透明度：后半段淡出
        float alpha = 1.0f;
        if (fs.progress > 0.5f) {
            alpha = 1.0f - (fs.progress - 0.5f) * 2.0f;
        }
        alpha = std::max(0.0f, std::min(1.0f, alpha));
        
        // 获取颜色和字体大小
        QColor color = getScoreColor(fs.score, fs.combo);
        color.setAlphaF(alpha);
        
        int fontSize = getFontSize(fs.score, fs.combo);
        
        // 缩放效果：开始时放大，然后缩小到正常
        float scale = 1.0f;
        if (fs.progress < 0.2f) {
            scale = 1.0f + 0.3f * (1.0f - fs.progress / 0.2f);  // 1.3 → 1.0
        }
        
        // 设置字体
        QFont font("Microsoft YaHei", static_cast<int>(fontSize * scale));
        font.setBold(true);
        painter.setFont(font);
        
        // 构建显示文本
        QString text = QString("+%1").arg(fs.score);
        if (fs.combo >= 2) {
            text += QString(" x%1").arg(fs.combo);
        }
        
        // 计算文本位置
        QFontMetrics fm(font);
        int textWidth = fm.horizontalAdvance(text);
        int textHeight = fm.height();
        
        float x = fs.centerX - textWidth / 2.0f;
        float y = fs.centerY + offsetY;
        
        // 绘制阴影
        QColor shadowColor(0, 0, 0, static_cast<int>(150 * alpha));
        painter.setPen(shadowColor);
        painter.drawText(static_cast<int>(x + 2), static_cast<int>(y + 2), text);
        
        // 绘制主文本
        painter.setPen(color);
        painter.drawText(static_cast<int>(x), static_cast<int>(y), text);
        
        // 绘制描边效果（高分时）
        if (fs.score >= 200 || fs.combo >= 3) {
            QPen outlinePen(QColor(255, 255, 255, static_cast<int>(100 * alpha)));
            outlinePen.setWidth(2);
            painter.setPen(outlinePen);
            
            // 简单的描边：四个方向偏移绘制
            for (int dx = -1; dx <= 1; dx += 2) {
                for (int dy = -1; dy <= 1; dy += 2) {
                    painter.drawText(static_cast<int>(x + dx), static_cast<int>(y + dy), text);
                }
            }
            
            // 再绘制一次主颜色覆盖
            painter.setPen(color);
            painter.drawText(static_cast<int>(x), static_cast<int>(y), text);
        }
    }
}

void ScoreFloatRenderer::clear()
{
    for (auto& fs : floatingScores_) {
        fs.active = false;
    }
}

bool ScoreFloatRenderer::hasActiveScores() const
{
    for (const auto& fs : floatingScores_) {
        if (fs.active) return true;
    }
    return false;
}

QColor ScoreFloatRenderer::getScoreColor(int score, int combo) const
{
    // 基于分数和连击的颜色方案
    // 普通: 白色 → 黄色 → 橙色 → 红色 → 紫色 → 彩虹
    
    if (combo >= 5 || score >= 1000) {
        // 彩虹/紫色（超高分）
        return QColor(200, 100, 255);  // 紫色
    } else if (combo >= 4 || score >= 500) {
        // 红色（高分）
        return QColor(255, 80, 80);
    } else if (combo >= 3 || score >= 300) {
        // 橙色
        return QColor(255, 165, 0);
    } else if (combo >= 2 || score >= 150) {
        // 金黄色
        return QColor(255, 215, 0);
    } else if (score >= 80) {
        // 浅黄色
        return QColor(255, 255, 100);
    } else {
        // 白色（普通分数）
        return QColor(255, 255, 255);
    }
}

int ScoreFloatRenderer::getFontSize(int score, int combo) const
{
    // 基础字号
    int baseSize = 18;
    
    // 根据分数增加字号
    if (score >= 500 || combo >= 4) {
        return baseSize + 10;  // 28
    } else if (score >= 300 || combo >= 3) {
        return baseSize + 6;   // 24
    } else if (score >= 150 || combo >= 2) {
        return baseSize + 3;   // 21
    } else {
        return baseSize;       // 18
    }
}
