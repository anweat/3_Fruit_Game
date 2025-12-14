#ifndef ACHIEVEMENTDIALOG_H
#define ACHIEVEMENTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QFrame>
#include <QMap>
#include "../src/achievement/AchievementDef.h"
#include "../src/data/Database.h"

/**
 * @brief 成就展示对话框
 * 
 * 显示玩家的成就列表和完成进度
 * - 登录用户：显示完整成就列表和进度
 * - 游客模式：提示需要登录才能保存和查看成就
 */
class AchievementDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param playerId 玩家ID（空字符串表示游客模式）
     * @param playerName 玩家名称
     * @param parent 父窗口
     */
    explicit AchievementDialog(const QString& playerId, 
                               const QString& playerName,
                               QWidget* parent = nullptr);
    ~AchievementDialog();

private:
    void setupUi();
    void setupStyleSheet();
    void loadAchievements();
    void createGuestModeView();
    void createAchievementView();
    QWidget* createAchievementCard(const AchievementDef& def, 
                                   const AchievementProgress& progress);
    QWidget* createCategorySection(AchievementCategory category,
                                   const QString& categoryName,
                                   const QString& categoryIcon);
    
    QString getRarityColor(AchievementRarity rarity) const;
    QString getRarityName(AchievementRarity rarity) const;
    QString getCategoryName(AchievementCategory category) const;
    QString getCategoryIcon(AchievementCategory category) const;
    
    QString playerId_;
    QString playerName_;
    bool isGuest_;
    
    QVBoxLayout* mainLayout_;
    QScrollArea* scrollArea_;
    QWidget* contentWidget_;
    
    // 成就数据
    QMap<QString, AchievementDef> achievements_;
    QMap<QString, AchievementProgress> progressMap_;
    
    // 统计数据
    int totalAchievements_;
    int completedAchievements_;
};

#endif // ACHIEVEMENTDIALOG_H
