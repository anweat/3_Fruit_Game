#include "ScoreCalculator.h"

ScoreCalculator::ScoreCalculator() : comboCount(0) {
    // 构造函数 - 初始化连击计数为0
}

ScoreCalculator::~ScoreCalculator() {
    // 析构函数 - 无需清理
}

/**
 * @brief 计算单次匹配的得分
 */
int ScoreCalculator::calculateMatchScore(const MatchResult& match, int comboMultiplier) const {
    // 1. 计算基础分
    int baseScore = getBaseScore(match.matchCount);
    
    // 2. 计算特殊元素奖励
    int specialBonus = getSpecialBonus(match.generateSpecial);
    
    // 3. 计算形状奖励（L形/T形）
    int shapeBonus = getShapeBonus(match);
    
    // 4. 应用连击倍数
    int totalScore = (baseScore + specialBonus + shapeBonus) * comboMultiplier;
    
    return totalScore;
}

/**
 * @brief 计算多个匹配的总得分
 */
int ScoreCalculator::calculateTotalScore(const std::vector<MatchResult>& matches, 
                                         int comboMultiplier) const {
    int totalScore = 0;
    
    for (const auto& match : matches) {
        totalScore += calculateMatchScore(match, comboMultiplier);
    }
    
    return totalScore;
}

/**
 * @brief 根据匹配数量计算基础分
 */
int ScoreCalculator::getBaseScore(int matchCount) const {
    if (matchCount < 3) {
        return 0;  // 少于3个不计分
    }
    
    switch (matchCount) {
        case 3:
            return SCORE_THREE_MATCH;
        case 4:
            return SCORE_FOUR_MATCH;
        case 5:
            return SCORE_FIVE_MATCH;
        default:
            // 6个及以上：300 + 50 * (n - 6)
            return SCORE_SIX_PLUS_BASE + SCORE_SIX_PLUS_INCREMENT * (matchCount - 6);
    }
}

/**
 * @brief 根据特殊元素类型计算奖励分
 */
int ScoreCalculator::getSpecialBonus(SpecialType specialType) const {
    switch (specialType) {
        case SpecialType::LINE_H:
        case SpecialType::LINE_V:
            return BONUS_LINE_BOMB;
        case SpecialType::DIAMOND:
            return BONUS_DIAMOND_BOMB;
        case SpecialType::RAINBOW:
            return BONUS_RAINBOW_BOMB;
        case SpecialType::NONE:
        default:
            return 0;
    }
}

/**
 * @brief 计算L形/T形奖励
 */
int ScoreCalculator::getShapeBonus(const MatchResult& match) const {
    // 如果生成菱形炸弹，说明是L形或T形匹配
    if (match.generateSpecial == SpecialType::DIAMOND) {
        return BONUS_L_T_SHAPE;
    }
    return 0;
}

/**
 * @brief 重置连击计数器
 */
void ScoreCalculator::resetCombo() {
    comboCount = 0;
}

/**
 * @brief 增加连击计数器
 */
int ScoreCalculator::incrementCombo() {
    comboCount++;
    return comboCount;
}

/**
 * @brief 获取当前连击数
 */
int ScoreCalculator::getComboCount() const {
    return comboCount;
}
