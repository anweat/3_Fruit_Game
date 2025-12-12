#ifndef MATCHDETECTOR_H
#define MATCHDETECTOR_H

#include "FruitTypes.h"
#include <vector>
#include <set>

/**
 * @brief 匹配检测器类
 * 负责检测游戏地图中的所有匹配（三连及以上）
 */
class MatchDetector {
public:
    MatchDetector();
    ~MatchDetector();
    
    /**
     * @brief 检测整个地图中的所有匹配
     * @param map 游戏地图引用
     * @return 所有匹配结果列表
     */
    std::vector<MatchResult> detectMatches(const std::vector<std::vector<Fruit>>& map);
    
    /**
     * @brief 检测指定位置周围的匹配
     * @param map 游戏地图引用
     * @param row 行坐标
     * @param col 列坐标
     * @return 该位置的匹配结果列表
     */
    std::vector<MatchResult> detectMatchesAt(const std::vector<std::vector<Fruit>>& map,
                                             int row, int col);
    
    /**
     * @brief 检测是否存在任何匹配
     * @param map 游戏地图引用
     * @return true表示存在匹配，false表示无匹配
     */
    bool hasMatches(const std::vector<std::vector<Fruit>>& map);
    
    /**
     * @brief 检测是否存在可能的移动（是否有解）
     * @param map 游戏地图引用
     * @return true表示存在可移动，false表示无解
     */
    bool hasPossibleMoves(const std::vector<std::vector<Fruit>>& map);
    
private:

    /**
     * @brief 检测指定位置周围的匹配（指定水果类型）
     * @param map 游戏地图引用
     * @param row 行坐标
     * @param col 列坐标
     * @param type 水果类型
     * @return 该位置的匹配结果列表
     */
    std::vector<MatchResult> detectTypeMatchesAt(const std::vector<std::vector<Fruit>>& map,
                                                 int row, int col,
                                                 FruitType type);

    /**
     * @brief 检测横向匹配
     * @param map 游戏地图引用
     * @param matched 已匹配位置标记数组
     * @return 横向匹配结果列表
     */
    std::vector<MatchResult> detectHorizontalMatches(
        const std::vector<std::vector<Fruit>>& map,
        bool matched[MAP_SIZE][MAP_SIZE]);
    
    /**
     * @brief 检测纵向匹配
     * @param map 游戏地图引用
     * @param matched 已匹配位置标记数组
     * @return 纵向匹配结果列表
     */
    std::vector<MatchResult> detectVerticalMatches(
        const std::vector<std::vector<Fruit>>& map,
        bool matched[MAP_SIZE][MAP_SIZE]);
    
    /**
     * @brief 合并交叉匹配（L形、T形等）
     * @param results 待合并的匹配结果列表
     * @return 合并后的匹配结果列表
     */
    std::vector<MatchResult> mergeIntersections(std::vector<MatchResult>& results);
    
    /**
     * @brief 确定应生成的特殊元素类型
     * @param count 匹配数量
     * @param direction 匹配方向
     * @return 特殊元素类型
     */
    SpecialType determineSpecialType(int count, MatchDirection direction);
    
    /**
     * @brief 检测两个位置交换后是否会形成匹配
     * @param map 游戏地图引用
     * @param row1 位置1行坐标
     * @param col1 位置1列坐标
     * @param row2 位置2行坐标
     * @param col2 位置2列坐标
     * @return true表示会形成匹配
     */
    bool wouldMatchAfterSwap(const std::vector<std::vector<Fruit>>& map,
                            int row1, int col1, int row2, int col2);

    // 对地图绑定一个可交换位置缓存以优化性能（TODO）
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> possibleSwapCache_;
};

#endif // MATCHDETECTOR_H
