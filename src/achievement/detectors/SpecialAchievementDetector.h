#ifndef SPECIALACHIEVEMENTDETECTOR_H
#define SPECIALACHIEVEMENTDETECTOR_H

#include "IAchievementDetector.h"

/**
 * @brief 特殊元素成就检测器
 * 
 * 负责检测以下成就：
 * - ach_special_first: 首次生成特殊元素
 * - ach_line_50/ach_diamond_30/ach_rainbow_20: 特定炸弹生成累计
 * - ach_special_200: 累计使用特殊元素
 * - ach_combo_line_line/line_diamond/diamond_diamond/any_rainbow/rainbow_rainbow: 炸弹组合
 */
class SpecialAchievementDetector : public IAchievementDetector
{
public:
    SpecialAchievementDetector() 
        : totalSpecialGenerated_(0), totalLineGenerated_(0),
          totalDiamondGenerated_(0), totalRainbowGenerated_(0),
          totalSpecialUsed_(0) {}

    void detect(
        const GameDataSnapshot& snapshot,
        std::function<void(const QString&, int, int)> callback
    ) override;

    QString getName() const override { return "SpecialAchievementDetector"; }

    QSet<QString> getResponsibleAchievements() const override {
        return {
            "ach_special_first", "ach_line_50", 
            "ach_diamond_30", "ach_rainbow_20",
            "ach_special_200",
            "ach_combo_line_line", "ach_combo_line_diamond",
            "ach_combo_diamond_diamond", "ach_combo_any_rainbow",
            "ach_combo_rainbow_rainbow"
        };
    }

    // 累计统计数据
    void setTotalSpecialGenerated(int total) { totalSpecialGenerated_ = total; }
    void setTotalLineGenerated(int total) { totalLineGenerated_ = total; }
    void setTotalDiamondGenerated(int total) { totalDiamondGenerated_ = total; }
    void setTotalRainbowGenerated(int total) { totalRainbowGenerated_ = total; }
    void setTotalSpecialUsed(int total) { totalSpecialUsed_ = total; }

private:
    int totalSpecialGenerated_;    // 累计生成特殊元素次数
    int totalLineGenerated_;       // 累计生成直线炸弹次数
    int totalDiamondGenerated_;    // 累计生成菱形炸弹次数
    int totalRainbowGenerated_;    // 累计生成万能炸弹次数
    int totalSpecialUsed_;         // 累计使用特殊元素次数
};

#endif // SPECIALACHIEVEMENTDETECTOR_H
