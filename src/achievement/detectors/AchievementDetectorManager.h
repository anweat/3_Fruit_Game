#ifndef ACHIEVEMENTDETECTORMANAGER_H
#define ACHIEVEMENTDETECTORMANAGER_H

#include "IAchievementDetector.h"
#include <QVector>
#include <memory>

/**
 * @brief 成就检测器管理类
 * 
 * 负责：
 * 1. 管理所有的成就检测器实例
 * 2. 在每次游戏数据更新时，依次调用所有检测器
 * 3. 统一处理检测结果的回调
 * 
 * 使用组合模式（Composite Pattern）将多个检测器组织在一起，
 * 提供统一的检测接口。
 * 
 * 优点：
 * - 易于添加/移除检测器
 * - 易于查看有哪些检测器在运行
 * - 易于调试（可分别启用/禁用检测器）
 * - 检测流程清晰
 */
class AchievementDetectorManager
{
public:
    AchievementDetectorManager();
    ~AchievementDetectorManager();

    /**
     * @brief 注册一个检测器
     * @param detector 检测器实例（管理器接管所有权）
     */
    void registerDetector(std::unique_ptr<IAchievementDetector> detector);

    /**
     * @brief 注销一个检测器
     * @param detectorName 检测器名称
     */
    void unregisterDetector(const QString& detectorName);

    /**
     * @brief 执行所有检测器的检测
     * @param snapshot 游戏数据快照
     * @param callback 检测结果回调 (achievementId, currentValue, targetValue)
     */
    void detectAll(
        const GameDataSnapshot& snapshot,
        std::function<void(const QString&, int, int)> callback
    );

    /**
     * @brief 获取所有已注册的检测器
     * @return 检测器列表
     */
    const QVector<IAchievementDetector*>& getDetectors() const { return detectors_; }

    /**
     * @brief 获取检测器数量
     */
    int getDetectorCount() const { return detectors_.size(); }

    /**
     * @brief 获取所有检测器负责的成就ID集合
     */
    QSet<QString> getAllResponsibleAchievements() const;

private:
    QVector<IAchievementDetector*> detectors_;  // 所有检测器指针列表
};

#endif // ACHIEVEMENTDETECTORMANAGER_H
