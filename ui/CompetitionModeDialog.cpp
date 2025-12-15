#include "CompetitionModeDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>

CompetitionModeDialog::CompetitionModeDialog(QWidget* parent)
    : QDialog(parent)
    , selectedDuration_(CompetitionDuration::SECONDS_60)
    , confirmed_(false)
{
    setupUi();
}

CompetitionModeDialog::~CompetitionModeDialog()
{
}

void CompetitionModeDialog::setupUi()
{
    setWindowTitle("选择比赛模式");
    setFixedSize(400, 350);
    setModal(true);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // 标题
    QLabel* titleLabel = new QLabel("🏆 比赛模式");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // 说明
    QLabel* descLabel = new QLabel(
        "比赛模式固定8×8棋盘\n"
        "道具限量：锤子×2 | 夹子×1 | 魔法棒×1\n"
        "成绩计入排行榜"
    );
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setStyleSheet("color: #666; margin: 10px;");
    mainLayout->addWidget(descLabel);
    
    mainLayout->addSpacing(10);
    
    // 60秒按钮
    btn60s_ = new QPushButton("⏱️ 60秒赛");
    btn60s_->setMinimumHeight(50);
    btn60s_->setStyleSheet(
        "QPushButton { "
        "   background-color: #4CAF50; "
        "   color: white; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   border-radius: 8px; "
        "} "
        "QPushButton:hover { background-color: #45a049; }"
    );
    connect(btn60s_, &QPushButton::clicked, this, &CompetitionModeDialog::on60sClicked);
    mainLayout->addWidget(btn60s_);
    
    // 120秒按钮
    btn120s_ = new QPushButton("⏱️ 120秒赛");
    btn120s_->setMinimumHeight(50);
    btn120s_->setStyleSheet(
        "QPushButton { "
        "   background-color: #2196F3; "
        "   color: white; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   border-radius: 8px; "
        "} "
        "QPushButton:hover { background-color: #1976D2; }"
    );
    connect(btn120s_, &QPushButton::clicked, this, &CompetitionModeDialog::on120sClicked);
    mainLayout->addWidget(btn120s_);
    
    // 180秒按钮
    btn180s_ = new QPushButton("⏱️ 180秒赛");
    btn180s_->setMinimumHeight(50);
    btn180s_->setStyleSheet(
        "QPushButton { "
        "   background-color: #FF9800; "
        "   color: white; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   border-radius: 8px; "
        "} "
        "QPushButton:hover { background-color: #F57C00; }"
    );
    connect(btn180s_, &QPushButton::clicked, this, &CompetitionModeDialog::on180sClicked);
    mainLayout->addWidget(btn180s_);
    
    mainLayout->addSpacing(10);
    
    // 取消按钮
    cancelBtn_ = new QPushButton("取消");
    cancelBtn_->setMinimumHeight(40);
    connect(cancelBtn_, &QPushButton::clicked, this, &QDialog::reject);
    mainLayout->addWidget(cancelBtn_);
}

void CompetitionModeDialog::on60sClicked()
{
    selectedDuration_ = CompetitionDuration::SECONDS_60;
    confirmed_ = true;
    accept();
}

void CompetitionModeDialog::on120sClicked()
{
    selectedDuration_ = CompetitionDuration::SECONDS_120;
    confirmed_ = true;
    accept();
}

void CompetitionModeDialog::on180sClicked()
{
    selectedDuration_ = CompetitionDuration::SECONDS_180;
    confirmed_ = true;
    accept();
}
