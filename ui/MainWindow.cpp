#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "LoginWidget.h"
#include "../src/achievement/AchievementManager.h"
#include "../src/data/Database.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QStackedWidget>
#include <QCoreApplication>

/**
 * @brief 构造函数
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , loginWidget_(nullptr)
    , currentPlayerId_("guest")
    , currentPlayerName_("")
    , gameEngine_(nullptr)
    , gameTestWidget_(nullptr)
    , gameOutputText_(nullptr)
    , testSwapButton_(nullptr)
    , backToMenuButton_(nullptr)
    , gameView_(nullptr)
    , gameViewWidget_(nullptr)
    , scoreLabel_(nullptr)
    , hammerButton_(nullptr)
    , clampButton_(nullptr)
    , magicWandButton_(nullptr)
    , hammerCountLabel_(nullptr)
    , clampCountLabel_(nullptr)
    , magicWandCountLabel_(nullptr)
    , achievementNotification_(nullptr)
{
    ui->setupUi(this);
    setupUi();
    connectSignals();
    
    // 初始化数据库（使用绝对路径）
    QString dbPath = QCoreApplication::applicationDirPath() + "/fruitcrush.db";
    if (!Database::instance().initialize(dbPath)) {
        qCritical() << "Failed to initialize database!";
    } else {
        qDebug() << "Database initialized at:" << dbPath;
    }
    
    // 初始化成就系统（通用部分）
    AchievementManager::instance().initialize();
    
    // 创建成就通知组件
    achievementNotification_ = new AchievementNotificationWidget(this);
    
    // Register achievement notification callback
    AchievementManager::instance().setNotificationCallback(
        [this](const AchievementNotification& notification) {
            achievementNotification_->enqueueNotification(notification);
        }
    );
    
    // 创建游戏引擎并传递给成就系统（用于添加奖励分数）
    if (!gameEngine_) {
        gameEngine_ = new GameEngine();
        AchievementManager::instance().setGameEngine(gameEngine_);
    }
    
    // 显示登录界面
    showLoginScreen();
    
    qDebug() << "MainWindow initialized";
}

/**
 * @brief 析构函数
 */
MainWindow::~MainWindow()
{
    qDebug() << "========== SHUTTING DOWN ==========";
    
    // 如果游戏还在进行中，先结束会话保存数据
    if (gameEngine_) {
        gameEngine_->endGameSession();
        qDebug() << "✅ Game session ended on shutdown";
    }
    
    // 关闭成就系统
    AchievementManager::instance().shutdown();
    
    // 关闭数据库连接
    Database::instance().close();
    qDebug() << "✅ Database closed";
    
    // 释放资源
    if (gameEngine_) {
        delete gameEngine_;
    }
    if (gameTestWidget_) {
        delete gameTestWidget_;
    }
    if (gameViewWidget_) {
        delete gameViewWidget_;
    }
    if (achievementNotification_) {
        delete achievementNotification_;
    }
    delete ui;
    
    qDebug() << "=====================================";
}

/**
 * @brief 初始化UI组件
 */
void MainWindow::setupUi()
{
    // 设置窗口标题和大小
    setWindowTitle("水果消消乐 - Fruit Crush");
    setMinimumSize(800, 600);
    
    // ui->setupUi 已经在构造函数中调用，会创建 centralwidget 和 stackedWidget
    // stackedWidget 已由 .ui 文件创建，包含 mainMenuPage
    
    qDebug() << "MainWindow setupUi completed";
}

/**
 * @brief 显示登录界面
 */
void MainWindow::showLoginScreen()
{
    // 创建登录界面并添加到 stackedWidget
    if (!loginWidget_) {
        loginWidget_ = new LoginWidget(this);
        
        // 将登录界面添加到 stackedWidget（作为新的页面）
        if (ui->stackedWidget) {
            ui->stackedWidget->addWidget(loginWidget_);
        }
        
        // 🔴 连接登录成功信号
        connect(loginWidget_, &LoginWidget::loginSucceeded, this, [this](const QString& playerId, const QString& playerName) {
            // 初始化该玩家的成就系统
            initAchievementSystemForPlayer(playerId);
            
            // 显示主菜单
            showMainMenu();
        });
    }
    
    // 切换到登录界面页面
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentWidget(loginWidget_);
    }
}

/**
 * @brief 初始化成就系统（用于当前玩家）
 */
void MainWindow::initAchievementSystemForPlayer(const QString& playerId)
{
    currentPlayerId_ = playerId;
    
    // 通知成就系统切换玩家
    AchievementManager::instance().setCurrentPlayerId(playerId);
    
    if (playerId == "guest") {
        currentPlayerName_ = "游客";
    } else {
        PlayerData player = Database::instance().getPlayer(playerId);
        currentPlayerName_ = player.username;
        Database::instance().initializeAchievements(playerId);
        Database::instance().setCurrentPlayerId(playerId);
    }
    
    // 设置数据库的当前玩家 ID
    Database::instance().setCurrentPlayerId(playerId);
}

/**
 * @brief 连接信号和槽
 */
void MainWindow::connectSignals()
{
    // 注意：主菜单按钮的连接会在 showMainMenu() 中进行
    // 这里暂时不连接，因为菜单还不存在
    // 当登录成功后，showMainMenu() 会重新创建菜单并连接按钮
}

/**
 * @brief 显示主菜单
 */
void MainWindow::showMainMenu()
{
    qDebug() << "Show Main Menu for player:" << currentPlayerId_;
    
    // 重新连接菜单按钮（现在我们知道玩家已登录）
    if (ui->casualModeButton) {
        connect(ui->casualModeButton, &QPushButton::clicked, this, &MainWindow::startCasualMode, Qt::UniqueConnection);
    }
    if (ui->competitionModeButton) {
        connect(ui->competitionModeButton, &QPushButton::clicked, this, &MainWindow::startCompetitionMode, Qt::UniqueConnection);
    }
    if (ui->leaderboardButton) {
        connect(ui->leaderboardButton, &QPushButton::clicked, this, &MainWindow::showLeaderboard, Qt::UniqueConnection);
    }
    if (ui->achievementsButton) {
        connect(ui->achievementsButton, &QPushButton::clicked, this, &MainWindow::showAchievements, Qt::UniqueConnection);
    }
    
    // 切换到主菜单页面（.ui 文件中的 mainMenuPage，index 0）
    if (ui && ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

/**
 * @brief 开始休闲模式游戏
 */
void MainWindow::startCasualMode()
{
    Q_ASSERT(gameEngine_ != nullptr);
    
    // 从数据库加载玩家数据
    int savedScore = 0;
    int hammerCount = 3, clampCount = 3, magicWandCount = 3;
    
    if (currentPlayerId_ != "guest") {
        savedScore = Database::instance().getPlayerScore(currentPlayerId_);
        Database::PropData props = Database::instance().getPlayerProps(currentPlayerId_);
        hammerCount = props.hammerCount;
        clampCount = props.clampCount;
        magicWandCount = props.magicWandCount;
    }
    
    // 初始化游戏引擎
    gameEngine_->initializeGame(savedScore);
    gameEngine_->getPropManager().setAllProps(hammerCount, clampCount, magicWandCount);
    gameEngine_->startGameSession("Casual");
    
    // ======== 第4步：创建/显示游戏界面 ========
    if (!gameViewWidget_) {
        createGameViewWidget();
    }
    gameView_->setGameEngine(gameEngine_);
    
    // 切换到游戏界面
    if (!ui->stackedWidget->findChild<QWidget*>("gamePageWidget")) {
        ui->stackedWidget->addWidget(gameViewWidget_);
    }
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
    AchievementDialog dialog(currentPlayerId_, currentPlayerName_, this);
    dialog.exec();
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
    qDebug() << "========== BACK TO MENU ==========";
    
    if (gameEngine_) {
        // ======== 结束游戏会话（保存分数+成就） ========
        gameEngine_->endGameSession();
        qDebug() << "✅ Game session ended, data saved";
    }
    
    qDebug() << "=====================================";
    
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
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    
    // 创建OpenGL游戏视图
    gameView_ = new GameView(gameViewWidget_);
    layout->addWidget(gameView_);
    
    // 添加底部控制栏
    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(15);
    
    // 分数显示
    scoreLabel_ = new QLabel("💯 分数: 0 | 🔥 连击: 0");
    scoreLabel_->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: #FFD700; padding: 10px; }");
    controlLayout->addWidget(scoreLabel_);
    
    controlLayout->addStretch();
    
    // 道具栏标签
    QLabel* propLabel = new QLabel("🎮 道具:");
    propLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; padding: 5px; }");
    controlLayout->addWidget(propLabel);
    
    // 锤子道具
    QVBoxLayout* hammerLayout = new QVBoxLayout();
    hammerLayout->setSpacing(2);
    hammerButton_ = new QPushButton();
    hammerButton_->setIcon(QIcon("resources/props/hammer.png"));
    hammerButton_->setIconSize(QSize(48, 48));
    hammerButton_->setFixedSize(60, 60);
    hammerButton_->setToolTip("🔨 锤子 - 消除单个水果");
    hammerButton_->setStyleSheet(
        "QPushButton { "
        "  border: 2px solid #8B4513; "
        "  border-radius: 8px; "
        "  background-color: #FFF8DC; "
        "} "
        "QPushButton:hover { "
        "  background-color: #FFE4B5; "
        "  border: 3px solid #A0522D; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #DEB887; "
        "}"
    );
    connect(hammerButton_, &QPushButton::clicked, this, &MainWindow::onHammerClicked);
    hammerCountLabel_ = new QLabel("x 3");
    hammerCountLabel_->setAlignment(Qt::AlignCenter);
    hammerCountLabel_->setStyleSheet("QLabel { font-size: 12px; font-weight: bold; }");
    hammerLayout->addWidget(hammerButton_);
    hammerLayout->addWidget(hammerCountLabel_);
    controlLayout->addLayout(hammerLayout);
    
    // 夹子道具
    QVBoxLayout* clampLayout = new QVBoxLayout();
    clampLayout->setSpacing(2);
    clampButton_ = new QPushButton();
    clampButton_->setIcon(QIcon("resources/props/clamp.png"));
    clampButton_->setIconSize(QSize(48, 48));
    clampButton_->setFixedSize(60, 60);
    clampButton_->setToolTip("✂️ 夹子 - 强制交换相邻水果");
    clampButton_->setStyleSheet(
        "QPushButton { "
        "  border: 2px solid #4169E1; "
        "  border-radius: 8px; "
        "  background-color: #F0F8FF; "
        "} "
        "QPushButton:hover { "
        "  background-color: #E6F3FF; "
        "  border: 3px solid #1E90FF; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #ADD8E6; "
        "}"
    );
    connect(clampButton_, &QPushButton::clicked, this, &MainWindow::onClampClicked);
    clampCountLabel_ = new QLabel("x 3");
    clampCountLabel_->setAlignment(Qt::AlignCenter);
    clampCountLabel_->setStyleSheet("QLabel { font-size: 12px; font-weight: bold; }");
    clampLayout->addWidget(clampButton_);
    clampLayout->addWidget(clampCountLabel_);
    controlLayout->addLayout(clampLayout);
    
    // 魔法棒道具
    QVBoxLayout* wandLayout = new QVBoxLayout();
    wandLayout->setSpacing(2);
    magicWandButton_ = new QPushButton();
    magicWandButton_->setIcon(QIcon("resources/props/magic_wand.png"));
    magicWandButton_->setIconSize(QSize(48, 48));
    magicWandButton_->setFixedSize(60, 60);
    magicWandButton_->setToolTip("✨ 魔法棒 - 消除所有同类型水果");
    magicWandButton_->setStyleSheet(
        "QPushButton { "
        "  border: 2px solid #9370DB; "
        "  border-radius: 8px; "
        "  background-color: #F8F0FF; "
        "} "
        "QPushButton:hover { "
        "  background-color: #F0E6FF; "
        "  border: 3px solid #8A2BE2; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #DDA0DD; "
        "}"
    );
    connect(magicWandButton_, &QPushButton::clicked, this, &MainWindow::onMagicWandClicked);
    magicWandCountLabel_ = new QLabel("x 3");
    magicWandCountLabel_->setAlignment(Qt::AlignCenter);
    magicWandCountLabel_->setStyleSheet("QLabel { font-size: 12px; font-weight: bold; }");
    wandLayout->addWidget(magicWandButton_);
    wandLayout->addWidget(magicWandCountLabel_);
    controlLayout->addLayout(wandLayout);
    
    controlLayout->addSpacing(20);
    
    // 返回按钮
    QPushButton* backButton = new QPushButton("返回主菜单");
    backButton->setMinimumSize(120, 40);
    connect(backButton, &QPushButton::clicked, this, &MainWindow::backToMenu);
    controlLayout->addWidget(backButton);
    
    layout->addLayout(controlLayout);
    
    // 创建定时器更新分数和道具数量
    QTimer* updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        if (gameEngine_ && scoreLabel_) {
            int score = gameEngine_->getCurrentScore();
            int combo = gameEngine_->getComboCount();
            scoreLabel_->setText(QString("💯 分数: %1 | 🔥 连击: %2").arg(score).arg(combo));
            updatePropCounts();
        }
    });
    updateTimer->start(100);  // 每100ms更新一次
    
    qDebug() << "GameView widget created with prop buttons";
}

/**
 * @brief 锤子按钮点击事件
 */
void MainWindow::onHammerClicked()
{
    if (!gameEngine_ || !gameView_) {
        return;
    }
    
    // 检查是否有锤子
    if (!gameEngine_->getPropManager().hasProp(PropType::HAMMER)) {
        // TODO: 显示提示：道具不足
        return;
    }
    
    // 拿取锤子
    gameView_->setClickMode(ClickMode::PROP_HAMMER);
}

/**
 * @brief 夹子按钮点击事件
 */
void MainWindow::onClampClicked()
{
    if (!gameEngine_ || !gameView_) {
        return;
    }
    
    // 检查是否有夹子
    if (!gameEngine_->getPropManager().hasProp(PropType::CLAMP)) {
        // TODO: 显示提示：道具不足
        return;
    }
    
    // 拿取夹子
    gameView_->setClickMode(ClickMode::PROP_CLAMP);
}

/**
 * @brief 魔法棒按钮点击事件
 */
void MainWindow::onMagicWandClicked()
{
    if (!gameEngine_ || !gameView_) {
        return;
    }
    
    // 检查是否有魔法棒
    if (!gameEngine_->getPropManager().hasProp(PropType::MAGIC_WAND)) {
        // TODO: 显示提示：道具不足
        return;
    }
    
    // 拿取魔法棒
    gameView_->setClickMode(ClickMode::PROP_MAGIC_WAND);
}

/**
 * @brief 更新道具数量显示
 */
void MainWindow::updatePropCounts()
{
    if (!gameEngine_) {
        return;
    }
    
    PropManager& propManager = gameEngine_->getPropManager();
    
    // 更新数量标签
    if (hammerCountLabel_) {
        int count = propManager.getPropCount(PropType::HAMMER);
        hammerCountLabel_->setText(QString("x %1").arg(count));
        hammerButton_->setEnabled(count > 0);
    }
    
    if (clampCountLabel_) {
        int count = propManager.getPropCount(PropType::CLAMP);
        clampCountLabel_->setText(QString("x %1").arg(count));
        clampButton_->setEnabled(count > 0);
    }
    
    if (magicWandCountLabel_) {
        int count = propManager.getPropCount(PropType::MAGIC_WAND);
        magicWandCountLabel_->setText(QString("x %1").arg(count));
        magicWandButton_->setEnabled(count > 0);
    }
}
