#ifndef SPECIALEFFECTPROCESSOR_H
#define SPECIALEFFECTPROCESSOR_H

#include "FruitTypes.h"
#include <vector>
#include <set>

/**
 * @brief 特殊效果处理器 - 处理特殊元素的触发效果和组合
 * 
 * 特殊元素效果：
 * 1. 直线炸弹（横）：消除整行
 * 2. 直线炸弹（竖）：消除整列
 * 3. 菱形炸弹：消除5×5菱形范围（曼哈顿距离≤2）
 * 4. 万能炸弹：消除场上所有同类型水果
 * 
 * 组合效果：
 * 1. 直线+直线：十字消除（整行+整列）
 * 2. 直线+菱形：消除3行+3列
 * 3. 菱形+菱形：消除7×7大范围
 * 4. 任意+万能：将场上某类型全部变为该特殊元素并引爆
 * 5. 万能+万能：全屏消除
 */
class SpecialEffectProcessor {
public:
    SpecialEffectProcessor();
    ~SpecialEffectProcessor();
    
    /**
     * @brief 触发特殊元素效果
     * @param map 游戏地图
     * @param row 行坐标
     * @param col 列坐标
     * @param affectedPositions 输出参数，受影响的位置列表（会累积）
     * @return 是否成功触发效果
     */
    bool triggerSpecialEffect(std::vector<std::vector<Fruit>>& map,
                              int row, int col,
                              std::set<std::pair<int, int>>& affectedPositions);
    
    /**
     * @brief 检测并触发特殊元素组合效果
     * @param map 游戏地图
     * @param row1 第一个特殊元素行坐标
     * @param col1 第一个特殊元素列坐标
     * @param row2 第二个特殊元素行坐标
     * @param col2 第二个特殊元素列坐标
     * @param affectedPositions 输出参数，受影响的位置列表
     * @return 是否成功触发组合效果
     */
    bool triggerCombinationEffect(std::vector<std::vector<Fruit>>& map,
                                   int row1, int col1,
                                   int row2, int col2,
                                   std::set<std::pair<int, int>>& affectedPositions);
    
private:
    /**
     * @brief 直线炸弹（横向）效果 - 消除整行
     */
    void effectLineH(std::vector<std::vector<Fruit>>& map,
                     int row,
                     std::set<std::pair<int, int>>& affectedPositions);
    
    /**
     * @brief 直线炸弹（纵向）效果 - 消除整列
     */
    void effectLineV(std::vector<std::vector<Fruit>>& map,
                     int col,
                     std::set<std::pair<int, int>>& affectedPositions);
    
    /**
     * @brief 菱形炸弹效果 - 消除5×5菱形范围
     */
    void effectDiamond(std::vector<std::vector<Fruit>>& map,
                       int row, int col,
                       std::set<std::pair<int, int>>& affectedPositions,
                       int range = 2);
    
    /**
     * @brief 万能炸弹效果 - 消除场上所有同类型水果
     */
    void effectRainbow(std::vector<std::vector<Fruit>>& map,
                       int row, int col,
                       std::set<std::pair<int, int>>& affectedPositions);
    
    /**
     * @brief 组合效果：直线+直线 → 十字消除
     */
    void comboLineLine(std::vector<std::vector<Fruit>>& map,
                       int row, int col,
                       std::set<std::pair<int, int>>& affectedPositions);
    
    /**
     * @brief 组合效果：直线+菱形 → 3行+3列消除
     */
    void comboLineDiamond(std::vector<std::vector<Fruit>>& map,
                          int row, int col,
                          std::set<std::pair<int, int>>& affectedPositions);
    
    /**
     * @brief 组合效果：菱形+菱形 → 7×7大范围消除
     */
    void comboDiamondDiamond(std::vector<std::vector<Fruit>>& map,
                             int row, int col,
                             std::set<std::pair<int, int>>& affectedPositions);
    
    /**
     * @brief 组合效果：任意+万能 → 将场上某类型全部变为该特殊元素并引爆
     */
    void comboSpecialRainbow(std::vector<std::vector<Fruit>>& map,
                             SpecialType specialType,
                             FruitType targetType,
                             std::set<std::pair<int, int>>& affectedPositions);
    
    /**
     * @brief 组合效果：万能+万能 → 全屏消除
     */
    void comboRainbowRainbow(std::vector<std::vector<Fruit>>& map,
                             std::set<std::pair<int, int>>& affectedPositions);
    
    /**
     * @brief 检查位置是否有效
     */
    bool isValidPosition(int row, int col) const;
};

#endif // SPECIALEFFECTPROCESSOR_H
