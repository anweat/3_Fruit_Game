#ifndef COMPETITIONMODEDIALOG_H
#define COMPETITIONMODEDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include "../src/mode/CompetitionMode.h"

/**
 * @brief 比赛模式选择对话框
 * 
 * 让玩家选择比赛时长（60s/120s/180s）
 */
class CompetitionModeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompetitionModeDialog(QWidget* parent = nullptr);
    ~CompetitionModeDialog();

    /**
     * @brief 获取选择的比赛时长
     */
    CompetitionDuration getSelectedDuration() const { return selectedDuration_; }

    /**
     * @brief 是否确认开始
     */
    bool isConfirmed() const { return confirmed_; }

private slots:
    void on60sClicked();
    void on120sClicked();
    void on180sClicked();

private:
    void setupUi();

    CompetitionDuration selectedDuration_;
    bool confirmed_;
    
    QPushButton* btn60s_;
    QPushButton* btn120s_;
    QPushButton* btn180s_;
    QPushButton* cancelBtn_;
};

#endif // COMPETITIONMODEDIALOG_H
