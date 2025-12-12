#ifndef GAMECYCLEPROCESSOR_H
#define GAMECYCLEPROCESSOR_H

#include "FruitTypes.h"
#include "MatchDetector.h"
#include "SpecialFruitGenerator.h"
#include "SpecialEffectProcessor.h"
#include "AnimationRecorder.h"
#include "FruitGenerator.h"
#include "ScoreCalculator.h"
#include <vector>
#include <set>

// 前置声明结构体
struct GameRound;
struct MatchResult;

/**
 * @brief 游戏循环处理器 - 处理匹配→消除→下落的循环逻辑
 * 
 * 职责：
 * 1. 检测并处理三消匹配
 * 2. 生成特殊元素（炸弹）
 * 3. 标记并触发消除（包括炸弹效果）
 * 4. 编排下落和填充
 * 5. 循环处理直到无匹配
 * 6. 检测并处理死局
 */
class GameCycleProcessor {
public:
    GameCycleProcessor(MatchDetector& matchDetector,
                       SpecialFruitGenerator& specialGenerator,
                       SpecialEffectProcessor& specialProcessor,
                       AnimationRecorder& animRecorder,
                       FruitGenerator& fruitGenerator,
                       ScoreCalculator& scoreCalculator);
    ~GameCycleProcessor();
    
    /**
     * @brief 处理一轮完整的游戏循环
     * @param map 游戏地图（会被修改）
     * @param outRounds 输出所有轮次数据
     * @param outTotalScore 输出总得分增量
     * @return 是否有消除发生
     */
    bool processMatchCycle(std::vector<std::vector<Fruit>>& map,
                           std::vector<GameRound>& outRounds,
                           int& outTotalScore);
    
    /**
     * @brief 检测并处理死局
     * @param map 游戏地图（会被修改）
     * @param outShuffled 输出是否发生重排
     * @param outNewMap 输出重排后的地图
     */
    void handleDeadlock(std::vector<std::vector<Fruit>>& map,
                        bool& outShuffled,
                        std::vector<std::vector<Fruit>>& outNewMap);
    
private:
    /**
     * @brief 处理特殊元素生成
     */
    void processSpecialGeneration(std::vector<std::vector<Fruit>>& map,
                                   const std::vector<MatchResult>& matches,
                                   std::set<std::pair<int, int>>& specialPositions);
    
    /**
     * @brief 标记匹配的水果为待消除
     */
    void markMatchesForElimination(std::vector<std::vector<Fruit>>& map,
                                    const std::vector<MatchResult>& matches,
                                    const std::set<std::pair<int, int>>& specialPositions);
    
    /**
     * @brief 触发特殊元素效果并标记受影响位置
     */
    void triggerSpecialEffects(std::vector<std::vector<Fruit>>& map,
                                const std::set<std::pair<int, int>>& specialPositions);
    
    MatchDetector& matchDetector_;
    SpecialFruitGenerator& specialGenerator_;
    SpecialEffectProcessor& specialProcessor_;
    AnimationRecorder& animRecorder_;
    FruitGenerator& fruitGenerator_;
    ScoreCalculator& scoreCalculator_;
};

#endif // GAMECYCLEPROCESSOR_H
