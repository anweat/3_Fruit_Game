#ifndef SPECIALFRUITGENERATOR_H
#define SPECIALFRUITGENERATOR_H

#include "FruitTypes.h"
#include <vector>

/**
 * @brief 特殊水果生成器 - 根据匹配条件判断并生成特殊元素
 * 
 * 生成规则：
 * 1. 4个水果横向消除 → 横向直线炸弹 (LINE_H)
 * 2. 4个水果纵向消除 → 纵向直线炸弹 (LINE_V)
 * 3. L形或T形5个水果消除 → 菱形炸弹 (DIAMOND)
 * 4. 5个及以上直线消除 → 万能炸弹 (RAINBOW)
 */
class SpecialFruitGenerator {
public:
    SpecialFruitGenerator();
    ~SpecialFruitGenerator();
    
    /**
     * @brief 根据匹配结果判断应该生成什么特殊元素
     * @param match 匹配结果
     * @return 应该生成的特殊元素类型
     */
    SpecialType determineSpecialType(const MatchResult& match) const;
    
    /**
     * @brief 在地图上生成特殊水果
     * @param map 游戏地图
     * @param match 匹配结果
     * @param specialType 要生成的特殊元素类型
     * @return 生成位置 (row, col)，如果无法生成返回 {-1, -1}
     */
    std::pair<int, int> generateSpecialFruit(std::vector<std::vector<Fruit>>& map,
                                               const MatchResult& match,
                                               SpecialType specialType);
    
    /**
     * @brief 检测L形或T形匹配
     * @param map 游戏地图
     * @param fruitType 水果类型
     * @param row 起始行
     * @param col 起始列
     * @return 如果检测到L/T形返回true，同时填充positions
     */
    bool detectLTShape(const std::vector<std::vector<Fruit>>& map,
                       FruitType fruitType,
                       int row, int col,
                       std::vector<std::pair<int, int>>& positions) const;
    
private:
    /**
     * @brief 计算特殊水果应该生成的位置（通常是匹配序列的中心位置）
     * @param match 匹配结果
     * @return 生成位置 (row, col)
     */
    std::pair<int, int> calculateGeneratePosition(const MatchResult& match) const;
};

#endif // SPECIALFRUITGENERATOR_H
