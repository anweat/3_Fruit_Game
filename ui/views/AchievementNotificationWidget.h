#ifndef ACHIEVEMENTNOTIFICATIONWIDGET_H
#define ACHIEVEMENTNOTIFICATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include "../../src/achievement/AchievementManager.h"

/**
 * @brief 成就通知弹窗 - 独立维护的UI渲染系统
 * 
 * 架构设计：
 * - 与成就系统完全解耦
 * - 独立维护通知队列
 * - 单独的渲染周期
 * - 线程安全的通知入队
 */
class AchievementNotificationWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal windowOpacity READ windowOpacity WRITE setWindowOpacity)

public:
    explicit AchievementNotificationWidget(QWidget *parent = nullptr);
    ~AchievementNotificationWidget();

    // 公开接口：入队一个通知（线程安全）
    void enqueueNotification(const AchievementNotification& notification);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    // 内部槽：处理队列中的通知
    void processQueue();
    void onSlideInFinished();
    void onDisplayTimeout();
    void onFadeOutFinished();

private:
    // 初始化
    void setupUI();
    void setupAnimations();
    
    // 显示流程
    void showCurrentNotification();
    void startSlideIn();
    void startFadeOut();
    
    // 辅助方法
    QString getRarityColor(AchievementRarity rarity);
    QString getRarityName(AchievementRarity rarity);
    QString getRarityIcon(AchievementRarity rarity);
    void updateLabelStyles();

    // ===== UI 组件 =====
    QLabel* iconLabel_;
    QLabel* titleLabel_;
    QLabel* rarityLabel_;
    QLabel* rewardLabel_;

    // ===== 动画 =====
    QPropertyAnimation* slideAnimation_;
    QPropertyAnimation* fadeAnimation_;
    QTimer* displayTimer_;

    // ===== 通知队列与状态 =====
    QQueue<AchievementNotification> notificationQueue_;
    AchievementNotification currentDisplaying_;  // 当前正在显示的
    bool isAnimating_;
    QMutex queueMutex_;  // 线程安全的队列锁

    // ===== 常量配置 =====
    static constexpr int WIDGET_WIDTH = 350;
    static constexpr int WIDGET_HEIGHT = 120;
    static constexpr int DISPLAY_DURATION = 3000;
    static constexpr int SLIDE_DURATION = 500;
    static constexpr int FADE_DURATION = 500;
};

#endif // ACHIEVEMENTNOTIFICATIONWIDGET_H
