#ifndef MILESTONEACHIEVEMENTDETECTOR_H
#define MILESTONEACHIEVEMENTDETECTOR_H

#include "IAchievementDetector.h"

/**
 * @brief 里程碑成就检测器
 * 
 * 负责检测以下成就：
 * - ach_first_game/5_games/milestone_10games/50games/100games: 游戏局数里程碑
 * - ach_milestone_allfruits: 全类型消除 (6种水果)
 * - ach_milestone_5000points: 点数富翁 (5000点)
 * - ach_milestone_master: 水果大师 (61个成就)
 */
class MilestoneAchievementDetector : public IAchievementDetector
{
public:
    MilestoneAchievementDetector() 
        : totalGames_(0), currentPlayerId_("") {}

    void detect(
        const GameDataSnapshot& snapshot,
        std::function<void(const QString&, int, int)> callback
    ) override;

    QString getName() const override { return "MilestoneAchievementDetector"; }

    QSet<QString> getResponsibleAchievements() const override {
        return {
            "ach_first_game", "ach_5_games",
            "ach_milestone_10games", "ach_milestone_50games",
            "ach_milestone_100games", "ach_milestone_allfruits",
            "ach_milestone_5000points", "ach_milestone_master"
        };
    }

    // 设置统计数据
    void setTotalGames(int total) { totalGames_ = total; }
    void setCurrentPlayerId(const QString& id) { currentPlayerId_ = id; }

private:
    int totalGames_;           // 累计游戏局数
    QString currentPlayerId_;  // 当前玩家ID（用于数据库查询）
};

#endif // MILESTONEACHIEVEMENTDETECTOR_H
