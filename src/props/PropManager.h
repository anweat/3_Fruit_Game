#ifndef PROPMANAGER_H
#define PROPMANAGER_H

#include <vector>
#include <set>
#include <utility>
#include "../core/FruitTypes.h"

/**
 * @brief 道具类型枚举
 */
enum class PropType {
    HAMMER,      ///< 锤子：消除单个水果
    CLAMP,       ///< 夹子：交换任意两个相邻水果
    MAGIC_WAND   ///< 魔法棒：消除整个类型的水果
};

/**
 * @brief 道具管理器
 * 
 * 职责：
 * 1. 管理玩家的道具库存
 * 2. 处理道具使用逻辑
 * 3. 计算道具影响范围
 */
class PropManager {
public:
    PropManager();
    ~PropManager();
    
    /**
     * @brief 检查是否有足够的道具
     * @param propType 道具类型
     * @return 是否有足够的道具
     */
    bool hasProp(PropType propType) const;
    
    /**
     * @brief 获取道具数量
     * @param propType 道具类型
     * @return 道具数量
     */
    int getPropCount(PropType propType) const;
    
    /**
     * @brief 添加道具
     * @param propType 道具类型
     * @param count 数量
     */
    void addProp(PropType propType, int count = 1);
    
    /**
     * @brief 设置道具数量
     * @param propType 道具类型
     * @param count 数量
     */
    void setPropCount(PropType propType, int count);
    
    /**
     * @brief 设置所有道具数量
     */
    void setAllProps(int hammer, int clamp, int magicWand);
    
    /**
     * @brief 使用锤子道具
     * @param map 游戏地图
     * @param row 目标行
     * @param col 目标列
     * @param outAffected 输出受影响的位置
     * @return 是否使用成功
     */
    bool useHammer(const std::vector<std::vector<Fruit>>& map,
                   int row, int col,
                   std::set<std::pair<int, int>>& outAffected);
    
    /**
     * @brief 使用夹子道具
     * @param map 游戏地图
     * @param row1 第一个位置行
     * @param col1 第一个位置列
     * @param row2 第二个位置行
     * @param col2 第二个位置列
     * @return 是否使用成功
     */
    bool useClamp(const std::vector<std::vector<Fruit>>& map,
                  int row1, int col1,
                  int row2, int col2);
    
    /**
     * @brief 使用魔法棒道具
     * @param map 游戏地图
     * @param row 目标行
     * @param col 目标列
     * @param outAffected 输出受影响的位置
     * @return 是否使用成功
     */
    bool useMagicWand(const std::vector<std::vector<Fruit>>& map,
                      int row, int col,
                      std::set<std::pair<int, int>>& outAffected);
    
private:
    int hammerCount_;      ///< 锤子数量
    int clampCount_;       ///< 夹子数量
    int magicWandCount_;   ///< 魔法棒数量
};

#endif // PROPMANAGER_H
