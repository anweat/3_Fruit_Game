#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>

/**
 * @brief 构造函数
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUi();
    connectSignals();
}

/**
 * @brief 析构函数
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief 初始化UI组件
 */
void MainWindow::setupUi()
{
    // 设置窗口标题和大小
    setWindowTitle("水果消消乐 - Fruit Crush");
    setMinimumSize(800, 600);
    
    qDebug() << "MainWindow initialized";
}

/**
 * @brief 连接信号和槽
 */
void MainWindow::connectSignals()
{
    // TODO: 连接各个按钮的信号与槽
}

/**
 * @brief 显示主菜单
 */
void MainWindow::showMainMenu()
{
    qDebug() << "Show Main Menu";
    // TODO: 实现主菜单显示逻辑
}

/**
 * @brief 开始休闲模式游戏
 */
void MainWindow::startCasualMode()
{
    qDebug() << "Start Casual Mode";
    // TODO: 实现休闲模式启动逻辑
}

/**
 * @brief 开始比赛模式游戏
 */
void MainWindow::startCompetitionMode()
{
    qDebug() << "Start Competition Mode";
    // TODO: 实现比赛模式启动逻辑
}

/**
 * @brief 显示排行榜
 */
void MainWindow::showLeaderboard()
{
    qDebug() << "Show Leaderboard";
    // TODO: 实现排行榜显示逻辑
}

/**
 * @brief 显示成就页面
 */
void MainWindow::showAchievements()
{
    qDebug() << "Show Achievements";
    // TODO: 实现成就页面显示逻辑
}
