#ifndef LEADERBOARDDIALOG_H
#define LEADERBOARDDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "../src/mode/CompetitionMode.h"

/**
 * @brief 排行榜对话框
 * 
 * 显示三种比赛时长的排行榜
 */
class LeaderboardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LeaderboardDialog(const QString& currentPlayerId, QWidget* parent = nullptr);
    ~LeaderboardDialog();

    /**
     * @brief 刷新排行榜数据
     */
    void refreshData();

private:
    void setupUi();
    void setupTable(QTableWidget* table);
    void loadLeaderboard(QTableWidget* table, CompetitionDuration duration);

    QString currentPlayerId_;
    
    QTabWidget* tabWidget_;
    QTableWidget* table60s_;
    QTableWidget* table120s_;
    QTableWidget* table180s_;
};

#endif // LEADERBOARDDIALOG_H
