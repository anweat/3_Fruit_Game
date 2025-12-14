#ifndef SCOREFLOATRENDERER_H
#define SCOREFLOATRENDERER_H

#include <vector>
#include <QColor>
#include <QString>
#include <QPainter>
#include <QOpenGLWidget>

/**
 * @brief 单个浮动分数项
 */
struct FloatingScore {
    int score = 0;          ///< 分数值
    int combo = 0;          ///< 连击数
    float progress = 0.0f;  ///< 动画进度 [0, 1]
    float centerX = 0.0f;   ///< 显示中心X（屏幕坐标）
    float centerY = 0.0f;   ///< 显示中心Y（屏幕坐标）
    bool active = false;    ///< 是否激活
};

/**
 * @brief 分数浮动显示渲染器
 * 
 * 功能：
 * - 在屏幕中上方显示消除得分
 * - 根据分数大小显示不同颜色
 * - 上浮并淡出消失的动画效果
 * - 支持多个分数同时显示（连锁消除）
 */
class ScoreFloatRenderer
{
public:
    ScoreFloatRenderer();
    ~ScoreFloatRenderer();
    
    /**
     * @brief 添加一个浮动分数
     * @param score 分数值
     * @param combo 连击数（影响颜色）
     * @param centerX 显示中心X（屏幕像素坐标）
     * @param centerY 显示中心Y（屏幕像素坐标）
     */
    void addScore(int score, int combo, float centerX, float centerY);
    
    /**
     * @brief 更新所有浮动分数的动画进度
     * @param deltaProgress 进度增量
     */
    void update(float deltaProgress);
    
    /**
     * @brief 渲染所有浮动分数
     * @param painter QPainter 引用
     * @param widgetWidth 窗口宽度
     * @param widgetHeight 窗口高度
     */
    void render(QPainter& painter, int widgetWidth, int widgetHeight);
    
    /**
     * @brief 清除所有浮动分数
     */
    void clear();
    
    /**
     * @brief 检查是否有活跃的浮动分数
     */
    bool hasActiveScores() const;
    
private:
    /**
     * @brief 根据分数和连击获取颜色
     */
    QColor getScoreColor(int score, int combo) const;
    
    /**
     * @brief 根据分数获取字体大小
     */
    int getFontSize(int score, int combo) const;
    
    std::vector<FloatingScore> floatingScores_;
    static const int MAX_FLOATING_SCORES = 10;  ///< 最大同时显示数量
};

#endif // SCOREFLOATRENDERER_H
