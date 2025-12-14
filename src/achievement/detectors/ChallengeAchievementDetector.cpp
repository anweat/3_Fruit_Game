#include "ChallengeAchievementDetector.h"
#include <QDateTime>

void ChallengeAchievementDetector::detect(
    const GameDataSnapshot& snapshot,
    std::function<void(const QString&, int, int)> callback
)
{
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - snapshot.gameStartTime;
    int elapsedSeconds = elapsed / 1000;
    
    // 速战速决 - 60秒内得分超过10000
    if (elapsedSeconds <= 60 && snapshot.currentScore >= 10000) {
        callback("ach_challenge_60s", snapshot.currentScore, 10000);
    }
    
    // 闪电手 - 30秒内完成20次消除
    if (elapsedSeconds <= 30 && snapshot.eliminateCount >= 20) {
        callback("ach_challenge_flash", snapshot.eliminateCount, 20);
    }
    
    // 无道具挑战
    if (!snapshot.propUsed && snapshot.currentScore >= 20000) {
        callback("ach_challenge_noprop", snapshot.currentScore, 20000);
    }
    
    // 连锁反应
    if (snapshot.chainReactionCount >= 5) {
        callback("ach_challenge_chain5", snapshot.chainReactionCount, 5);
    }
    
    // 地图清理
    if (snapshot.specialEliminateCount >= 30) {
        callback("ach_challenge_30fruits", snapshot.specialEliminateCount, 30);
    }
    
    // 持久战
    if (elapsedSeconds >= 600) {
        callback("ach_challenge_marathon", elapsedSeconds, 600);
    }
}
