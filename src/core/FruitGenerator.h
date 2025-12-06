#ifndef FRUITGENERATOR_H
#define FRUITGENERATOR_H

#include "FruitTypes.h"
#include <random>

/**
 * @brief 水果生成器类
 * 负责生成随机水果，确保初始地图无三连
 */
class FruitGenerator {
public:
    FruitGenerator();
    ~FruitGenerator();
    
    /**
     * @brief 生成单个随机水果类型
     * @return 随机的水果类型 (不包括EMPTY)
     */
    FruitType generateRandomFruit();
    
    /**
     * @brief 生成单个随机水果类型，排除指定类型
     * @param excludeTypes 需要排除的水果类型列表
     * @return 随机的水果类型
     */
    FruitType generateRandomFruit(const std::vector<FruitType>& excludeTypes);
    
    /**
     * @brief 初始化整个游戏地图
     * @param map 游戏地图引用 (8×8二维数组)
     * 确保初始地图没有三连
     */
    void initializeMap(std::vector<std::vector<Fruit>>& map);
    
    /**
     * @brief 在指定位置生成水果，避免立即形成三连
     * @param map 游戏地图引用
     * @param row 行坐标
     * @param col 列坐标
     * @return 生成的水果类型
     */
    FruitType generateSafeFruit(const std::vector<std::vector<Fruit>>& map, int row, int col);
    
    /**
     * @brief 填充地图的空位
     * @param map 游戏地图引用
     * 从上到下填充所有EMPTY位置，确保不会立即形成三连
     */
    void fillEmptySlots(std::vector<std::vector<Fruit>>& map);
    
    /**
     * @brief 重排地图（当无可移动时）
     * @param map 游戏地图引用
     * @param detector 匹配检测器引用，用于检测是否有可移动
     * 打乱现有水果，重新排列，确保无三连且有可移动
     */
    void shuffleMap(std::vector<std::vector<Fruit>>& map, class MatchDetector& detector);
    
    /**
     * @brief 确保地图有可移动（如果无解则自动重排）
     * @param map 游戏地图引用
     * @param detector 匹配检测器引用
     * @return true表示地图有可移动，false表示重排失败（理论上不应该发生）
     */
    bool ensurePlayable(std::vector<std::vector<Fruit>>& map, class MatchDetector& detector);
    
    /**
     * @brief 设置随机种子
     * @param seed 种子值
     */
    void setSeed(unsigned int seed);
    
private:
    std::mt19937 rng_;  // 随机数生成器
    
    /**
     * @brief 检查在指定位置放置指定水果类型是否会立即形成三连
     * @param map 游戏地图引用
     * @param row 行坐标
     * @param col 列坐标
     * @param type 水果类型
     * @return true表示会形成三连，false表示安全
     */
    bool wouldCreateMatch(const std::vector<std::vector<Fruit>>& map, 
                         int row, int col, FruitType type);
};

#endif // FRUITGENERATOR_H
