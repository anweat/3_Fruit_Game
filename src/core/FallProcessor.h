#ifndef FALLPROCESSOR_H
#define FALLPROCESSOR_H

#include "FruitTypes.h"
#include "FruitGenerator.h"
#include <vector>
#include <utility>  // for std::pair

/**
 * @brief 下落处理器 - 处理水果下落和空位填充
 * 
 * 功能：
 * 1. 处理消除后的水果下落
 * 2. 填充新的水果
 * 3. 生成下落动画任务
 * 4. 支持特殊元素的下落
 */
class FallProcessor {
public:
    FallProcessor();
    ~FallProcessor();
    
    /**
     * @brief 处理地图上所有需要下落的水果
     * @param map 游戏地图引用
     * @param generator 水果生成器引用
     * @return 下落步骤列表，每个步骤包含所有同时下落的水果移动记录
     * 
     * 返回格式：vector<vector<pair<from, to>>>
     * - 外层vector：按时间顺序的下落步骤
     * - 内层vector：同一步骤中同时下落的所有移动
     * - pair<from, to>：水果从from位置移动到to位置
     */
    std::vector<std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>>  
        processFall(std::vector<std::vector<Fruit>>& map, FruitGenerator& generator);
    
    /**
     * @brief 处理单列的下落
     * @param map 游戏地图引用
     * @param col 列索引
     * @return 该列的下落步骤列表
     */
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> 
        processColumnFall(std::vector<std::vector<Fruit>>& map, int col);
    
    /**
     * @brief 填充空位
     * @param map 游戏地图引用
     * @param generator 水果生成器引用
     * @return 新填充的水果位置列表
     */
    std::vector<std::pair<int, int>> 
        fillEmptySlots(std::vector<std::vector<Fruit>>& map, FruitGenerator& generator);
    
    /**
     * @brief 检查地图是否有空位
     * @param map 游戏地图
     * @return true表示有空位，false表示无空位
     */
    bool hasEmptySlots(const std::vector<std::vector<Fruit>>& map) const;
    
    /**
     * @brief 获取所有空位的位置
     * @param map 游戏地图
     * @return 空位位置列表
     */
    std::vector<std::pair<int, int>> 
        getEmptySlots(const std::vector<std::vector<Fruit>>& map) const;
    
private:
    /**
     * @brief 计算某个位置下方的空位数量
     * @param map 游戏地图
     * @param row 行索引
     * @param col 列索引
     * @return 下方空位数量
     */
    int countEmptySlotsBelow(const std::vector<std::vector<Fruit>>& map, 
                             int row, int col) const;
};

#endif // FALLPROCESSOR_H
