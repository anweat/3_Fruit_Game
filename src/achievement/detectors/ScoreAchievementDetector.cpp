#include "ScoreAchievementDetector.h"

void ScoreAchievementDetector::detect(
    const GameDataSnapshot& snapshot,
    std::function<void(const QString&, int, int)> callback
)
{
    int score = snapshot.currentScore;
    
    // 单局得分阶梯
    if (score >= 1000) callback("ach_score_1k", score, 1000);
    if (score >= 5000) callback("ach_score_5k", score, 5000);
    if (score >= 10000) callback("ach_score_10k", score, 10000);
    if (score >= 20000) callback("ach_score_20k", score, 20000);
    if (score >= 30000) callback("ach_score_30k", score, 30000);
    if (score >= 50000) callback("ach_score_50k", score, 50000);
    if (score >= 100000) callback("ach_score_100k", score, 100000);
    
    // 单次消除爆发
    if (snapshot.lastMatchScore >= 500) {
        callback("ach_score_burst", snapshot.lastMatchScore, 500);
    }
}
