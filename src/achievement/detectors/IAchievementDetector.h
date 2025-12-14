#ifndef IACHIEVEMENTDETECTOR_H
#define IACHIEVEMENTDETECTOR_H

#include "../AchievementManager.h"
#include <QString>
#include <QSet>
#include <functional>

/**
 * @brief 成就检测器抽象基类
 * 
 * 所有成就检测器都继承此接口，实现特定类别的成就检测逻辑。
 * 通过策略模式（Strategy Pattern）实现成就检测的模块化和可扩展性。
 * 
 * 优势：
 * - 易于添加新的成就检测器（只需新建一个类）
 * - 易于修改现有成就的判定逻辑（只需修改对应检测器）
 * - 检测器之间相互独立，不互相影响
 * - 便于单元测试（可独立测试每个检测器）
 * - 符合开闭原则（对扩展开放，对修改关闭）
 */
class IAchievementDetector
{
public:
    virtual ~IAchievementDetector() = default;

    /**
     * @brief 执行成就检测
     * @param snapshot 游戏数据快照
     * @param callback 成就完成回调函数 (achievementId, currentValue, targetValue)
     * 
     * 检测器应该检查snapshot中的数据，判断是否有成就达成条件被触发。
     * 当成就达成条件满足时，通过callback函数通知管理器。
     * 
     * callback签名: void(const QString& achievementId, int currentValue, int targetValue)
     * 例如: callback("ach_combo_3", 5, 3) 表示连击成就达成（当前5 >= 目标3）
     */
    virtual void detect(
        const GameDataSnapshot& snapshot,
        std::function<void(const QString&, int, int)> callback
    ) = 0;

    /**
     * @brief 获取检测器名称
     * @return 检测器名称（用于调试和日志）
     */
    virtual QString getName() const = 0;

    /**
     * @brief 返回此检测器负责的成就ID列表
     * @return 成就ID集合
     */
    virtual QSet<QString> getResponsibleAchievements() const = 0;
};

#endif // IACHIEVEMENTDETECTOR_H
