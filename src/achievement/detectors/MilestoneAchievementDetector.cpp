#include "MilestoneAchievementDetector.h"
#include "../AchievementManager.h"
#include "../../data/Database.h"

void MilestoneAchievementDetector::detect(
    const GameDataSnapshot& snapshot,
    std::function<void(const QString&, int, int)> callback
)
{
    // 游戏局数里程碑
    callback("ach_first_game", totalGames_, 1);
    callback("ach_5_games", totalGames_, 5);
    callback("ach_milestone_10games", totalGames_, 10);
    callback("ach_milestone_50games", totalGames_, 50);
    callback("ach_milestone_100games", totalGames_, 100);
    
    // 全类型消除
    if (snapshot.fruitTypesEliminated.size() >= 6) {
        callback("ach_milestone_allfruits", snapshot.fruitTypesEliminated.size(), 6);
    }
    
    // 点数富翁（从数据库查询）
    if (!currentPlayerId_.isEmpty()) {
        PlayerData player = Database::instance().getPlayer(currentPlayerId_);
        if (player.totalPoints >= 5000) {
            callback("ach_milestone_5000points", player.totalPoints, 5000);
        }
        
        // 水果大师（从数据库查询已完成成就数）
        int completedCount = Database::instance().getCompletedAchievementCount(currentPlayerId_);
        if (completedCount >= 61) {
            callback("ach_milestone_master", completedCount, 61);
        }
    }
}
