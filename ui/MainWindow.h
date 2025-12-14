#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GameEngine.h"
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include "views/GameView.h"
#include "views/AchievementNotificationWidget.h"
#include "LoginWidget.h"
#include "AchievementDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief 主窗口类
 * 
 * 游戏的主窗口，管理所有子视图的切换和显示
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~MainWindow();

private slots:
    /**
     * @brief 显示登录界面
     */
    void showLoginScreen();
    
    /**
     * @brief 显示主菜单
     */
    void showMainMenu();
    
    /**
     * @brief 开始休闲模式游戏
     */
    void startCasualMode();
    
    /**
     * @brief 开始比赛模式游戏
     */
    void startCompetitionMode();
    
    /**
     * @brief 显示排行榜
     */
    void showLeaderboard();
    
    /**
     * @brief 显示成就页面
     */
    void showAchievements();
    
    /**
     * @brief 道具按钮点击 - 锤子
     */
    void onHammerClicked();
    
    /**
     * @brief 道具按钮点击 - 夹子
     */
    void onClampClicked();
    
    /**
     * @brief 道具按钮点击 - 魔法棒
     */
    void onMagicWandClicked();
    
    /**
     * @brief 更新道具数量显示
     */
    void updatePropCounts();

private:
    Ui::MainWindow *ui;
    
    // 登录界面
    LoginWidget* loginWidget_;
    QString currentPlayerId_;      // 当前玩家ID
    QString currentPlayerName_;    // 当前玩家名称
    
    // 游戏引擎
    GameEngine* gameEngine_;
    
    // 游戏测试界面组件（控制台模式）
    QWidget* gameTestWidget_;
    QTextEdit* gameOutputText_;
    QPushButton* testSwapButton_;
    QPushButton* backToMenuButton_;
    
    // OpenGL游戏视图
    GameView* gameView_;
    QWidget* gameViewWidget_;
    QLabel* scoreLabel_;  // 分数标签
    
    // 道具按钮
    QPushButton* hammerButton_;     // 锤子按钮
    QPushButton* clampButton_;      // 夹子按钮
    QPushButton* magicWandButton_;  // 魔法棒按钮
    QLabel* hammerCountLabel_;      // 锤子数量标签
    QLabel* clampCountLabel_;       // 夹子数量标签
    QLabel* magicWandCountLabel_;   // 魔法棒数量标签
    
    // 成就通知组件
    AchievementNotificationWidget* achievementNotification_;
    
    /**
     * @brief 初始化UI组件
     */
    void setupUi();
    
    /**
     * @brief 连接信号和槽
     */
    void connectSignals();
    
    /**
     * @brief 创建游戏测试界面
     */
    void createGameTestWidget();
    
    /**
     * @brief 创建OpenGL游戏视图
     */
    void createGameViewWidget();
    
    /**
     * @brief 显示游戏地图
     */
    void displayGameMap();
    
    /**
     * @brief 测试交换操作
     */
    void testSwap();
    
    /**
     * @brief 返回主菜单
     */
    void backToMenu();
    
    /**
     * @brief 初始化成就系统（用于当前玩家）
     */
    void initAchievementSystemForPlayer(const QString& playerId);
};

#endif // MAINWINDOW_H
