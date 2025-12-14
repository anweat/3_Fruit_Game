#include "AchievementDetectorManager.h"
#include <QDebug>
#include <algorithm>

AchievementDetectorManager::AchievementDetectorManager()
{
}

AchievementDetectorManager::~AchievementDetectorManager()
{
    // 手动清理所有检测器
    for (auto detector : detectors_) {
        delete detector;
    }
    detectors_.clear();
}

void AchievementDetectorManager::registerDetector(std::unique_ptr<IAchievementDetector> detector)
{
    if (!detector) {
        qWarning() << "Trying to register null detector";
        return;
    }
    
    // 释放所有权给指针并存储
    IAchievementDetector* rawPtr = detector.release();
    detectors_.push_back(rawPtr);
    
    qDebug() << "Registered achievement detector:" << rawPtr->getName();
}

void AchievementDetectorManager::unregisterDetector(const QString& detectorName)
{
    // 查找并删除
    auto it = std::find_if(detectors_.begin(), detectors_.end(),
        [&detectorName](IAchievementDetector* detector) {
            return detector->getName() == detectorName;
        });
    
    if (it != detectors_.end()) {
        delete *it;
        detectors_.erase(it);
        qDebug() << "Unregistered achievement detector:" << detectorName;
        return;
    }
    
    qWarning() << "Detector not found:" << detectorName;
}

void AchievementDetectorManager::detectAll(
    const GameDataSnapshot& snapshot,
    std::function<void(const QString&, int, int)> callback
)
{
    // 依次调用所有检测器
    for (auto detector : detectors_) {
        detector->detect(snapshot, callback);
    }
}

QSet<QString> AchievementDetectorManager::getAllResponsibleAchievements() const
{
    QSet<QString> allAchievements;
    
    for (const auto detector : detectors_) {
        allAchievements.unite(detector->getResponsibleAchievements());
    }
    
    return allAchievements;
}
