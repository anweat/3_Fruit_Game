#include "LeaderboardDialog.h"
#include "../src/data/RankManager.h"
#include <QHeaderView>
#include <QPushButton>
#include <QFont>

LeaderboardDialog::LeaderboardDialog(const QString& currentPlayerId, QWidget* parent)
    : QDialog(parent)
    , currentPlayerId_(currentPlayerId)
{
    setupUi();
    refreshData();
}

LeaderboardDialog::~LeaderboardDialog()
{
}

void LeaderboardDialog::setupUi()
{
    setWindowTitle("🏆 排行榜");
    setMinimumSize(500, 450);
    setModal(true);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 标题
    QLabel* titleLabel = new QLabel("🏆 比赛排行榜");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // 创建Tab控件
    tabWidget_ = new QTabWidget(this);
    
    // 60秒排行榜
    QWidget* page60s = new QWidget();
    QVBoxLayout* layout60s = new QVBoxLayout(page60s);
    table60s_ = new QTableWidget();
    setupTable(table60s_);
    layout60s->addWidget(table60s_);
    tabWidget_->addTab(page60s, "⏱️ 60秒赛");
    
    // 120秒排行榜
    QWidget* page120s = new QWidget();
    QVBoxLayout* layout120s = new QVBoxLayout(page120s);
    table120s_ = new QTableWidget();
    setupTable(table120s_);
    layout120s->addWidget(table120s_);
    tabWidget_->addTab(page120s, "⏱️ 120秒赛");
    
    // 180秒排行榜
    QWidget* page180s = new QWidget();
    QVBoxLayout* layout180s = new QVBoxLayout(page180s);
    table180s_ = new QTableWidget();
    setupTable(table180s_);
    layout180s->addWidget(table180s_);
    tabWidget_->addTab(page180s, "⏱️ 180秒赛");
    
    mainLayout->addWidget(tabWidget_);
    
    // 刷新按钮和关闭按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* refreshBtn = new QPushButton("🔄 刷新");
    refreshBtn->setMinimumHeight(35);
    connect(refreshBtn, &QPushButton::clicked, this, &LeaderboardDialog::refreshData);
    buttonLayout->addWidget(refreshBtn);
    
    buttonLayout->addStretch();
    
    QPushButton* closeBtn = new QPushButton("关闭");
    closeBtn->setMinimumHeight(35);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeBtn);
    
    mainLayout->addLayout(buttonLayout);
}

void LeaderboardDialog::setupTable(QTableWidget* table)
{
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"排名", "玩家", "得分", "最大连击", "时间"});
    
    // 设置列宽
    table->setColumnWidth(0, 50);   // 排名
    table->setColumnWidth(1, 120);  // 玩家
    table->setColumnWidth(2, 80);   // 得分
    table->setColumnWidth(3, 80);   // 最大连击
    table->setColumnWidth(4, 120);  // 时间
    
    // 设置表格属性
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
}

void LeaderboardDialog::loadLeaderboard(QTableWidget* table, CompetitionDuration duration)
{
    table->setRowCount(0);
    
    QList<RankRecord> records = RankManager::instance().getLeaderboard(duration, 10);
    
    table->setRowCount(records.size());
    
    for (int i = 0; i < records.size(); ++i) {
        const RankRecord& record = records[i];
        
        // 排名
        QTableWidgetItem* rankItem = new QTableWidgetItem(QString::number(record.rank));
        rankItem->setTextAlignment(Qt::AlignCenter);
        // 前三名特殊标记
        if (record.rank == 1) {
            rankItem->setText("🥇");
        } else if (record.rank == 2) {
            rankItem->setText("🥈");
        } else if (record.rank == 3) {
            rankItem->setText("🥉");
        }
        table->setItem(i, 0, rankItem);
        
        // 玩家名
        QTableWidgetItem* nameItem = new QTableWidgetItem(record.playerName);
        // 高亮当前玩家
        if (record.playerId == currentPlayerId_) {
            nameItem->setBackground(QColor(255, 255, 200));
            nameItem->setText("⭐ " + record.playerName);
        }
        table->setItem(i, 1, nameItem);
        
        // 得分
        QTableWidgetItem* scoreItem = new QTableWidgetItem(QString::number(record.score));
        scoreItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 2, scoreItem);
        
        // 最大连击
        QTableWidgetItem* comboItem = new QTableWidgetItem(QString::number(record.maxCombo));
        comboItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 3, comboItem);
        
        // 时间
        QTableWidgetItem* timeItem = new QTableWidgetItem(record.playedAt.toString("MM-dd hh:mm"));
        timeItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 4, timeItem);
    }
    
    // 如果没有数据，显示提示
    if (records.isEmpty()) {
        table->setRowCount(1);
        QTableWidgetItem* emptyItem = new QTableWidgetItem("暂无记录，快来挑战吧！");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(0, 0, emptyItem);
        table->setSpan(0, 0, 1, 5);
    }
}

void LeaderboardDialog::refreshData()
{
    loadLeaderboard(table60s_, CompetitionDuration::SECONDS_60);
    loadLeaderboard(table120s_, CompetitionDuration::SECONDS_120);
    loadLeaderboard(table180s_, CompetitionDuration::SECONDS_180);
}
