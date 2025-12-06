#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>

/**
 * @brief 构造函数
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , gameEngine_(nullptr)
    , gameTestWidget_(nullptr)
    , gameOutputText_(nullptr)
    , testSwapButton_(nullptr)
    , backToMenuButton_(nullptr)
    , gameView_(nullptr)
    , gameViewWidget_(nullptr)
    , scoreLabel_(nullptr)
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
    if (gameEngine_) {
        delete gameEngine_;
    }
    if (gameTestWidget_) {
        delete gameTestWidget_;
    }
    if (gameViewWidget_) {
        delete gameViewWidget_;
    }
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
    // 连接主菜单按钮
    connect(ui->casualModeButton, &QPushButton::clicked, this, &MainWindow::startCasualMode);
    connect(ui->competitionModeButton, &QPushButton::clicked, this, &MainWindow::startCompetitionMode);
    connect(ui->leaderboardButton, &QPushButton::clicked, this, &MainWindow::showLeaderboard);
    connect(ui->achievementsButton, &QPushButton::clicked, this, &MainWindow::showAchievements);
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
    qDebug() << "Start Casual Mode - OpenGL Rendering";
    
    // 创建游戏引擎
    if (!gameEngine_) {
        gameEngine_ = new GameEngine();
    }
    
    // 初始化游戏
    gameEngine_->initializeGame();
    
    // 创建OpenGL游戏视图
    if (!gameViewWidget_) {
        createGameViewWidget();
    }
    
    // 设置引擎
    gameView_->setGameEngine(gameEngine_);
    
    // 切换到游戏界面
    ui->stackedWidget->addWidget(gameViewWidget_);
    ui->stackedWidget->setCurrentWidget(gameViewWidget_);
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

/**
 * @brief 创建游戏测试界面
 */
void MainWindow::createGameTestWidget()
{
    gameTestWidget_ = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(gameTestWidget_);
    
    // 游戏信息标签
    QLabel* titleLabel = new QLabel("🍎 游戏引擎测试 🍊");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);
    
    // 游戏输出文本框（显示地图和信息）
    gameOutputText_ = new QTextEdit();
    gameOutputText_->setReadOnly(true);
    gameOutputText_->setMinimumHeight(400);
    QFont monoFont("Courier New", 10);
    gameOutputText_->setFont(monoFont);
    layout->addWidget(gameOutputText_);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    testSwapButton_ = new QPushButton("测试交换 (0,0) <-> (0,1)");
    connect(testSwapButton_, &QPushButton::clicked, this, &MainWindow::testSwap);
    buttonLayout->addWidget(testSwapButton_);
    
    backToMenuButton_ = new QPushButton("返回主菜单");
    connect(backToMenuButton_, &QPushButton::clicked, this, &MainWindow::backToMenu);
    buttonLayout->addWidget(backToMenuButton_);
    
    layout->addLayout(buttonLayout);
}

/**
 * @brief 显示游戏地图
 */
void MainWindow::displayGameMap()
{
    if (!gameEngine_ || !gameOutputText_) {
        return;
    }
    
    QString output;
    const auto& map = gameEngine_->getMap();
    
    // 显示游戏状态
    output += "===========================================\n";
    output += QString("🎮 游戏引擎状态\n");
    output += "===========================================\n";
    output += QString("💯 当前分数: %1\n").arg(gameEngine_->getCurrentScore());
    output += QString("🔥 连击数: %1\n").arg(gameEngine_->getComboCount());
    output += QString("✅ 有可移动: %1\n").arg(gameEngine_->hasValidMoves() ? "是" : "否");
    output += "\n";
    
    // 显示地图
    output += "🌎 游戏地图 (8x8):\n";
    output += "   ";
    for (int col = 0; col < MAP_SIZE; col++) {
        output += QString(" %1 ").arg(col);
    }
    output += "\n";
    
    for (int row = 0; row < MAP_SIZE; row++) {
        output += QString(" %1 ").arg(row);
        for (int col = 0; col < MAP_SIZE; col++) {
            const Fruit& fruit = map[row][col];
            QString fruitSymbol;
            
            // 根据水果类型显示符号
            switch (fruit.type) {
                case FruitType::APPLE:      fruitSymbol = "🍎"; break;
                case FruitType::ORANGE:     fruitSymbol = "🍊"; break;
                case FruitType::GRAPE:      fruitSymbol = "🍇"; break;
                case FruitType::BANANA:     fruitSymbol = "🍌"; break;
                case FruitType::WATERMELON: fruitSymbol = "🍉"; break;
                case FruitType::STRAWBERRY: fruitSymbol = "🍓"; break;
                case FruitType::EMPTY:      fruitSymbol = "⬜"; break;
            }
            
            // 如果有特殊属性，添加标记
            if (fruit.special != SpecialType::NONE) {
                switch (fruit.special) {
                    case SpecialType::LINE_H:  fruitSymbol += "H"; break;
                    case SpecialType::LINE_V:  fruitSymbol += "V"; break;
                    case SpecialType::DIAMOND: fruitSymbol += "D"; break;
                    case SpecialType::RAINBOW: fruitSymbol += "R"; break;
                    default: break;
                }
            }
            
            output += fruitSymbol + " ";
        }
        output += "\n";
    }
    
    output += "\n";
    output += "💡 提示: 点击 '测试交换' 按钮来测试游戏引擎\n";
    output += "===========================================\n";
    
    gameOutputText_->setPlainText(output);
    
    // 滚动到顶部
    gameOutputText_->verticalScrollBar()->setValue(0);
}

/**
 * @brief 测试交换操作
 */
void MainWindow::testSwap()
{
    if (!gameEngine_) {
        return;
    }
    
    qDebug() << "Testing swap (0,0) <-> (0,1)";
    
    // 尝试交换 (0,0) 和 (0,1)
    bool success = gameEngine_->swapFruits(0, 0, 0, 1);
    
    QString message;
    if (success) {
        message = QString("✅ 交换成功！当前分数: %1, 连击: %2")
                 .arg(gameEngine_->getCurrentScore())
                 .arg(gameEngine_->getComboCount());
    } else {
        message = "❌ 交换失败！该交换不会产生匹配";
    }
    
    qDebug() << message;
    
    // 重新显示地图
    displayGameMap();
    
    // 在输出框底部显示消息
    gameOutputText_->append("\n" + message);
}

/**
 * @brief 返回主菜单
 */
void MainWindow::backToMenu()
{
    qDebug() << "Back to main menu";
    
    // 切换到主菜单页面
    ui->stackedWidget->setCurrentIndex(0);
}

/**
 * @brief 创建OpenGL游戏视图
 */
void MainWindow::createGameViewWidget()
{
    gameViewWidget_ = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(gameViewWidget_);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建OpenGL游戏视图
    gameView_ = new GameView(gameViewWidget_);
    layout->addWidget(gameView_);
    
    // 添加底部控制栏
    QHBoxLayout* controlLayout = new QHBoxLayout();
    
    // 分数显示
    scoreLabel_ = new QLabel("💯 分数: 0 | 🔥 连击: 0");
    scoreLabel_->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: #FFD700; padding: 10px; }");
    controlLayout->addWidget(scoreLabel_);
    
    controlLayout->addStretch();
    
    // 返回按钮
    QPushButton* backButton = new QPushButton("返回主菜单");
    backButton->setMinimumSize(120, 40);
    connect(backButton, &QPushButton::clicked, this, &MainWindow::backToMenu);
    controlLayout->addWidget(backButton);
    
    layout->addLayout(controlLayout);
    
    // 创建定时器更新分数
    QTimer* scoreTimer = new QTimer(this);
    connect(scoreTimer, &QTimer::timeout, this, [this]() {
        if (gameEngine_ && scoreLabel_) {
            int score = gameEngine_->getCurrentScore();
            int combo = gameEngine_->getComboCount();
            scoreLabel_->setText(QString("💯 分数: %1 | 🔥 连击: %2").arg(score).arg(combo));
        }
    });
    scoreTimer->start(100);  // 每100ms更新一次
    
    qDebug() << "GameView widget created";
}
