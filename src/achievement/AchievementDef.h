#ifndef ACHIEVEMENTDEF_H
#define ACHIEVEMENTDEF_H

#include <QString>

/**
 * @brief 成就稀有度枚举
 */
enum class AchievementRarity {
    BRONZE,   // 青铜
    SILVER,   // 白银
    GOLD,     // 黄金
    DIAMOND   // 钻石
};

/**
 * @brief 成就类别枚举
 */
enum class AchievementCategory {
    BEGINNER,       // 新手入门
    COMBO,          // 连击系列
    MULTI_MATCH,    // 多消系列
    SPECIAL,        // 特殊元素
    SCORE,          // 得分系列
    PROP,           // 道具使用
    CHALLENGE,      // 特殊挑战
    MILESTONE       // 里程碑
};

/**
 * @brief 成就定义结构体
 */
struct AchievementDef {
    QString id;                         // 成就唯一ID
    QString name;                       // 成就名称
    QString description;                // 成就描述
    AchievementCategory category;       // 成就类别
    AchievementRarity rarity;          // 稀有度
    int reward;                         // 点数奖励
    
    // 解锁条件
    int targetValue;                    // 目标值
    QString conditionType;              // 条件类型 (如 "combo", "match", "score")
};

#endif // ACHIEVEMENTDEF_H
