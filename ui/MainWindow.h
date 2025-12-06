#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GameEngine.h"
#include <QTextEdit>
#include <QPushButton>

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

private:
    Ui::MainWindow *ui;
    
    // 游戏引擎
    GameEngine* gameEngine_;
    
    // 游戏测试界面组件
    QWidget* gameTestWidget_;
    QTextEdit* gameOutputText_;
    QPushButton* testSwapButton_;
    QPushButton* backToMenuButton_;
    
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
};

#endif // MAINWINDOW_H
