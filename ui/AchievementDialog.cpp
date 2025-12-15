#include "AchievementDialog.h"
#include "../src/achievement/AchievementManager.h"
#include <QApplication>
#include <QScreen>

AchievementDialog::AchievementDialog(const QString& playerId, 
                                     const QString& playerName,
                                     QWidget* parent)
    : QDialog(parent)
    , playerId_(playerId)
    , playerName_(playerName)
    , isGuest_(playerId.isEmpty() || playerId == "guest")
    , totalAchievements_(0)
    , completedAchievements_(0)
{
    setWindowTitle("🏆 成就系统");
    setMinimumSize(500, 600);
    resize(550, 700);
    
    // 居中显示
    if (QScreen* screen = QApplication::primaryScreen()) {
        QRect screenGeometry = screen->availableGeometry();
        move((screenGeometry.width() - width()) / 2,
             (screenGeometry.height() - height()) / 2);
    }
    
    setupStyleSheet();
    setupUi();
}

AchievementDialog::~AchievementDialog()
{
}

void AchievementDialog::setupStyleSheet()
{
    // 样式已通过全局QSS加载，此处不再需要硬编码
    // 保留此函数以防后续需要对话框特定样式
}

void AchievementDialog::setupUi()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setSpacing(15);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    
    if (isGuest_) {
        createGuestModeView();
    } else {
        loadAchievements();
        createAchievementView();
    }
    
    // 关闭按钮
    QPushButton* closeButton = new QPushButton("关闭");
    closeButton->setObjectName("closeButton");
    closeButton->setFixedHeight(40);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout_->addWidget(closeButton);
}

void AchievementDialog::createGuestModeView()
{
    QWidget* guestWidget = new QWidget();
    QVBoxLayout* guestLayout = new QVBoxLayout(guestWidget);
    guestLayout->setAlignment(Qt::AlignCenter);
    guestLayout->setSpacing(20);
    
    // 图标
    QLabel* iconLabel = new QLabel("🔒");
    iconLabel->setStyleSheet("font-size: 64px;");
    iconLabel->setAlignment(Qt::AlignCenter);
    guestLayout->addWidget(iconLabel);
    
    // 标题
    QLabel* titleLabel = new QLabel("游客模式");
    titleLabel->setStyleSheet(R"(
        color: #FF6B35;
        font-size: 24px;
        font-weight: bold;
    )");
    titleLabel->setAlignment(Qt::AlignCenter);
    guestLayout->addWidget(titleLabel);
    
    // 提示信息
    QLabel* infoLabel = new QLabel(
        "游客模式下成就进度不会保存\n\n"
        "请登录或注册账户以：\n"
        "• 解锁并保存成就进度\n"
        "• 获得成就奖励点数\n"
        "• 查看完整成就列表"
    );
    infoLabel->setStyleSheet(R"(
        color: #6A5A4A;
        font-size: 14px;
        line-height: 1.6;
    )");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setWordWrap(true);
    guestLayout->addWidget(infoLabel);
    
    // 提示框
    QFrame* tipFrame = new QFrame();
    tipFrame->setObjectName("tipFrame");
    QVBoxLayout* tipLayout = new QVBoxLayout(tipFrame);
    
    QLabel* tipLabel = new QLabel("💡 提示：返回主菜单点击\"退出登录\"可切换到登录界面");
    tipLabel->setStyleSheet("color: #8B6914; font-size: 12px;");
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setWordWrap(true);
    tipLayout->addWidget(tipLabel);
    
    guestLayout->addWidget(tipFrame);
    guestLayout->addStretch();
    
    mainLayout_->addWidget(guestWidget);
}

void AchievementDialog::loadAchievements()
{
    // 获取所有成就定义
    achievements_ = AchievementManager::instance().getAllAchievements();
    totalAchievements_ = achievements_.size();
    
    // 获取玩家进度
    QList<AchievementProgress> progressList = Database::instance().getAllAchievementProgress(playerId_);
    for (const auto& progress : progressList) {
        progressMap_[progress.achievementId] = progress;
        if (progress.state != AchievementState::LOCKED) {
            completedAchievements_++;
        }
    }
}

void AchievementDialog::createAchievementView()
{
    // 头部信息
    QWidget* headerWidget = new QWidget();
    headerWidget->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #FFF5ED, stop:1 #FFE4D6);
            border: 3px solid #FFD4B8;
            border-radius: 12px;
        }
    )");
    QVBoxLayout* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setSpacing(10);
    
    // 玩家名称
    QLabel* playerLabel = new QLabel(QString("🎮 %1 的成就").arg(playerName_));
    playerLabel->setStyleSheet(R"(
        color: #FF6B35;
        font-size: 18px;
        font-weight: bold;
    )");
    playerLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(playerLabel);
    
    // 进度统计
    QLabel* statsLabel = new QLabel(QString("已完成: %1 / %2")
                                    .arg(completedAchievements_)
                                    .arg(totalAchievements_));
    statsLabel->setStyleSheet("color: #8B6914; font-size: 14px;");
    statsLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(statsLabel);
    
    // 总进度条
    QProgressBar* totalProgress = new QProgressBar();
    totalProgress->setRange(0, totalAchievements_);
    totalProgress->setValue(completedAchievements_);
    totalProgress->setTextVisible(false);
    totalProgress->setFixedHeight(8);
    totalProgress->setStyleSheet(R"(
        QProgressBar {
            background-color: #FFE4D6;
            border: 2px solid #FFD4B8;
            border-radius: 4px;
        }
        QProgressBar::chunk {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #FFB347, stop:1 #FF6B35);
            border-radius: 4px;
        }
    )");
    headerLayout->addWidget(totalProgress);
    
    mainLayout_->addWidget(headerWidget);
    
    // 滚动区域
    scrollArea_ = new QScrollArea();
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    contentWidget_ = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget_);
    contentLayout->setSpacing(15);
    contentLayout->setContentsMargins(5, 5, 5, 5);
    
    // 按类别分组显示
    QList<AchievementCategory> categories = {
        AchievementCategory::BEGINNER,
        AchievementCategory::COMBO,
        AchievementCategory::MULTI_MATCH,
        AchievementCategory::SPECIAL,
        AchievementCategory::SCORE,
        AchievementCategory::PROP,
        AchievementCategory::CHALLENGE,
        AchievementCategory::MILESTONE
    };
    
    for (AchievementCategory cat : categories) {
        QWidget* section = createCategorySection(cat, getCategoryName(cat), getCategoryIcon(cat));
        if (section) {
            contentLayout->addWidget(section);
        }
    }
    
    contentLayout->addStretch();
    scrollArea_->setWidget(contentWidget_);
    mainLayout_->addWidget(scrollArea_, 1);
}

QWidget* AchievementDialog::createCategorySection(AchievementCategory category,
                                                   const QString& categoryName,
                                                   const QString& categoryIcon)
{
    // 筛选该类别的成就
    QList<AchievementDef> categoryAchievements;
    for (const auto& def : achievements_) {
        if (def.category == category) {
            categoryAchievements.append(def);
        }
    }
    
    if (categoryAchievements.isEmpty()) {
        return nullptr;
    }
    
    QWidget* sectionWidget = new QWidget();
    QVBoxLayout* sectionLayout = new QVBoxLayout(sectionWidget);
    sectionLayout->setSpacing(8);
    sectionLayout->setContentsMargins(0, 0, 0, 10);
    
    // 类别标题
    QLabel* categoryLabel = new QLabel(QString("%1 %2").arg(categoryIcon, categoryName));
    categoryLabel->setStyleSheet(R"(
        color: #5A3825;
        font-size: 16px;
        font-weight: bold;
        padding: 5px 0;
    )");
    sectionLayout->addWidget(categoryLabel);
    
    // 成就卡片
    for (const auto& def : categoryAchievements) {
        AchievementProgress progress;
        if (progressMap_.contains(def.id)) {
            progress = progressMap_[def.id];
        } else {
            progress.currentValue = 0;
            progress.targetValue = def.targetValue;
            progress.state = AchievementState::LOCKED;
        }
        
        QWidget* card = createAchievementCard(def, progress);
        sectionLayout->addWidget(card);
    }
    
    return sectionWidget;
}

QWidget* AchievementDialog::createAchievementCard(const AchievementDef& def, 
                                                   const AchievementProgress& progress)
{
    bool isCompleted = (progress.state != AchievementState::LOCKED);
    
    QFrame* card = new QFrame();
    card->setStyleSheet(QString(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 %1, stop:1 %2);
            border-radius: 10px;
            border: 3px solid %3;
            border-left: 5px solid %4;
        }
    )").arg(isCompleted ? "#FFFFFF" : "#FFF5ED",
            isCompleted ? "#FFF5ED" : "#FFE4D6",
            isCompleted ? "#FFD4B8" : "#FFE4D6",
            getRarityColor(def.rarity)));
    
    QHBoxLayout* cardLayout = new QHBoxLayout(card);
    cardLayout->setSpacing(12);
    cardLayout->setContentsMargins(12, 10, 12, 10);
    
    // 左侧：图标或状态
    QLabel* iconLabel = new QLabel(def.icon.isEmpty() ? "🏆" : def.icon);
    iconLabel->setStyleSheet(QString("font-size: 28px; %1")
                             .arg(isCompleted ? "" : "opacity: 0.5;"));
    iconLabel->setFixedWidth(40);
    iconLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(iconLabel);
    
    // 中间：名称和描述
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);
    
    // 名称行（名称 + 稀有度标签）
    QHBoxLayout* nameRow = new QHBoxLayout();
    nameRow->setSpacing(8);
    
    QLabel* nameLabel = new QLabel(def.name);
    nameLabel->setStyleSheet(QString(R"(
        color: %1;
        font-size: 14px;
        font-weight: bold;
    )").arg(isCompleted ? "#4A2815" : "#8B6914"));
    nameRow->addWidget(nameLabel);
    
    // 稀有度标签
    QLabel* rarityLabel = new QLabel(getRarityName(def.rarity));
    rarityLabel->setStyleSheet(QString(R"(
        color: %1;
        font-size: 10px;
        background-color: %2;
        border: 1px solid %1;
        border-radius: 3px;
        padding: 2px 6px;
    )").arg(getRarityColor(def.rarity), 
            isCompleted ? "rgba(255,244,230,0.8)" : "rgba(255,228,214,0.6)"));
    nameRow->addWidget(rarityLabel);
    nameRow->addStretch();
    
    infoLayout->addLayout(nameRow);
    
    // 描述
    QLabel* descLabel = new QLabel(def.description);
    descLabel->setStyleSheet(QString("color: %1; font-size: 11px;")
                             .arg(isCompleted ? "#6A5A4A" : "#8B7355"));
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);
    
    // 进度条（未完成时显示）
    if (!isCompleted) {
        QProgressBar* progressBar = new QProgressBar();
        progressBar->setRange(0, def.targetValue);
        progressBar->setValue(progress.currentValue);
        progressBar->setTextVisible(false);
        progressBar->setFixedHeight(6);
        progressBar->setStyleSheet(QString(R"(
            QProgressBar {
                background-color: #FFE4D6;
                border: 1px solid #FFD4B8;
                border-radius: 3px;
            }
            QProgressBar::chunk {
                background-color: %1;
                border-radius: 3px;
            }
        )").arg(getRarityColor(def.rarity)));
        infoLayout->addWidget(progressBar);
        
        // 进度文字
        QLabel* progressLabel = new QLabel(QString("%1 / %2")
                                           .arg(progress.currentValue)
                                           .arg(def.targetValue));
        progressLabel->setStyleSheet("color: #8B6914; font-size: 10px;");
        infoLayout->addWidget(progressLabel);
    }
    
    cardLayout->addLayout(infoLayout, 1);
    
    // 右侧：奖励或完成标记
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignCenter);
    
    if (isCompleted) {
        QLabel* checkLabel = new QLabel("✅");
        checkLabel->setStyleSheet("font-size: 20px;");
        checkLabel->setAlignment(Qt::AlignCenter);
        rightLayout->addWidget(checkLabel);
    }
    
    QLabel* rewardLabel = new QLabel(QString("+%1").arg(def.reward));
    rewardLabel->setStyleSheet(QString(R"(
        color: %1;
        font-size: 12px;
        font-weight: bold;
    )").arg(isCompleted ? "#FF8C00" : "#CD853F"));
    rewardLabel->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(rewardLabel);
    
    cardLayout->addLayout(rightLayout);
    
    return card;
}

QString AchievementDialog::getRarityColor(AchievementRarity rarity) const
{
    switch (rarity) {
        case AchievementRarity::BRONZE:  return "#cd7f32";
        case AchievementRarity::SILVER:  return "#c0c0c0";
        case AchievementRarity::GOLD:    return "#ffd700";
        case AchievementRarity::DIAMOND: return "#b9f2ff";
        default: return "#808080";
    }
}

QString AchievementDialog::getRarityName(AchievementRarity rarity) const
{
    switch (rarity) {
        case AchievementRarity::BRONZE:  return "青铜";
        case AchievementRarity::SILVER:  return "白银";
        case AchievementRarity::GOLD:    return "黄金";
        case AchievementRarity::DIAMOND: return "钻石";
        default: return "普通";
    }
}

QString AchievementDialog::getCategoryName(AchievementCategory category) const
{
    switch (category) {
        case AchievementCategory::BEGINNER:   return "新手入门";
        case AchievementCategory::COMBO:      return "连击大师";
        case AchievementCategory::MULTI_MATCH: return "多消达人";
        case AchievementCategory::SPECIAL:    return "特殊元素";
        case AchievementCategory::SCORE:      return "得分高手";
        case AchievementCategory::PROP:       return "道具专家";
        case AchievementCategory::CHALLENGE:  return "特殊挑战";
        case AchievementCategory::MILESTONE:  return "里程碑";
        default: return "其他";
    }
}

QString AchievementDialog::getCategoryIcon(AchievementCategory category) const
{
    switch (category) {
        case AchievementCategory::BEGINNER:   return "🌟";
        case AchievementCategory::COMBO:      return "🔥";
        case AchievementCategory::MULTI_MATCH: return "💎";
        case AchievementCategory::SPECIAL:    return "✨";
        case AchievementCategory::SCORE:      return "🎯";
        case AchievementCategory::PROP:       return "🔧";
        case AchievementCategory::CHALLENGE:  return "🏅";
        case AchievementCategory::MILESTONE:  return "🚀";
        default: return "📌";
    }
}
