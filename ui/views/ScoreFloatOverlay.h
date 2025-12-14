#ifndef SCOREFLOATOVERLAY_H
#define SCOREFLOATOVERLAY_H

#include <QWidget>
#include <QTimer>
#include <vector>
#include <QColor>

/**
 * @brief 单个浮动分数项
 */
struct FloatingScoreItem {
    int score = 0;          ///< 分数值
    int combo = 0;          ///< 连击数
    float progress = 0.0f;  ///< 动画进度 [0, 1]
    float centerX = 0.0f;   ///< 显示中心X（父坐标系）
    float centerY = 0.0f;   ///< 显示中心Y（父坐标系）
    bool active = false;    ///< 是否激活
};

/**
 * @brief 分数浮动显示覆盖层
 * 
 * 独立的透明 Widget 覆盖在 GameView 上方，
 * 使用独立的渲染管线，不影响 OpenGL 渲染性能。
 * 
 * 功能：
 * - 在消除位置显示得分
 * - 根据分数大小显示不同颜色
 * - 上浮并淡出消失的动画效果
 */
class ScoreFloatOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit ScoreFloatOverlay(QWidget* parent = nullptr);
    ~ScoreFloatOverlay();
    
    /**
     * @brief 添加一个浮动分数
     * @param score 分数值
     * @param combo 连击数（影响颜色）
     * @param centerX 显示中心X（相对于父窗口）
     * @param centerY 显示中心Y（相对于父窗口）
     */
    void addScore(int score, int combo, float centerX, float centerY);
    
    /**
     * @brief 清除所有浮动分数
     */
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onAnimationTick();

private:
    QColor getScoreColor(int score, int combo) const;
    int getFontSize(int score, int combo) const;
    
    std::vector<FloatingScoreItem> floatingScores_;
    QTimer* animTimer_;
    
    static const int MAX_FLOATING_SCORES = 10;
    static constexpr float ANIMATION_DURATION = 1.5f;  ///< 动画持续时间（秒）
    static constexpr float FLOAT_DISTANCE = 60.0f;     ///< 上浮距离（像素）
};

#endif // SCOREFLOATOVERLAY_H
