#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "FruitTypes.h"
#include "FruitGenerator.h"
#include "MatchDetector.h"
#include "FallProcessor.h"
#include "ScoreCalculator.h"
#include "SpecialFruitGenerator.h"
#include "SpecialEffectProcessor.h"
#include "SwapHandler.h"
#include "AnimationRecorder.h"
#include "GameCycleProcessor.h"
#include <set>
#include <vector>

/**
 * @brief 单次交换步骤信息（用于动画与成就统计）
 */
struct SwapStep {
    int row1 = -1;
    int col1 = -1;
    int row2 = -1;
    int col2 = -1;
    bool success = false; ///< 是否交换成功（成功则进入消除流程，失败用于回弹动画）
};

/**
 * @brief 炸弹特效类型
 */
enum class BombEffectType {
    NONE,
    LINE_H,     ///< 横排消除（白色长条覆盖一行）
    LINE_V,     ///< 竖排消除（白色长条覆盖一列）
    DIAMOND,    ///< 菱形消除（白色正方形扩散）
    RAINBOW     ///< 彩虹消除（全屏闪光）
};

/**
 * @brief 单个炸弹特效信息
 */
struct BombEffect {
    BombEffectType type = BombEffectType::NONE;
    int row = -1;       ///< 中心行（LINE_H 使用此行，DIAMOND/RAINBOW 使用此作为中心）
    int col = -1;       ///< 中心列（LINE_V 使用此列，DIAMOND/RAINBOW 使用此作为中心）
    int range = 2;      ///< 范围（菱形炸弹使用，默认为2表示5×5）
};

/**
 * @brief 单轮消除步骤信息
 */
struct EliminationStep {
    std::vector<std::pair<int, int>> positions; ///< 本轮被消除的所有格子
    std::vector<BombEffect> bombEffects;        ///< 本轮触发的炸弹特效列表
};

/**
 * @brief 单个下落移动信息
 */
struct FallMove {
    int fromRow = -1;
    int fromCol = -1;
    int toRow   = -1;
    int toCol   = -1;
};

/**
 * @brief 单个新生成水果信息
 */
struct NewFruit {
    int row = -1;
    int col = -1;
    FruitType type = FruitType::EMPTY;
    SpecialType special = SpecialType::NONE;
};

/**
 * @brief 单轮下落步骤信息
 */
struct FallStep {
    std::vector<FallMove> moves;       ///< 本轮所有下落或移动
    std::vector<NewFruit> newFruits;   ///< 本轮新生成的水果（包含类型信息）
};

/**
 * @brief 一轮消除+下落的配对事件
 */
struct GameRound {
    EliminationStep elimination;   ///< 本轮消除的元素
    FallStep fall;                 ///< 本轮下落+新生成
};

/**
 * @brief 一次完整主循环的动作记录
 *
 * 说明：
 * - 由 GameEngine 在 swapFruits / processGameCycle 内填充
 * - GameView 用于驱动动画
 * - 成就系统可用来统计连消、一次性消除数量等
 *
 * 使用方式：
 * 1. swap.success == false: 只播放交换回弹动画
 * 2. swap.success == true: 播放交换动画 → 逐轮播放 rounds[i].elimination → rounds[i].fall
 * 3. shuffled == true: 死局重排，播放重排动画
 */
struct GameAnimationSequence {
    SwapStep swap;                   ///< 本次玩家交换信息
    std::vector<GameRound> rounds;   ///< 多轮消除+下落的配对事件
    int totalScoreDelta = 0;         ///< 本次操作总得分增量
    bool shuffled = false;           ///< 是否发生死局重排
    std::vector<std::vector<Fruit>> newMapAfterShuffle;  ///< 重排后的新地图（用于动画）
};

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
     * @brief 获取最近一次完整主循环的动作记录（只读）
     */
    const GameAnimationSequence& getLastAnimation() const { return lastAnimation_; }

    
    /**
     * @brief 验证交换是否合法（是否相邻）
     */
    bool isValidSwap(int row1, int col1, int row2, int col2) const;
    
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
    // 基础子系统
    FruitGenerator fruitGenerator_;              ///< 水果生成器
    MatchDetector matchDetector_;                ///< 匹配检测器
    FallProcessor fallProcessor_;                ///< 下落处理器
    ScoreCalculator scoreCalculator_;            ///< 计分系统
    SpecialFruitGenerator specialGenerator_;     ///< 特殊元素生成器
    SpecialEffectProcessor specialProcessor_;    ///< 特殊效果处理器
    
    // 解耦后的功能模块
    SwapHandler swapHandler_;                    ///< 交换处理器
    AnimationRecorder animRecorder_;             ///< 动画记录器
    GameCycleProcessor cycleProcessor_;          ///< 循环处理器
    
    // 游戏数据
    std::vector<std::vector<Fruit>> map_;        ///< 游戏地图 (8×8)
    GameState state_;                            ///< 当前游戏状态
    int currentScore_;                           ///< 当前分数
    int totalMatches_;                           ///< 总匹配次数（统计用）
    
    // 记录最近一次玩家操作产生的完整动画序列
    GameAnimationSequence lastAnimation_;
};

#endif // GAMEENGINE_H
