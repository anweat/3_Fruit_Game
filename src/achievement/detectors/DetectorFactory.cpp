#include "DetectorFactory.h"
#include "BeginnerAchievementDetector.h"
#include "ComboAchievementDetector.h"
#include "MultiMatchAchievementDetector.h"
#include "SpecialAchievementDetector.h"
#include "ScoreAchievementDetector.h"
#include "PropAchievementDetector.h"
#include "ChallengeAchievementDetector.h"
#include "MilestoneAchievementDetector.h"

std::unique_ptr<AchievementDetectorManager> DetectorFactory::createDetectorManager()
{
    auto manager = std::make_unique<AchievementDetectorManager>();
    
    // 注册8个检测器
    manager->registerDetector(std::make_unique<BeginnerAchievementDetector>());
    manager->registerDetector(std::make_unique<ComboAchievementDetector>());
    manager->registerDetector(std::make_unique<MultiMatchAchievementDetector>());
    manager->registerDetector(std::make_unique<SpecialAchievementDetector>());
    manager->registerDetector(std::make_unique<ScoreAchievementDetector>());
    manager->registerDetector(std::make_unique<PropAchievementDetector>());
    manager->registerDetector(std::make_unique<ChallengeAchievementDetector>());
    manager->registerDetector(std::make_unique<MilestoneAchievementDetector>());
    
    return manager;
}

void DetectorFactory::registerPlayerStats(
    AchievementDetectorManager* manager,
    const QString& playerId,
    int totalGames,
    int totalCombo3Plus,
    int totalMatch4, int totalMatch5, int totalMatch6, int totalMatch8,
    int totalSpecialGenerated, int totalLineGenerated, 
    int totalDiamondGenerated, int totalRainbowGenerated, int totalSpecialUsed,
    int totalPropUsed
)
{
    const auto& detectors = manager->getDetectors();
    
    // 向各个检测器设置统计数据
    for (auto detector : detectors) {
        if (detector->getName() == "ComboAchievementDetector") {
            auto combo = dynamic_cast<ComboAchievementDetector*>(detector);
            if (combo) combo->setTotalCombo3Plus(totalCombo3Plus);
        }
        else if (detector->getName() == "MultiMatchAchievementDetector") {
            auto multi = dynamic_cast<MultiMatchAchievementDetector*>(detector);
            if (multi) {
                multi->setTotalMatch4(totalMatch4);
                multi->setTotalMatch5(totalMatch5);
                multi->setTotalMatch6(totalMatch6);
                multi->setTotalMatch8(totalMatch8);
            }
        }
        else if (detector->getName() == "SpecialAchievementDetector") {
            auto special = dynamic_cast<SpecialAchievementDetector*>(detector);
            if (special) {
                special->setTotalSpecialGenerated(totalSpecialGenerated);
                special->setTotalLineGenerated(totalLineGenerated);
                special->setTotalDiamondGenerated(totalDiamondGenerated);
                special->setTotalRainbowGenerated(totalRainbowGenerated);
                special->setTotalSpecialUsed(totalSpecialUsed);
            }
        }
        else if (detector->getName() == "PropAchievementDetector") {
            auto prop = dynamic_cast<PropAchievementDetector*>(detector);
            if (prop) prop->setTotalPropUsed(totalPropUsed);
        }
        else if (detector->getName() == "MilestoneAchievementDetector") {
            auto milestone = dynamic_cast<MilestoneAchievementDetector*>(detector);
            if (milestone) {
                milestone->setTotalGames(totalGames);
                milestone->setCurrentPlayerId(playerId);
            }
        }
    }
}
