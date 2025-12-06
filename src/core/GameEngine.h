#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "FruitTypes.h"
#include "FruitGenerator.h"
#include "MatchDetector.h"
#include "FallProcessor.h"
#include "ScoreCalculator.h"
#include "SpecialFruitGenerator.h"
#include "SpecialEffectProcessor.h"
#include <vector>
#include <set>

/**
 * @brief 游戏状态枚举
 */
enum class GameState {
    IDLE,           // 空闲状态（等待玩家操作）
    SWAPPING,       // 交换中
    MATCHING,       // 匹配检测中
    ELIMINATING,    // 消除中
    FALLING,        // 下落中
    PROCESSING      // 处理连锁反应中
};

/**
 * @brief 游戏引擎 - 整合所有子系统，管理游戏主循环
 * 
 * 职责：
 * 1. 管理游戏地图
 * 2. 处理玩家交换操作
 * 3. 检测和处理匹配
 * 4. 处理特殊元素效果
 * 5. 管理下落和填充
 * 6. 计算得分和连击
 * 7. 检测死局并重排
 */
class GameEngine {
public:
    GameEngine();
    ~GameEngine();
    
    /**
     * @brief 初始化游戏（创建地图）
     */
    void initializeGame();
    
    /**
     * @brief 尝试交换两个水果
     * @param row1 第一个水果的行
     * @param col1 第一个水果的列
     * @param row2 第二个水果的行
     * @param col2 第二个水果的列
     * @return 交换是否成功
     */
    bool swapFruits(int row1, int col1, int row2, int col2);
    
    /**
     * @brief 处理一轮游戏循环（匹配→消除→下落→再匹配）
     * @return 是否有消除发生
     */
    bool processGameCycle();
    
    /**
     * @brief 获取当前地图
     */
    const std::vector<std::vector<Fruit>>& getMap() const { return map_; }
    
    /**
     * @brief 获取当前分数
     */
    int getCurrentScore() const { return currentScore_; }
    
    /**
     * @brief 获取连击数
     */
    int getComboCount() const { return scoreCalculator_.getComboCount(); }
    
    /**
     * @brief 获取游戏状态
     */
    GameState getState() const { return state_; }
    
    /**
     * @brief 检查地图是否有可移动
     */
    bool hasValidMoves() const;
    
    /**
     * @brief 重置游戏
     */
    void resetGame();
    
private:
    /**
     * @brief 检测并处理所有匹配
     * @return 匹配的数量
     */
    int detectAndProcessMatches();
    
    /**
     * @brief 处理特殊元素生成
     * @param matches 匹配结果列表
     */
    void processSpecialGeneration(const std::vector<MatchResult>& matches);
    
    /**
     * @brief 消除匹配的水果
     * @param matches 匹配结果列表
     */
    void eliminateMatches(const std::vector<MatchResult>& matches);
    
    /**
     * @brief 处理下落和填充空位
     * @return 是否有水果下落
     */
    bool processFallAndRefill();
    
    /**
     * @brief 检查并处理死局
     */
    void checkAndHandleDeadlock();
    
    /**
     * @brief 验证交换是否合法（是否相邻）
     */
    bool isValidSwap(int row1, int col1, int row2, int col2) const;
    
private:
    // 子系统
    FruitGenerator fruitGenerator_;              ///< 水果生成器
    MatchDetector matchDetector_;                ///< 匹配检测器
    FallProcessor fallProcessor_;                ///< 下落处理器
    ScoreCalculator scoreCalculator_;            ///< 计分系统
    SpecialFruitGenerator specialGenerator_;     ///< 特殊元素生成器
    SpecialEffectProcessor specialProcessor_;    ///< 特殊效果处理器
    
    // 游戏数据
    std::vector<std::vector<Fruit>> map_;        ///< 游戏地图 (8×8)
    GameState state_;                            ///< 当前游戏状态
    int currentScore_;                           ///< 当前分数
    int totalMatches_;                           ///< 总匹配次数（统计用）
};

#endif // GAMEENGINE_H
