#include "LoginWidget.h"
#include "../src/data/Database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QFrame>

LoginWidget::LoginWidget(QWidget* parent)
    : QWidget(parent)
    , currentPlayerId_("guest")
    , currentPlayerName_("")
    , currentPlayerScore_(0)
{
    setupUi();
    showLoginInterface();
}

void LoginWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 标题
    QLabel* titleLabel = new QLabel("🎮 水果消消乐");
    QFont titleFont("Arial", 28, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // ==================== 登录界面部分 ====================
    
    // 分隔符
    QFrame* separator1 = new QFrame();
    separator1->setFrameShape(QFrame::HLine);
    separator1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator1);
    
    // 玩家ID输入
    QHBoxLayout* idLayout = new QHBoxLayout();
    QLabel* idLabel = new QLabel("玩家 ID：");
    idLabel->setMinimumWidth(80);
    playerIdInput_ = new QLineEdit();
    playerIdInput_->setPlaceholderText("输入玩家ID");
    playerIdInput_->setMinimumHeight(35);
    idLayout->addWidget(idLabel);
    idLayout->addWidget(playerIdInput_);
    mainLayout->addLayout(idLayout);
    
    // 玩家名称输入（新账户）
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("玩家名称：");
    nameLabel->setMinimumWidth(80);
    playerNameInput_ = new QLineEdit();
    playerNameInput_->setPlaceholderText("新账户时填写（可选）");
    playerNameInput_->setMinimumHeight(35);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(playerNameInput_);
    mainLayout->addLayout(nameLayout);
    
    // 状态标签
    statusLabel_ = new QLabel("");
    statusLabel_->setObjectName("statusLabel");
    mainLayout->addWidget(statusLabel_);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    loginButton_ = new QPushButton("🔐 登录");
    loginButton_->setObjectName("loginButton");
    loginButton_->setMinimumHeight(40);
    loginButton_->setMinimumWidth(120);
    buttonLayout->addWidget(loginButton_);
    
    guestButton_ = new QPushButton("👤 游客模式");
    guestButton_->setObjectName("guestButton");
    guestButton_->setMinimumHeight(40);
    guestButton_->setMinimumWidth(120);
    buttonLayout->addWidget(guestButton_);
    
    mainLayout->addLayout(buttonLayout);
    
    // ==================== 账户信息显示部分 ====================
    
    // 分隔符
    QFrame* separator2 = new QFrame();
    separator2->setFrameShape(QFrame::HLine);
    separator2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator2);
    
    // 玩家信息显示（初始隐藏）
    playerInfoLabel_ = new QLabel("");
    playerInfoLabel_->setObjectName("playerInfoLabel");
    playerInfoLabel_->setAlignment(Qt::AlignLeft);
    playerInfoLabel_->setVisible(false);
    mainLayout->addWidget(playerInfoLabel_);
    
    logoutButton_ = new QPushButton("🚪 登出");
    logoutButton_->setObjectName("logoutButton");
    logoutButton_->setMinimumHeight(35);
    logoutButton_->setVisible(false);
    mainLayout->addWidget(logoutButton_);
    
    mainLayout->addStretch();
    
    // 连接信号
    connect(loginButton_, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
    connect(guestButton_, &QPushButton::clicked, this, &LoginWidget::onGuestClicked);
    connect(logoutButton_, &QPushButton::clicked, this, &LoginWidget::resetLoginState);
}

void LoginWidget::onLoginClicked()
{
    QString playerId = playerIdInput_->text().trimmed();
    QString playerName = playerNameInput_->text().trimmed();
    
    if (playerId.isEmpty()) {
        statusLabel_->setText("❌ 请输入玩家ID");
        statusLabel_->setStyleSheet("color: red;");
        return;
    }
    
    if (performLogin(playerId, playerName)) {
        statusLabel_->setText("✅ 登录成功！");
        statusLabel_->setStyleSheet("color: green;");
        showAccountInterface();
        
        // 🔴 发送登录成功信号，让 MainWindow 显示主菜单
        emit loginSucceeded(currentPlayerId_, currentPlayerName_);
    } else {
        statusLabel_->setText("❌ 登录失败，请检查ID");
        statusLabel_->setStyleSheet("color: red;");
    }
}

void LoginWidget::onGuestClicked()
{
    currentPlayerId_ = "guest";
    currentPlayerName_ = "游客";
    currentPlayerScore_ = 0;
    
    statusLabel_->setText("✅ 以游客身份进入（不保存数据）");
    statusLabel_->setStyleSheet("color: orange;");
    
    showAccountInterface();
    
    // 🔴 发送登录成功信号，让 MainWindow 显示主菜单
    emit loginSucceeded(currentPlayerId_, currentPlayerName_);
}

bool LoginWidget::performLogin(const QString& playerId, const QString& playerName)
{
    // 查询或创建玩家
    PlayerData player = Database::instance().getPlayer(playerId);
    
    if (player.playerId.isEmpty()) {
        // 玩家不存在，创建新玩家
        if (playerName.isEmpty()) {
            statusLabel_->setText("❌ 新账户请输入玩家名称");
            statusLabel_->setStyleSheet("color: red;");
            return false;
        }
        
        if (!Database::instance().createPlayer(playerId, playerName)) {
            qWarning() << "Failed to create player:" << playerId;
            return false;
        }
        
        // 初始化该玩家的成就
        if (!Database::instance().initializeAchievements(playerId)) {
            qWarning() << "Failed to initialize achievements for player:" << playerId;
            return false;
        }
        
        currentPlayerId_ = playerId;
        currentPlayerName_ = playerName;
        currentPlayerScore_ = 0;
    } else {
        // 玩家已存在
        currentPlayerId_ = playerId;
        currentPlayerName_ = player.username;
        currentPlayerScore_ = Database::instance().getPlayerScore(playerId);
        
        // 确保该玩家的成就已初始化
        Database::instance().initializeAchievements(playerId);
    }
    
    return true;
}

void LoginWidget::updatePlayerInfoDisplay()
{
    if (isLoggedIn()) {
        playerInfoLabel_->setText(
            QString("👤 玩家ID：%1  |  📝 名称：%2  |  📊 分数：%3")
                .arg(currentPlayerId_, currentPlayerName_, QString::number(currentPlayerScore_))
        );
    } else {
        playerInfoLabel_->setText(
            "👤 游客模式 - 数据不会被保存"
        );
    }
}

void LoginWidget::showLoginInterface()
{
    playerIdInput_->setVisible(true);
    playerIdInput_->clear();
    playerNameInput_->setVisible(true);
    playerNameInput_->clear();
    statusLabel_->clear();
    loginButton_->setVisible(true);
    guestButton_->setVisible(true);
    
    playerInfoLabel_->setVisible(false);
    logoutButton_->setVisible(false);
}

void LoginWidget::showAccountInterface()
{
    playerIdInput_->setVisible(false);
    playerNameInput_->setVisible(false);
    statusLabel_->clear();
    loginButton_->setVisible(false);
    guestButton_->setVisible(false);
    
    updatePlayerInfoDisplay();
    playerInfoLabel_->setVisible(true);
    logoutButton_->setVisible(true);
}

void LoginWidget::resetLoginState()
{
    currentPlayerId_ = "guest";
    currentPlayerName_ = "";
    currentPlayerScore_ = 0;
    
    showLoginInterface();
    statusLabel_->setText("已登出");
    statusLabel_->setStyleSheet("color: gray;");
}
