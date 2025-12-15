#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GameEngine.h"
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QTableWidget>
#include "views/GameView.h"
#include "views/AchievementNotificationWidget.h"
#include "LoginWidget.h"
#include "AchievementDialog.h"
#include "../src/mode/CompetitionMode.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief 游戏模式枚举
 */
enum class GameModeType {
    CASUAL,      ///< 休闲模式
    COMPETITION  ///< 比赛模式
};

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
     * @brief 显示设置对话框
     */
    void showSettings();
    
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
    
    /**
     * @brief 购买锤子
     */
    void onBuyHammer();
    
    /**
     * @brief 购买夹子
     */
    void onBuyClamp();
    
    /**
     * @brief 购买魔法棒
     */
    void onBuyMagicWand();
    
    /**
     * @brief 比赛倒计时更新
     */
    void onCompetitionTimeUpdated(int remainingSeconds);
    
    /**
     * @brief 比赛正常结束（记录成绩）
     */
    void onCompetitionEnded();
    
    /**
     * @brief 比赛放弃（不记录成绩）
     */
    void onCompetitionAbandoned();

private:
    Ui::MainWindow *ui;
    
    // 登录界面
    LoginWidget* loginWidget_;
    QString currentPlayerId_;      // 当前玩家ID
    QString currentPlayerName_;    // 当前玩家名称
    
    // 游戏引擎
    GameEngine* gameEngine_;
    
    // 当前游戏模式
    GameModeType currentGameMode_;
    
    // 比赛模式管理
    CompetitionMode* competitionMode_;
    CompetitionDuration currentCompetitionDuration_;
    
    // 游戏测试界面组件（控制台模式）
    QWidget* gameTestWidget_;
    QTextEdit* gameOutputText_;
    QPushButton* testSwapButton_;
    QPushButton* backToMenuButton_;
    
    // 比赛模式选择界面
    QWidget* competitionSelectWidget_;
    
    // 比赛结束界面
    QWidget* competitionEndWidget_;
    QLabel* endScoreLabel_;
    QLabel* endComboLabel_;
    QLabel* endRankLabel_;
    QLabel* endMessageLabel_;
    
    // 排行榜界面
    QWidget* leaderboardWidget_;
    QTabWidget* leaderboardTabWidget_;
    QTableWidget* leaderboard60Table_;
    QTableWidget* leaderboard120Table_;
    QTableWidget* leaderboard180Table_;
    
    // 休闲模式游戏视图
    GameView* casualGameView_;
    QWidget* casualGameViewWidget_;
    QLabel* casualScoreLabel_;
    
    // 比赛模式游戏视图
    GameView* competitionGameView_;
    QWidget* competitionGameViewWidget_;
    QLabel* competitionScoreLabel_;
    QLabel* timerLabel_;  // 倒计时标签（比赛模式）
    
    // 休闲模式道具按钮
    QPushButton* casualHammerButton_;
    QPushButton* casualClampButton_;
    QPushButton* casualMagicWandButton_;
    QLabel* casualHammerCountLabel_;
    QLabel* casualClampCountLabel_;
    QLabel* casualMagicWandCountLabel_;
    QPushButton* casualBuyHammerButton_;
    QPushButton* casualBuyClampButton_;
    QPushButton* casualBuyMagicWandButton_;
    
    // 比赛模式道具按钮
    QPushButton* compHammerButton_;
    QPushButton* compClampButton_;
    QPushButton* compMagicWandButton_;
    QLabel* compHammerCountLabel_;
    QLabel* compClampCountLabel_;
    QLabel* compMagicWandCountLabel_;
    
    // 道具价格常量
    static constexpr int HAMMER_PRICE = 200;
    static constexpr int CLAMP_PRICE = 200;
    static constexpr int MAGIC_WAND_PRICE = 400;
    
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
     * @brief 创建比赛模式选择界面
     */
    void createCompetitionSelectWidget();
    
    /**
     * @brief 创建比赛结束界面
     */
    void createCompetitionEndWidget();
    
    /**
     * @brief 创建排行榜界面
     */
    void createLeaderboardWidget();
    
    /**
     * @brief 刷新排行榜数据
     */
    void refreshLeaderboardData();
    
    /**
     * @brief 创建休闲模式游戏视图
     */
    void createCasualGameViewWidget();
    
    /**
     * @brief 创建比赛模式游戏视图
     */
    void createCompetitionGameViewWidget();
    
    /**
     * @brief 更新休闲模式道具显示
     */
    void updateCasualPropCounts();
    
    /**
     * @brief 更新比赛模式道具显示
     */
    void updateCompetitionPropCounts();
    
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
    
    /**
     * @brief 显示比赛结束界面（嵌入式）
     */
    void showCompetitionEndScreen(int finalScore, int maxCombo);
    
    /**
     * @brief 开始选定时长的比赛
     */
    void startSelectedCompetition(CompetitionDuration duration);
};

#endif // MAINWINDOW_H
