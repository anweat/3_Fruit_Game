#include "ScoreFloatOverlay.h"
#include <QPainter>
#include <QFont>
#include <cmath>

ScoreFloatOverlay::ScoreFloatOverlay(QWidget* parent)
    : QWidget(parent)
{
    // 设置透明背景，不接收鼠标事件（穿透到下层）
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    
    // 预分配空间
    floatingScores_.reserve(MAX_FLOATING_SCORES);
    
    // 独立动画定时器（约 60 FPS）
    animTimer_ = new QTimer(this);
    connect(animTimer_, &QTimer::timeout, this, &ScoreFloatOverlay::onAnimationTick);
    animTimer_->start(16);
}

ScoreFloatOverlay::~ScoreFloatOverlay()
{
}

void ScoreFloatOverlay::addScore(int score, int combo, float centerX, float centerY)
{
    if (score <= 0) return;
    
    // 🔧 计算当前活跃的分数数量，用于堆叠偏移
    int activeCount = 0;
    for (const auto& fs : floatingScores_) {
        if (fs.active) {
            activeCount++;
        }
    }
    
    // 查找空闲槽位
    int slot = -1;
    for (size_t i = 0; i < floatingScores_.size(); ++i) {
        if (!floatingScores_[i].active) {
            slot = static_cast<int>(i);
            break;
        }
    }
    
    if (slot < 0) {
        if (floatingScores_.size() < MAX_FLOATING_SCORES) {
            floatingScores_.push_back(FloatingScoreItem());
            slot = static_cast<int>(floatingScores_.size() - 1);
        } else {
            // 找进度最大的覆盖
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
        floatingScores_[slot].stackIndex = activeCount;  // 🔧 设置堆叠索引
        floatingScores_[slot].active = true;
    }
}

void ScoreFloatOverlay::clear()
{
    for (auto& fs : floatingScores_) {
        fs.active = false;
    }
}

void ScoreFloatOverlay::onAnimationTick()
{
    bool hasActive = false;
    bool hadActiveLastFrame = false;
    float deltaProgress = 0.016f / ANIMATION_DURATION;  // 16ms / 1500ms
    
    for (auto& fs : floatingScores_) {
        if (fs.active) {
            hadActiveLastFrame = true;
            fs.progress += deltaProgress;
            if (fs.progress >= 1.0f) {
                fs.active = false;
            } else {
                hasActive = true;
            }
        }
    }
    
    // 🔧 关键修复：如果上一帧有活跃的分数（即使现在全部结束），也要重绘以清除残留
    if (hasActive || hadActiveLastFrame) {
        update();
    }
}

void ScoreFloatOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    // 🔧 关键修复：使用eraseRect清除背景，比CompositionMode_Clear更可靠
    QPainter painter(this);
    painter.eraseRect(rect());
    
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    
    for (const auto& fs : floatingScores_) {
        if (!fs.active) continue;
        
        // 计算上浮偏移 + 堆叠偏移（让多个分数形成一长串）
        float offsetY = -FLOAT_DISTANCE * fs.progress - STACK_SPACING * fs.stackIndex;
        
        // 计算透明度（后半段淡出）
        float alpha = 1.0f;
        if (fs.progress > 0.5f) {
            alpha = 1.0f - (fs.progress - 0.5f) * 2.0f;
        }
        alpha = std::max(0.0f, std::min(1.0f, alpha));
        
        // 获取颜色和字体大小
        QColor color = getScoreColor(fs.score, fs.combo);
        color.setAlphaF(alpha);
        
        int fontSize = getFontSize(fs.score, fs.combo);
        
        // 缩放效果
        float scale = 1.0f;
        if (fs.progress < 0.2f) {
            scale = 1.0f + 0.3f * (1.0f - fs.progress / 0.2f);
        }
        
        // 设置字体
        QFont font("Microsoft YaHei", static_cast<int>(fontSize * scale));
        font.setBold(true);
        painter.setFont(font);
        
        // 构建文本
        QString text = QString("+%1").arg(fs.score);
        if (fs.combo >= 2) {
            text += QString(" x%1").arg(fs.combo);
        }
        
        // 计算位置
        QFontMetrics fm(font);
        int textWidth = fm.horizontalAdvance(text);
        
        float x = fs.centerX - textWidth / 2.0f;
        float y = fs.centerY + offsetY;
        
        // 绘制描边（高分或连击时）
        if (fs.score >= 100 || fs.combo >= 2) {
            QColor outlineColor(0, 0, 0, static_cast<int>(200 * alpha));
            painter.setPen(outlineColor);
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (dx != 0 || dy != 0) {
                        painter.drawText(static_cast<int>(x + dx), static_cast<int>(y + dy), text);
                    }
                }
            }
        } else {
            // 普通分数只绘制阴影
            QColor shadowColor(0, 0, 0, static_cast<int>(150 * alpha));
            painter.setPen(shadowColor);
            painter.drawText(static_cast<int>(x + 2), static_cast<int>(y + 2), text);
        }
        
        // 绘制主文本
        painter.setPen(color);
        painter.drawText(static_cast<int>(x), static_cast<int>(y), text);
    }
}

QColor ScoreFloatOverlay::getScoreColor(int score, int combo) const
{
    if (combo >= 5 || score >= 1000) {
        return QColor(200, 100, 255);  // 紫色
    } else if (combo >= 4 || score >= 500) {
        return QColor(255, 80, 80);    // 红色
    } else if (combo >= 3 || score >= 300) {
        return QColor(255, 165, 0);    // 橙色
    } else if (combo >= 2 || score >= 150) {
        return QColor(255, 215, 0);    // 金黄
    } else if (score >= 80) {
        return QColor(255, 255, 100);  // 浅黄
    } else {
        return QColor(255, 255, 255);  // 白色
    }
}

int ScoreFloatOverlay::getFontSize(int score, int combo) const
{
    int baseSize = 18;
    
    if (score >= 500 || combo >= 4) {
        return baseSize + 10;
    } else if (score >= 300 || combo >= 3) {
        return baseSize + 6;
    } else if (score >= 150 || combo >= 2) {
        return baseSize + 3;
    } else {
        return baseSize;
    }
}
