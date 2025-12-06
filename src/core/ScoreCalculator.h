#ifndef SCORECALCULATOR_H
#define SCORECALCULATOR_H

#include "FruitTypes.h"
#include <vector>

/**
 * @brief 计分系统 - 计算消除得分和连击奖励
 * 
 * 计分规则：
 * 1. 基础分：每个水果10分
 * 2. 匹配数量奖励：3消=30分, 4消=80分, 5消=200分, 6消及以上=300+50*(n-6)分
 * 3. 特殊元素奖励：直线炸弹+50分, 菱形炸弹+100分, 万能炸弹+150分
 * 4. 连击奖励：连击系数 = 连击次数，总分 = 基础分 × 连击系数
 * 5. L形/T形奖励：额外+100分
 */
class ScoreCalculator {
public:
    ScoreCalculator();
    ~ScoreCalculator();
    
    /**
     * @brief 计算单次匹配的得分
     * @param match 匹配结果
     * @param comboMultiplier 连击倍数（默认1）
     * @return 该次匹配的总得分
     */
    int calculateMatchScore(const MatchResult& match, int comboMultiplier = 1) const;
    
    /**
     * @brief 计算多个匹配的总得分
     * @param matches 所有匹配结果
     * @param comboMultiplier 连击倍数（默认1）
     * @return 总得分
     */
    int calculateTotalScore(const std::vector<MatchResult>& matches, 
                            int comboMultiplier = 1) const;
    
    /**
     * @brief 根据匹配数量计算基础分
     * @param matchCount 匹配的水果数量
     * @return 基础分
     */
    int getBaseScore(int matchCount) const;
    
    /**
     * @brief 根据特殊元素类型计算奖励分
     * @param specialType 特殊元素类型
     * @return 奖励分
     */
    int getSpecialBonus(SpecialType specialType) const;
    
    /**
     * @brief 计算L形/T形奖励
     * @param match 匹配结果
     * @return 奖励分
     */
    int getShapeBonus(const MatchResult& match) const;
    
    /**
     * @brief 重置连击计数器
     */
    void resetCombo();
    
    /**
     * @brief 增加连击计数器
     * @return 当前连击数
     */
    int incrementCombo();
    
    /**
     * @brief 获取当前连击数
     * @return 连击数
     */
    int getComboCount() const;
    
private:
    int comboCount;  ///< 当前连击数
    
    static const int BASE_SCORE_PER_FRUIT = 10;        ///< 每个水果基础分
    static const int SCORE_THREE_MATCH = 30;           ///< 3消基础分
    static const int SCORE_FOUR_MATCH = 80;            ///< 4消基础分
    static const int SCORE_FIVE_MATCH = 200;           ///< 5消基础分
    static const int SCORE_SIX_PLUS_BASE = 300;        ///< 6消及以上基础分
    static const int SCORE_SIX_PLUS_INCREMENT = 50;    ///< 6消以上每增加1个的额外分
    
    static const int BONUS_LINE_BOMB = 50;             ///< 直线炸弹奖励
    static const int BONUS_DIAMOND_BOMB = 100;         ///< 菱形炸弹奖励
    static const int BONUS_RAINBOW_BOMB = 150;         ///< 万能炸弹奖励
    static const int BONUS_L_T_SHAPE = 100;            ///< L形/T形奖励
};

#endif // SCORECALCULATOR_H
