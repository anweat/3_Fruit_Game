#ifndef DETECTORFACTORY_H
#define DETECTORSFACTORY_H

#include "AchievementDetectorManager.h"

/**
 * @brief 检测器工厂
 * 
 * 提供统一的接口来创建和初始化所有成就检测器。
 * 这样做的好处是：
 * 1. 检测器的创建逻辑集中管理
 * 2. 易于添加新的检测器（只需在工厂中添加一行代码）
 * 3. 易于配置检测器的参数
 */
class DetectorFactory
{
public:
    /**
     * @brief 创建并初始化所有检测器
     * @return 初始化好的检测器管理器
     * 
     * 包含以下8个检测器：
     * 1. BeginnerAchievementDetector - 初学者成就
     * 2. ComboAchievementDetector - 连击成就
     * 3. MultiMatchAchievementDetector - 多消成就
     * 4. SpecialAchievementDetector - 特殊元素成就
     * 5. ScoreAchievementDetector - 得分成就
     * 6. PropAchievementDetector - 道具成就
     * 7. ChallengeAchievementDetector - 挑战成就
     * 8. MilestoneAchievementDetector - 里程碑成就
     */
    static std::unique_ptr<AchievementDetectorManager> createDetectorManager();

    /**
     * @brief 注册玩家相关的统计数据到检测器
     * @param manager 检测器管理器
     * @param playerId 玩家ID
     * @param totalGames 总游戏局数
     * @param comboStats 连击统计
     * @param matchStats 消除统计
     * @param specialStats 特殊元素统计
     * @param propStats 道具统计
     */
    static void registerPlayerStats(
        AchievementDetectorManager* manager,
        const QString& playerId,
        int totalGames,
        int totalCombo3Plus,
        int totalMatch4, int totalMatch5, int totalMatch6, int totalMatch8,
        int totalSpecialGenerated, int totalLineGenerated, 
        int totalDiamondGenerated, int totalRainbowGenerated, int totalSpecialUsed,
        int totalPropUsed
    );
};

#endif // DETECTORSFACTORY_H
