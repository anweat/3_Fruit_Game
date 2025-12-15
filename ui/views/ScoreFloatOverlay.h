#ifndef SCOREFLOATOVERLAY_H
#define SCOREFLOATOVERLAY_H

#include <QWidget>
#include <QTimer>
#include <vector>
#include <queue>
#include <QColor>

/**
 * @brief 单个浮动分数项
 */
struct FloatingScoreItem {
    int score = 0;          ///< 分数值
    int combo = 0;          ///< 连击数
    float progress = 0.0f;  ///< 动画进度 [0, 1]
    float centerX = 0.0f;   ///< 显示中心X（固定在顶部）
    float centerY = 0.0f;   ///< 显示中心Y（固定在顶部）
    int stackIndex = 0;     ///< 堆叠索引（用于垂直偏移）
    bool active = false;    ///< 是否激活
};

/**
 * @brief 待显示分数队列项
 */
struct PendingScore {
    int score;
    int combo;
};

/**
 * @brief 分数浮动显示覆盖层
 * 
 * 独立的透明 Widget 覆盖在 GameView 上方，使用队列系统每 0.1 秒显示一个分数。
 * 
 * 改进：
 * - 所有分数固定在地图顶部生成
 * - 不显示"x连击数"标签
 * - 改为显示"连击个数×分数"
 * - 按队列每 0.1 秒加入一个新分数
 */
class ScoreFloatOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit ScoreFloatOverlay(QWidget* parent = nullptr);
    ~ScoreFloatOverlay();
    
    /**
     * @brief 设置地图信息（用于计算固定生成位置）
     * @param mapSize 地图大小（8×8）
     * @param gridStartY 网格顶部Y坐标
     * @param cellSize 格子大小
     */
    void setMapInfo(int mapSize, float gridStartY, float cellSize);
    
    /**
     * @brief 添加一个浮动分数到队列
     * @param score 分数值
     * @param combo 连击数
     */
    void addScore(int score, int combo);
    
    /**
     * @brief 清除所有浮动分数
     */
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onAnimationTick();
    void onQueueTick();

private:
    QColor getScoreColor(int score, int combo) const;
    int getFontSize(int score, int combo) const;
    
    std::vector<FloatingScoreItem> floatingScores_;
    std::queue<PendingScore> pendingScores_;  ///< 待显示的分数队列
    QTimer* animTimer_;
    QTimer* queueTimer_;  ///< 0.1 秒间隔的队列定时器
    
    // 地图信息
    int mapSize_ = 8;
    float gridStartY_ = 0.0f;
    float cellSize_ = 40.0f;
    float displayCenterX_ = 400.0f;  ///< 固定显示中心X
    
    static const int MAX_FLOATING_SCORES = 20;  // 增加到20以显示更多浮动分数
    static constexpr float ANIMATION_DURATION = 1.5f;  ///< 动画持续时间（秒）
    static constexpr float FLOAT_DISTANCE = 80.0f;     ///< 上浮距离（像素）
    static constexpr float STACK_SPACING = 30.0f;      ///< 堆叠间隔（像素）
    static constexpr float QUEUE_INTERVAL = 0.2f;      ///< 队列间隔（秒）
};

#endif // SCOREFLOATOVERLAY_H
