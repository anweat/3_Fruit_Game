#ifndef MULTIMATCHACHIEVEMENTDETECTOR_H
#define MULTIMATCHACHIEVEMENTDETECTOR_H

#include "IAchievementDetector.h"

/**
 * @brief 多消成就检测器
 * 
 * 负责检测以下成就：
 * - ach_match4_first: 首次4消
 * - ach_match5_first: 首次5消
 * - ach_match6: 首次6消
 * - ach_match8: 首次8消
 * - ach_match4_100/ach_match5_50/ach_match6_20/ach_match8_10: 累计消除数
 * - ach_match5plus_3/ach_match6plus_5: 单局多消次数
 */
class MultiMatchAchievementDetector : public IAchievementDetector
{
public:
    MultiMatchAchievementDetector() 
        : totalMatch4_(0), totalMatch5_(0), 
          totalMatch6_(0), totalMatch8_(0) {}

    void detect(
        const GameDataSnapshot& snapshot,
        std::function<void(const QString&, int, int)> callback
    ) override;

    QString getName() const override { return "MultiMatchAchievementDetector"; }

    QSet<QString> getResponsibleAchievements() const override {
        return {
            "ach_match4_first", "ach_match5_first", 
            "ach_match6", "ach_match8",
            "ach_match4_100", "ach_match5_50", 
            "ach_match6_20", "ach_match8_10",
            "ach_match5plus_3", "ach_match6plus_5"
        };
    }

    // 累计统计数据
    void setTotalMatch4(int total) { totalMatch4_ = total; }
    void setTotalMatch5(int total) { totalMatch5_ = total; }
    void setTotalMatch6(int total) { totalMatch6_ = total; }
    void setTotalMatch8(int total) { totalMatch8_ = total; }

private:
    int totalMatch4_;  // 累计4消次数
    int totalMatch5_;  // 累计5消次数
    int totalMatch6_;  // 累计6消次数
    int totalMatch8_;  // 累计8消次数
};

#endif // MULTIMATCHACHIEVEMENTDETECTOR_H
