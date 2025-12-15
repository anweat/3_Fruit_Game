#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "LoginWidget.h"
#include "AchievementDialog.h"
#include "SettingsDialog.h"
#include "../src/achievement/AchievementManager.h"
#include "../src/data/Database.h"
#include "../src/data/RankManager.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QStackedWidget>
#include <QCoreApplication>
#include <QSettings>
#include <QMessageBox>
#include <QHeaderView>

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
    , currentGameMode_(GameModeType::CASUAL)
    , competitionMode_(nullptr)
    , currentCompetitionDuration_(CompetitionDuration::SECONDS_60)
    , gameTestWidget_(nullptr)
    , gameOutputText_(nullptr)
    , testSwapButton_(nullptr)
    , backToMenuButton_(nullptr)
    , competitionSelectWidget_(nullptr)
    , competitionEndWidget_(nullptr)
    , endScoreLabel_(nullptr)
    , endComboLabel_(nullptr)
    , endRankLabel_(nullptr)
    , endMessageLabel_(nullptr)
    , leaderboardWidget_(nullptr)
    , leaderboardTabWidget_(nullptr)
    , leaderboard60Table_(nullptr)
    , leaderboard120Table_(nullptr)
    , leaderboard180Table_(nullptr)
    , casualGameView_(nullptr)
    , casualGameViewWidget_(nullptr)
    , casualScoreLabel_(nullptr)
    , competitionGameView_(nullptr)
    , competitionGameViewWidget_(nullptr)
    , competitionScoreLabel_(nullptr)
    , timerLabel_(nullptr)
    , casualHammerButton_(nullptr)
    , casualClampButton_(nullptr)
    , casualMagicWandButton_(nullptr)
    , casualHammerCountLabel_(nullptr)
    , casualClampCountLabel_(nullptr)
    , casualMagicWandCountLabel_(nullptr)
    , casualBuyHammerButton_(nullptr)
    , casualBuyClampButton_(nullptr)
    , casualBuyMagicWandButton_(nullptr)
    , compHammerButton_(nullptr)
    , compClampButton_(nullptr)
    , compMagicWandButton_(nullptr)
    , compHammerCountLabel_(nullptr)
    , compClampCountLabel_(nullptr)
    , compMagicWandCountLabel_(nullptr)
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
    
    // 创建比赛模式管理器
    competitionMode_ = new CompetitionMode(this);
    connect(competitionMode_, &CompetitionMode::timeUpdated, 
            this, &MainWindow::onCompetitionTimeUpdated);
    connect(competitionMode_, &CompetitionMode::competitionEnded, 
            this, &MainWindow::onCompetitionEnded);
    connect(competitionMode_, &CompetitionMode::competitionAbandoned, 
            this, &MainWindow::onCompetitionAbandoned);
    
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
    if (casualGameViewWidget_) {
        delete casualGameViewWidget_;
    }
    if (competitionGameViewWidget_) {
        delete competitionGameViewWidget_;
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
    if (ui->settingsButton) {
        connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::showSettings, Qt::UniqueConnection);
    }
    if (ui->settingsButton) {
        connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::showSettings, Qt::UniqueConnection);
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
    
    currentGameMode_ = GameModeType::CASUAL;
    
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
    
    // 🆕 读取设置中的地图大小
    QSettings settings("FruitCrush", "GameSettings");
    int mapSize = settings.value("casual/mapSize", 8).toInt();
    
    // 初始化游戏引擎（传入地图大小）
    gameEngine_->initializeGame(savedScore, mapSize);
    gameEngine_->getPropManager().setAllProps(hammerCount, clampCount, magicWandCount);
    gameEngine_->startGameSession("Casual");
    
    // 创建/显示休闲模式游戏界面
    if (!casualGameViewWidget_) {
        createCasualGameViewWidget();
    }
    casualGameView_->setGameEngine(gameEngine_);
    casualGameView_->updateMapLayout();
    
    // 切换到游戏界面
    if (!ui->stackedWidget->findChild<QWidget*>("casualGamePageWidget")) {
        ui->stackedWidget->addWidget(casualGameViewWidget_);
    }
    ui->stackedWidget->setCurrentWidget(casualGameViewWidget_);
}

/**
 * @brief 开始比赛模式游戏
 */
void MainWindow::startCompetitionMode()
{
    qDebug() << "Start Competition Mode - Show selection screen";
    
    // 创建并显示比赛模式选择界面
    if (!competitionSelectWidget_) {
        createCompetitionSelectWidget();
    }
    
    if (!ui->stackedWidget->findChild<QWidget*>("competitionSelectWidget")) {
        ui->stackedWidget->addWidget(competitionSelectWidget_);
    }
    ui->stackedWidget->setCurrentWidget(competitionSelectWidget_);
}

/**
 * @brief 开始选定时长的比赛
 */
void MainWindow::startSelectedCompetition(CompetitionDuration duration)
{
    currentCompetitionDuration_ = duration;
    currentGameMode_ = GameModeType::COMPETITION;
    
    Q_ASSERT(gameEngine_ != nullptr);
    
    // 初始化游戏引擎（比赛模式固定8x8，从0分开始）
    gameEngine_->initializeGame(0, 8);
    
    // 设置比赛模式道具配给（锤子2，夹子1，魔法棒1）
    gameEngine_->getPropManager().setAllProps(2, 1, 1);
    gameEngine_->startGameSession("Competition");
    
    // 创建/显示比赛模式游戏界面
    if (!competitionGameViewWidget_) {
        createCompetitionGameViewWidget();
    }
    competitionGameView_->setGameEngine(gameEngine_);
    competitionGameView_->updateMapLayout();
    
    // 重置倒计时样式
    if (timerLabel_) {
        timerLabel_->setStyleSheet(
            "font-size: 24px; font-weight: bold; color: #E91E63; "
            "padding: 5px 15px; background-color: #FFF3E0; border-radius: 8px;"
        );
    }
    
    // 切换到游戏界面
    if (!ui->stackedWidget->findChild<QWidget*>("competitionGamePageWidget")) {
        ui->stackedWidget->addWidget(competitionGameViewWidget_);
    }
    ui->stackedWidget->setCurrentWidget(competitionGameViewWidget_);
    
    // 配置并启动比赛计时器
    CompetitionConfig config;
    config.duration = currentCompetitionDuration_;
    competitionMode_->setConfig(config);
    competitionMode_->startCompetition();
    
    qDebug() << "Competition started:" << CompetitionMode::getDurationString(currentCompetitionDuration_);
}

/**
 * @brief 显示排行榜
 */
void MainWindow::showLeaderboard()
{
    qDebug() << "Show Leaderboard";
    
    // 创建排行榜界面
    if (!leaderboardWidget_) {
        createLeaderboardWidget();
    }
    
    // 刷新数据
    refreshLeaderboardData();
    
    // 显示排行榜界面
    if (!ui->stackedWidget->findChild<QWidget*>("leaderboardWidget")) {
        ui->stackedWidget->addWidget(leaderboardWidget_);
    }
    ui->stackedWidget->setCurrentWidget(leaderboardWidget_);
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
 * @brief 显示设置对话框
 */
void MainWindow::showSettings()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

/**
 * @brief 创建比赛模式选择界面
 */
void MainWindow::createCompetitionSelectWidget()
{
    competitionSelectWidget_ = new QWidget();
    competitionSelectWidget_->setObjectName("competitionSelectWidget");
    QVBoxLayout* layout = new QVBoxLayout(competitionSelectWidget_);
    layout->setContentsMargins(50, 50, 50, 50);
    layout->setSpacing(30);
    
    // 标题
    QLabel* titleLabel = new QLabel("🏆 选择比赛模式");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 32px; font-weight: bold; color: #FF9800; "
        "padding: 20px; background-color: #FFF3E0; border-radius: 15px;"
    );
    layout->addWidget(titleLabel);
    
    // 说明
    QLabel* descLabel = new QLabel(
        "⚡ 比赛模式规则：\n"
        "• 固定 8×8 棋盘\n"
        "• 限量道具：锤子×2、夹子×1、魔法棒×1\n"
        "• 在限定时间内尽可能获得高分！"
    );
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setStyleSheet(
        "font-size: 16px; color: #666; padding: 15px; "
        "background-color: #f5f5f5; border-radius: 10px;"
    );
    layout->addWidget(descLabel);
    
    layout->addStretch();
    
    // 按钮区域
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(30);
    
    // 60秒按钮
    QPushButton* btn60 = new QPushButton("⏱️ 60秒赛\n快速挑战");
    btn60->setFixedSize(180, 100);
    btn60->setStyleSheet(
        "QPushButton { font-size: 18px; font-weight: bold; color: white; "
        "background-color: #4CAF50; border-radius: 15px; }"
        "QPushButton:hover { background-color: #45a049; }"
    );
    connect(btn60, &QPushButton::clicked, this, [this]() {
        startSelectedCompetition(CompetitionDuration::SECONDS_60);
    });
    btnLayout->addWidget(btn60);
    
    // 120秒按钮
    QPushButton* btn120 = new QPushButton("⏱️ 120秒赛\n标准挑战");
    btn120->setFixedSize(180, 100);
    btn120->setStyleSheet(
        "QPushButton { font-size: 18px; font-weight: bold; color: white; "
        "background-color: #2196F3; border-radius: 15px; }"
        "QPushButton:hover { background-color: #1976D2; }"
    );
    connect(btn120, &QPushButton::clicked, this, [this]() {
        startSelectedCompetition(CompetitionDuration::SECONDS_120);
    });
    btnLayout->addWidget(btn120);
    
    // 180秒按钮
    QPushButton* btn180 = new QPushButton("⏱️ 180秒赛\n极限挑战");
    btn180->setFixedSize(180, 100);
    btn180->setStyleSheet(
        "QPushButton { font-size: 18px; font-weight: bold; color: white; "
        "background-color: #9C27B0; border-radius: 15px; }"
        "QPushButton:hover { background-color: #7B1FA2; }"
    );
    connect(btn180, &QPushButton::clicked, this, [this]() {
        startSelectedCompetition(CompetitionDuration::SECONDS_180);
    });
    btnLayout->addWidget(btn180);
    
    layout->addLayout(btnLayout);
    
    layout->addStretch();
    
    // 返回按钮
    QPushButton* backBtn = new QPushButton("← 返回主菜单");
    backBtn->setFixedSize(200, 50);
    backBtn->setStyleSheet(
        "QPushButton { font-size: 16px; color: #666; "
        "background-color: #e0e0e0; border-radius: 10px; }"
        "QPushButton:hover { background-color: #bdbdbd; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::backToMenu);
    layout->addWidget(backBtn, 0, Qt::AlignCenter);
    
    qDebug() << "Competition select widget created";
}

/**
 * @brief 创建比赛结束界面
 */
void MainWindow::createCompetitionEndWidget()
{
    competitionEndWidget_ = new QWidget();
    competitionEndWidget_->setObjectName("competitionEndWidget");
    QVBoxLayout* layout = new QVBoxLayout(competitionEndWidget_);
    layout->setContentsMargins(50, 50, 50, 50);
    layout->setSpacing(20);
    
    // 标题
    QLabel* titleLabel = new QLabel("🏆 比赛结束！");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 36px; font-weight: bold; color: #FF9800; "
        "padding: 20px;"
    );
    layout->addWidget(titleLabel);
    
    layout->addStretch();
    
    // 比赛类型
    endRankLabel_ = new QLabel("📊 比赛类型: 60秒赛");
    endRankLabel_->setAlignment(Qt::AlignCenter);
    endRankLabel_->setStyleSheet("font-size: 20px; color: #666; padding: 10px;");
    layout->addWidget(endRankLabel_);
    
    // 分数显示
    endScoreLabel_ = new QLabel("💯 最终得分: 0");
    endScoreLabel_->setAlignment(Qt::AlignCenter);
    endScoreLabel_->setStyleSheet(
        "font-size: 32px; font-weight: bold; color: #E91E63; "
        "padding: 20px; background-color: #FCE4EC; border-radius: 15px;"
    );
    layout->addWidget(endScoreLabel_);
    
    // 连击显示
    endComboLabel_ = new QLabel("🔥 最大连击: 0");
    endComboLabel_->setAlignment(Qt::AlignCenter);
    endComboLabel_->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: #FF5722; padding: 15px;"
    );
    layout->addWidget(endComboLabel_);
    
    // 消息（排名/个人最佳等）
    endMessageLabel_ = new QLabel("");
    endMessageLabel_->setAlignment(Qt::AlignCenter);
    endMessageLabel_->setStyleSheet(
        "font-size: 18px; color: #4CAF50; padding: 15px;"
    );
    layout->addWidget(endMessageLabel_);
    
    layout->addStretch();
    
    // 按钮区域
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(30);
    
    // 查看排行榜按钮
    QPushButton* rankBtn = new QPushButton("📊 查看排行榜");
    rankBtn->setFixedSize(180, 60);
    rankBtn->setStyleSheet(
        "QPushButton { font-size: 18px; font-weight: bold; color: white; "
        "background-color: #2196F3; border-radius: 15px; }"
        "QPushButton:hover { background-color: #1976D2; }"
    );
    connect(rankBtn, &QPushButton::clicked, this, &MainWindow::showLeaderboard);
    btnLayout->addWidget(rankBtn);
    
    // 再来一局按钮
    QPushButton* retryBtn = new QPushButton("🔄 再来一局");
    retryBtn->setFixedSize(180, 60);
    retryBtn->setStyleSheet(
        "QPushButton { font-size: 18px; font-weight: bold; color: white; "
        "background-color: #4CAF50; border-radius: 15px; }"
        "QPushButton:hover { background-color: #45a049; }"
    );
    connect(retryBtn, &QPushButton::clicked, this, &MainWindow::startCompetitionMode);
    btnLayout->addWidget(retryBtn);
    
    // 返回主菜单按钮
    QPushButton* backBtn = new QPushButton("🏠 返回主菜单");
    backBtn->setFixedSize(180, 60);
    backBtn->setStyleSheet(
        "QPushButton { font-size: 18px; font-weight: bold; color: #666; "
        "background-color: #e0e0e0; border-radius: 15px; }"
        "QPushButton:hover { background-color: #bdbdbd; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::backToMenu);
    btnLayout->addWidget(backBtn);
    
    layout->addLayout(btnLayout);
    
    qDebug() << "Competition end widget created";
}

/**
 * @brief 创建排行榜界面
 */
void MainWindow::createLeaderboardWidget()
{
    leaderboardWidget_ = new QWidget();
    leaderboardWidget_->setObjectName("leaderboardWidget");
    QVBoxLayout* layout = new QVBoxLayout(leaderboardWidget_);
    layout->setContentsMargins(40, 30, 40, 30);
    layout->setSpacing(20);
    
    // 标题
    QLabel* titleLabel = new QLabel("🏆 排行榜");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 36px; font-weight: bold; color: #FF9800; "
        "padding: 15px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #FFF8E1, stop:0.5 #FFECB3, stop:1 #FFF8E1); "
        "border-radius: 20px;"
    );
    layout->addWidget(titleLabel);
    
    // 创建Tab控件
    leaderboardTabWidget_ = new QTabWidget();
    leaderboardTabWidget_->setStyleSheet(
        "QTabWidget::pane { "
        "   border: none; "
        "   background-color: #FAFAFA; "
        "   border-radius: 15px; "
        "   padding: 10px; "
        "}"
        "QTabBar::tab { "
        "   padding: 14px 40px; "
        "   font-size: 15px; "
        "   font-weight: bold; "
        "   border-radius: 20px; "
        "   margin: 3px; "
        "}"
        "QTabBar::tab:selected { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #42A5F5, stop:1 #1E88E5); "
        "   color: white; "
        "}"
        "QTabBar::tab:!selected { "
        "   background-color: #EEEEEE; "
        "   color: #757575; "
        "}"
        "QTabBar::tab:!selected:hover { "
        "   background-color: #E0E0E0; "
        "}"
    );
    
    // 创建三个排行榜表格
    auto createTable = [this]() -> QTableWidget* {
        QTableWidget* table = new QTableWidget();
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({"🏅 排名", "👤 玩家", "💯 得分", "📅 时间"});
        table->horizontalHeader()->setStretchLastSection(true);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setAlternatingRowColors(true);
        table->setShowGrid(false);
        table->verticalHeader()->setVisible(false);
        table->setStyleSheet(
            "QTableWidget { "
            "   font-size: 15px; "
            "   background-color: white; "
            "   border: none; "
            "   border-radius: 15px; "
            "}"
            "QHeaderView::section { "
            "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "       stop:0 #66BB6A, stop:1 #43A047); "
            "   color: white; "
            "   font-weight: bold; "
            "   font-size: 14px; "
            "   padding: 12px; "
            "   border: none; "
            "}"
            "QTableWidget::item { "
            "   padding: 10px; "
            "   border-bottom: 1px solid #F0F0F0; "
            "}"
            "QTableWidget::item:selected { "
            "   background-color: #E3F2FD; "
            "   color: #1565C0; "
            "}"
        );
        table->setMinimumHeight(350);
        return table;
    };
    
    leaderboard60Table_ = createTable();
    leaderboard120Table_ = createTable();
    leaderboard180Table_ = createTable();
    
    leaderboardTabWidget_->addTab(leaderboard60Table_, "⚡ 60秒赛");
    leaderboardTabWidget_->addTab(leaderboard120Table_, "🎯 120秒赛");
    leaderboardTabWidget_->addTab(leaderboard180Table_, "🔥 180秒赛");
    
    layout->addWidget(leaderboardTabWidget_);
    
    // 返回按钮
    QPushButton* backBtn = new QPushButton("🏠 返回主菜单");
    backBtn->setFixedSize(180, 50);
    backBtn->setStyleSheet(
        "QPushButton { "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   color: white; "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #78909C, stop:1 #546E7A); "
        "   border-radius: 25px; "
        "   border: none; "
        "}"
        "QPushButton:hover { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #90A4AE, stop:1 #607D8B); "
        "}"
    );
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::backToMenu);
    layout->addWidget(backBtn, 0, Qt::AlignCenter);
    
    qDebug() << "Leaderboard widget created";
}

/**
 * @brief 刷新排行榜数据
 */
void MainWindow::refreshLeaderboardData()
{
    auto fillTable = [this](QTableWidget* table, CompetitionDuration duration) {
        auto records = RankManager::instance().getLeaderboard(duration, 20);
        table->setRowCount(records.size());
        
        for (int i = 0; i < records.size(); ++i) {
            const auto& record = records[i];
            
            // 排名（带图标）
            QString rankText;
            if (record.rank == 1) rankText = "🥇";
            else if (record.rank == 2) rankText = "🥈";
            else if (record.rank == 3) rankText = "🥉";
            else rankText = QString("  %1  ").arg(record.rank);
            
            QTableWidgetItem* rankItem = new QTableWidgetItem(rankText);
            rankItem->setTextAlignment(Qt::AlignCenter);
            QFont rankFont = rankItem->font();
            if (record.rank <= 3) {
                rankFont.setPointSize(18);
            }
            rankItem->setFont(rankFont);
            
            // 高亮当前玩家
            if (record.playerId == currentPlayerId_) {
                rankItem->setBackground(QColor("#E8F5E9"));
            }
            
            table->setItem(i, 0, rankItem);
            
            // 玩家名
            QString nameText = record.playerName;
            if (record.playerId == currentPlayerId_) {
                nameText = "⭐ " + record.playerName;
            }
            QTableWidgetItem* nameItem = new QTableWidgetItem(nameText);
            nameItem->setTextAlignment(Qt::AlignCenter);
            if (record.playerId == currentPlayerId_) {
                nameItem->setBackground(QColor("#E8F5E9"));
                QFont boldFont = nameItem->font();
                boldFont.setBold(true);
                nameItem->setFont(boldFont);
            }
            table->setItem(i, 1, nameItem);
            
            // 得分
            QTableWidgetItem* scoreItem = new QTableWidgetItem(QString::number(record.score));
            scoreItem->setTextAlignment(Qt::AlignCenter);
            QFont scoreFont = scoreItem->font();
            scoreFont.setBold(true);
            scoreItem->setFont(scoreFont);
            if (record.rank <= 3) {
                scoreItem->setForeground(QColor("#E65100"));
            }
            if (record.playerId == currentPlayerId_) {
                scoreItem->setBackground(QColor("#E8F5E9"));
            }
            table->setItem(i, 2, scoreItem);
            
            // 时间（简化格式）
            QString timeStr = record.playedAt.toString("MM-dd HH:mm");
            QTableWidgetItem* timeItem = new QTableWidgetItem(timeStr);
            timeItem->setTextAlignment(Qt::AlignCenter);
            timeItem->setForeground(QColor("#9E9E9E"));
            if (record.playerId == currentPlayerId_) {
                timeItem->setBackground(QColor("#E8F5E9"));
            }
            table->setItem(i, 3, timeItem);
        }
        
        // 设置行高
        for (int i = 0; i < table->rowCount(); ++i) {
            table->setRowHeight(i, 45);
        }
    };
    
    fillTable(leaderboard60Table_, CompetitionDuration::SECONDS_60);
    fillTable(leaderboard120Table_, CompetitionDuration::SECONDS_120);
    fillTable(leaderboard180Table_, CompetitionDuration::SECONDS_180);
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
    int mapSize = static_cast<int>(map.size());
    output += QString("🌎 游戏地图 (%1x%1):\n").arg(mapSize);
    output += "   ";
    for (int col = 0; col < mapSize; col++) {
        output += QString(" %1 ").arg(col);
    }
    output += "\n";
    
    for (int row = 0; row < mapSize; row++) {
        output += QString(" %1 ").arg(row);
        for (int col = 0; col < mapSize; col++) {
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
 * @brief 创建休闲模式游戏视图
 */
void MainWindow::createCasualGameViewWidget()
{
    casualGameViewWidget_ = new QWidget();
    casualGameViewWidget_->setObjectName("casualGamePageWidget");
    QVBoxLayout* layout = new QVBoxLayout(casualGameViewWidget_);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    
    // 创建OpenGL游戏视图
    casualGameView_ = new GameView(casualGameViewWidget_);
    layout->addWidget(casualGameView_);
    
    // 添加底部控制栏
    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(15);
    
    // 分数显示
    casualScoreLabel_ = new QLabel("💯 分数: 0");
    controlLayout->addWidget(casualScoreLabel_);
    
    controlLayout->addStretch();
    
    // 道具栏标签
    QLabel* propLabel = new QLabel("🎮 道具:");
    controlLayout->addWidget(propLabel);
    
    // 锤子道具
    QVBoxLayout* hammerLayout = new QVBoxLayout();
    hammerLayout->setSpacing(2);
    casualHammerButton_ = new QPushButton();
    casualHammerButton_->setIcon(QIcon("resources/props/hammer.png"));
    casualHammerButton_->setIconSize(QSize(48, 48));
    casualHammerButton_->setFixedSize(60, 60);
    casualHammerButton_->setToolTip("🔨 锤子 - 消除单个水果");
    connect(casualHammerButton_, &QPushButton::clicked, this, &MainWindow::onHammerClicked);
    casualHammerCountLabel_ = new QLabel("x 3");
    casualHammerCountLabel_->setAlignment(Qt::AlignCenter);
    casualBuyHammerButton_ = new QPushButton(QString("💰%1").arg(HAMMER_PRICE));
    casualBuyHammerButton_->setFixedSize(60, 24);
    casualBuyHammerButton_->setToolTip("购买锤子 (消耗200分)");
    connect(casualBuyHammerButton_, &QPushButton::clicked, this, &MainWindow::onBuyHammer);
    hammerLayout->addWidget(casualHammerButton_);
    hammerLayout->addWidget(casualHammerCountLabel_);
    hammerLayout->addWidget(casualBuyHammerButton_);
    controlLayout->addLayout(hammerLayout);
    
    // 夹子道具
    QVBoxLayout* clampLayout = new QVBoxLayout();
    clampLayout->setSpacing(2);
    casualClampButton_ = new QPushButton();
    casualClampButton_->setIcon(QIcon("resources/props/clamp.png"));
    casualClampButton_->setIconSize(QSize(48, 48));
    casualClampButton_->setFixedSize(60, 60);
    casualClampButton_->setToolTip("✂️ 夹子 - 强制交换相邻水果");
    connect(casualClampButton_, &QPushButton::clicked, this, &MainWindow::onClampClicked);
    casualClampCountLabel_ = new QLabel("x 3");
    casualClampCountLabel_->setAlignment(Qt::AlignCenter);
    casualBuyClampButton_ = new QPushButton(QString("💰%1").arg(CLAMP_PRICE));
    casualBuyClampButton_->setFixedSize(60, 24);
    casualBuyClampButton_->setToolTip("购买夹子 (消耗200分)");
    connect(casualBuyClampButton_, &QPushButton::clicked, this, &MainWindow::onBuyClamp);
    clampLayout->addWidget(casualClampButton_);
    clampLayout->addWidget(casualClampCountLabel_);
    clampLayout->addWidget(casualBuyClampButton_);
    controlLayout->addLayout(clampLayout);
    
    // 魔法棒道具
    QVBoxLayout* wandLayout = new QVBoxLayout();
    wandLayout->setSpacing(2);
    casualMagicWandButton_ = new QPushButton();
    casualMagicWandButton_->setIcon(QIcon("resources/props/magic_wand.png"));
    casualMagicWandButton_->setIconSize(QSize(48, 48));
    casualMagicWandButton_->setFixedSize(60, 60);
    casualMagicWandButton_->setToolTip("✨ 魔法棒 - 刷新棋盘");
    connect(casualMagicWandButton_, &QPushButton::clicked, this, &MainWindow::onMagicWandClicked);
    casualMagicWandCountLabel_ = new QLabel("x 3");
    casualMagicWandCountLabel_->setAlignment(Qt::AlignCenter);
    casualBuyMagicWandButton_ = new QPushButton(QString("💰%1").arg(MAGIC_WAND_PRICE));
    casualBuyMagicWandButton_->setFixedSize(60, 24);
    casualBuyMagicWandButton_->setToolTip("购买魔法棒 (消耗400分)");
    connect(casualBuyMagicWandButton_, &QPushButton::clicked, this, &MainWindow::onBuyMagicWand);
    wandLayout->addWidget(casualMagicWandButton_);
    wandLayout->addWidget(casualMagicWandCountLabel_);
    wandLayout->addWidget(casualBuyMagicWandButton_);
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
        if (gameEngine_ && casualScoreLabel_ && currentGameMode_ == GameModeType::CASUAL) {
            int score = gameEngine_->getCurrentScore();
            casualScoreLabel_->setText(QString("💯 分数: %1").arg(score));
            updateCasualPropCounts();
        }
    });
    updateTimer->start(100);
    
    qDebug() << "Casual GameView widget created with prop buttons";
}

/**
 * @brief 锤子按钮点击事件
 */
void MainWindow::onHammerClicked()
{
    if (!gameEngine_) return;
    
    // 检查是否有锤子
    if (!gameEngine_->getPropManager().hasProp(PropType::HAMMER)) {
        return;
    }
    
    // 根据当前模式使用对应的GameView
    if (currentGameMode_ == GameModeType::CASUAL && casualGameView_) {
        casualGameView_->setClickMode(ClickMode::PROP_HAMMER);
    } else if (currentGameMode_ == GameModeType::COMPETITION && competitionGameView_) {
        competitionGameView_->setClickMode(ClickMode::PROP_HAMMER);
    }
}

/**
 * @brief 夹子按钮点击事件
 */
void MainWindow::onClampClicked()
{
    if (!gameEngine_) return;
    
    // 检查是否有夹子
    if (!gameEngine_->getPropManager().hasProp(PropType::CLAMP)) {
        return;
    }
    
    // 根据当前模式使用对应的GameView
    if (currentGameMode_ == GameModeType::CASUAL && casualGameView_) {
        casualGameView_->setClickMode(ClickMode::PROP_CLAMP);
    } else if (currentGameMode_ == GameModeType::COMPETITION && competitionGameView_) {
        competitionGameView_->setClickMode(ClickMode::PROP_CLAMP);
    }
}

/**
 * @brief 魔法棒按钮点击事件
 */
void MainWindow::onMagicWandClicked()
{
    if (!gameEngine_) return;
    
    // 检查是否有魔法棒
    if (!gameEngine_->getPropManager().hasProp(PropType::MAGIC_WAND)) {
        return;
    }
    
    // 根据当前模式使用对应的GameView
    if (currentGameMode_ == GameModeType::CASUAL && casualGameView_) {
        casualGameView_->setClickMode(ClickMode::PROP_MAGIC_WAND);
    } else if (currentGameMode_ == GameModeType::COMPETITION && competitionGameView_) {
        competitionGameView_->setClickMode(ClickMode::PROP_MAGIC_WAND);
    }
}

/**
 * @brief 更新休闲模式道具数量显示
 */
void MainWindow::updateCasualPropCounts()
{
    if (!gameEngine_) return;
    
    PropManager& propManager = gameEngine_->getPropManager();
    int currentScore = gameEngine_->getCurrentScore();
    
    // 更新数量标签
    if (casualHammerCountLabel_) {
        int count = propManager.getPropCount(PropType::HAMMER);
        casualHammerCountLabel_->setText(QString("x %1").arg(count));
        casualHammerButton_->setEnabled(count > 0);
    }
    
    if (casualClampCountLabel_) {
        int count = propManager.getPropCount(PropType::CLAMP);
        casualClampCountLabel_->setText(QString("x %1").arg(count));
        casualClampButton_->setEnabled(count > 0);
    }
    
    if (casualMagicWandCountLabel_) {
        int count = propManager.getPropCount(PropType::MAGIC_WAND);
        casualMagicWandCountLabel_->setText(QString("x %1").arg(count));
        casualMagicWandButton_->setEnabled(count > 0);
    }
    
    // 更新购买按钮状态（根据分数是否足够）
    if (casualBuyHammerButton_) {
        casualBuyHammerButton_->setEnabled(currentScore >= HAMMER_PRICE);
    }
    if (casualBuyClampButton_) {
        casualBuyClampButton_->setEnabled(currentScore >= CLAMP_PRICE);
    }
    if (casualBuyMagicWandButton_) {
        casualBuyMagicWandButton_->setEnabled(currentScore >= MAGIC_WAND_PRICE);
    }
}

/**
 * @brief 更新比赛模式道具数量显示
 */
void MainWindow::updateCompetitionPropCounts()
{
    if (!gameEngine_) return;
    
    PropManager& propManager = gameEngine_->getPropManager();
    
    // 更新数量标签
    if (compHammerCountLabel_) {
        int count = propManager.getPropCount(PropType::HAMMER);
        compHammerCountLabel_->setText(QString("x %1").arg(count));
        compHammerButton_->setEnabled(count > 0);
    }
    
    if (compClampCountLabel_) {
        int count = propManager.getPropCount(PropType::CLAMP);
        compClampCountLabel_->setText(QString("x %1").arg(count));
        compClampButton_->setEnabled(count > 0);
    }
    
    if (compMagicWandCountLabel_) {
        int count = propManager.getPropCount(PropType::MAGIC_WAND);
        compMagicWandCountLabel_->setText(QString("x %1").arg(count));
        compMagicWandButton_->setEnabled(count > 0);
    }
}

/**
 * @brief 更新道具数量显示（兼容旧代码）
 */
void MainWindow::updatePropCounts()
{
    if (currentGameMode_ == GameModeType::CASUAL) {
        updateCasualPropCounts();
    } else {
        updateCompetitionPropCounts();
    }
}

/**
 * @brief 购买锤子
 */
void MainWindow::onBuyHammer()
{
    if (!gameEngine_) return;
    
    int currentScore = gameEngine_->getCurrentScore();
    if (currentScore < HAMMER_PRICE) {
        qDebug() << "分数不足，无法购买锤子";
        return;
    }
    
    // 扣除分数
    gameEngine_->addScore(-HAMMER_PRICE);
    
    // 增加道具
    gameEngine_->getPropManager().addProp(PropType::HAMMER, 1);
    
    qDebug() << "购买锤子成功! 剩余分数:" << gameEngine_->getCurrentScore();
    
    // 更新显示
    updateCasualPropCounts();
}

/**
 * @brief 购买夹子
 */
void MainWindow::onBuyClamp()
{
    if (!gameEngine_) return;
    
    int currentScore = gameEngine_->getCurrentScore();
    if (currentScore < CLAMP_PRICE) {
        qDebug() << "分数不足，无法购买夹子";
        return;
    }
    
    // 扣除分数
    gameEngine_->addScore(-CLAMP_PRICE);
    
    // 增加道具
    gameEngine_->getPropManager().addProp(PropType::CLAMP, 1);
    
    qDebug() << "购买夹子成功! 剩余分数:" << gameEngine_->getCurrentScore();
    
    // 更新显示
    updateCasualPropCounts();
}

/**
 * @brief 购买魔法棒
 */
void MainWindow::onBuyMagicWand()
{
    if (!gameEngine_) return;
    
    int currentScore = gameEngine_->getCurrentScore();
    if (currentScore < MAGIC_WAND_PRICE) {
        qDebug() << "分数不足，无法购买魔法棒";
        return;
    }
    
    // 扣除分数
    gameEngine_->addScore(-MAGIC_WAND_PRICE);
    
    // 增加道具
    gameEngine_->getPropManager().addProp(PropType::MAGIC_WAND, 1);
    
    qDebug() << "购买魔法棒成功! 剩余分数:" << gameEngine_->getCurrentScore();
    
    // 更新显示
    updateCasualPropCounts();
}

/**
 * @brief 创建比赛模式游戏视图（无购买按钮，有倒计时）
 */
void MainWindow::createCompetitionGameViewWidget()
{
    competitionGameViewWidget_ = new QWidget();
    competitionGameViewWidget_->setObjectName("competitionPageWidget");
    QVBoxLayout* layout = new QVBoxLayout(competitionGameViewWidget_);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    
    // 顶部信息栏（倒计时 + 分数）
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    // 比赛模式标签
    QLabel* modeLabel = new QLabel("🏆 比赛模式");
    modeLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FF9800;");
    topLayout->addWidget(modeLabel);
    
    topLayout->addStretch();
    
    // 倒计时显示
    timerLabel_ = new QLabel("⏱️ 00:00");
    timerLabel_->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: #E91E63; "
        "padding: 5px 15px; background-color: #FFF3E0; border-radius: 8px;"
    );
    topLayout->addWidget(timerLabel_);
    
    topLayout->addStretch();
    
    // 分数显示
    competitionScoreLabel_ = new QLabel("💯 分数: 0");
    competitionScoreLabel_->setStyleSheet("font-size: 18px; font-weight: bold;");
    topLayout->addWidget(competitionScoreLabel_);
    
    layout->addLayout(topLayout);
    
    // 创建OpenGL游戏视图
    competitionGameView_ = new GameView(competitionGameViewWidget_);
    layout->addWidget(competitionGameView_);
    
    // 底部控制栏（道具，无购买按钮）
    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(15);
    
    // 道具栏标签
    QLabel* propLabel = new QLabel("🎮 道具（限量）:");
    controlLayout->addWidget(propLabel);
    
    // 锤子道具
    QVBoxLayout* hammerLayout = new QVBoxLayout();
    hammerLayout->setSpacing(2);
    compHammerButton_ = new QPushButton();
    compHammerButton_->setIcon(QIcon("resources/props/hammer.png"));
    compHammerButton_->setIconSize(QSize(48, 48));
    compHammerButton_->setFixedSize(60, 60);
    compHammerButton_->setToolTip("🔨 锤子 - 消除单个水果 (剩余2个)");
    connect(compHammerButton_, &QPushButton::clicked, this, &MainWindow::onHammerClicked);
    compHammerCountLabel_ = new QLabel("x 2");
    compHammerCountLabel_->setAlignment(Qt::AlignCenter);
    hammerLayout->addWidget(compHammerButton_);
    hammerLayout->addWidget(compHammerCountLabel_);
    controlLayout->addLayout(hammerLayout);
    
    // 夹子道具
    QVBoxLayout* clampLayout = new QVBoxLayout();
    clampLayout->setSpacing(2);
    compClampButton_ = new QPushButton();
    compClampButton_->setIcon(QIcon("resources/props/clamp.png"));
    compClampButton_->setIconSize(QSize(48, 48));
    compClampButton_->setFixedSize(60, 60);
    compClampButton_->setToolTip("✂️ 夹子 - 强制交换相邻水果 (剩余1个)");
    connect(compClampButton_, &QPushButton::clicked, this, &MainWindow::onClampClicked);
    compClampCountLabel_ = new QLabel("x 1");
    compClampCountLabel_->setAlignment(Qt::AlignCenter);
    clampLayout->addWidget(compClampButton_);
    clampLayout->addWidget(compClampCountLabel_);
    controlLayout->addLayout(clampLayout);
    
    // 魔法棒道具
    QVBoxLayout* wandLayout = new QVBoxLayout();
    wandLayout->setSpacing(2);
    compMagicWandButton_ = new QPushButton();
    compMagicWandButton_->setIcon(QIcon("resources/props/magic_wand.png"));
    compMagicWandButton_->setIconSize(QSize(48, 48));
    compMagicWandButton_->setFixedSize(60, 60);
    compMagicWandButton_->setToolTip("✨ 魔法棒 - 刷新棋盘 (剩余1个)");
    connect(compMagicWandButton_, &QPushButton::clicked, this, &MainWindow::onMagicWandClicked);
    compMagicWandCountLabel_ = new QLabel("x 1");
    compMagicWandCountLabel_->setAlignment(Qt::AlignCenter);
    wandLayout->addWidget(compMagicWandButton_);
    wandLayout->addWidget(compMagicWandCountLabel_);
    controlLayout->addLayout(wandLayout);
    
    controlLayout->addStretch();
    
    // 放弃比赛按钮
    QPushButton* quitButton = new QPushButton("🚪 放弃比赛");
    quitButton->setMinimumSize(120, 40);
    quitButton->setStyleSheet("background-color: #f44336; color: white;");
    connect(quitButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "确认放弃", 
            "确定要放弃本次比赛吗？\n成绩将不会被记录。",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::Yes) {
            competitionMode_->abandonCompetition();  // 使用放弃方法，不记录成绩
        }
    });
    controlLayout->addWidget(quitButton);
    
    layout->addLayout(controlLayout);
    
    // 创建定时器更新分数和道具数量
    QTimer* updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        if (gameEngine_ && competitionScoreLabel_ && currentGameMode_ == GameModeType::COMPETITION) {
            int score = gameEngine_->getCurrentScore();
            competitionScoreLabel_->setText(QString("💯 分数: %1").arg(score));
            updateCompetitionPropCounts();
        }
    });
    updateTimer->start(100);
    
    qDebug() << "Competition GameView widget created";
}

/**
 * @brief 比赛倒计时更新
 */
void MainWindow::onCompetitionTimeUpdated(int remainingSeconds)
{
    if (!timerLabel_) return;
    
    int minutes = remainingSeconds / 60;
    int seconds = remainingSeconds % 60;
    QString timeStr = QString("⏱️ %1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
    
    timerLabel_->setText(timeStr);
    
    // 最后10秒变红色警告
    if (remainingSeconds <= 10) {
        timerLabel_->setStyleSheet(
            "font-size: 24px; font-weight: bold; color: white; "
            "padding: 5px 15px; background-color: #F44336; border-radius: 8px;"
        );
    } else if (remainingSeconds <= 30) {
        timerLabel_->setStyleSheet(
            "font-size: 24px; font-weight: bold; color: #FF9800; "
            "padding: 5px 15px; background-color: #FFF3E0; border-radius: 8px;"
        );
    }
}

/**
 * @brief 比赛正常结束（记录成绩）
 */
void MainWindow::onCompetitionEnded()
{
    if (!gameEngine_) return;
    
    int finalScore = gameEngine_->getCurrentScore();
    int maxCombo = gameEngine_->getSessionStats().maxCombo;
    
    qDebug() << "Competition ended! Score:" << finalScore << "Max Combo:" << maxCombo;
    
    // 显示比赛结束界面（嵌入式）
    showCompetitionEndScreen(finalScore, maxCombo);
}

/**
 * @brief 比赛放弃（不记录成绩）
 */
void MainWindow::onCompetitionAbandoned()
{
    qDebug() << "Competition abandoned - score not recorded";
    backToMenu();
}

/**
 * @brief 显示比赛结束界面（嵌入式）
 */
void MainWindow::showCompetitionEndScreen(int finalScore, int maxCombo)
{
    // 记录成绩到排行榜
    bool isPersonalBest = RankManager::instance().isPersonalBest(
        currentPlayerId_, finalScore, currentCompetitionDuration_);
    
    if (currentPlayerId_ != "guest") {
        RankManager::instance().recordScore(
            currentPlayerId_, 
            currentPlayerName_,
            finalScore, 
            maxCombo, 
            currentCompetitionDuration_
        );
    }
    
    // 获取排名
    int rank = RankManager::instance().getPlayerRank(currentPlayerId_, currentCompetitionDuration_);
    
    // 创建结束界面
    if (!competitionEndWidget_) {
        createCompetitionEndWidget();
    }
    
    // 更新显示内容
    if (endScoreLabel_) {
        endScoreLabel_->setText(QString("💯 最终得分: %1").arg(finalScore));
    }
    if (endComboLabel_) {
        endComboLabel_->setText(QString("🔥 最大连击: %1").arg(maxCombo));
    }
    
    QString message;
    if (currentPlayerId_ != "guest") {
        if (isPersonalBest) {
            message = "🎉 恭喜！新的个人最佳记录！";
        }
        if (rank > 0 && rank <= 10) {
            if (!message.isEmpty()) message += "\n";
            message += QString("🏅 当前排名: 第 %1 名").arg(rank);
        }
    } else {
        message = "💡 登录后可保存成绩到排行榜";
    }
    
    if (endRankLabel_) {
        endRankLabel_->setText(QString("📊 比赛类型: %1")
            .arg(CompetitionMode::getDurationString(currentCompetitionDuration_)));
    }
    if (endMessageLabel_) {
        endMessageLabel_->setText(message);
    }
    
    // 显示结束界面
    if (!ui->stackedWidget->findChild<QWidget*>("competitionEndWidget")) {
        ui->stackedWidget->addWidget(competitionEndWidget_);
    }
    ui->stackedWidget->setCurrentWidget(competitionEndWidget_);
}
