#include "GameCycleProcessor.h"
#include "GameEngine.h"  // 包含完整的结构体定义
#include <algorithm>

GameCycleProcessor::GameCycleProcessor(MatchDetector& matchDetector,
                                       SpecialFruitGenerator& specialGenerator,
                                       SpecialEffectProcessor& specialProcessor,
                                       AnimationRecorder& animRecorder,
                                       FruitGenerator& fruitGenerator,
                                       ScoreCalculator& scoreCalculator)
    : matchDetector_(matchDetector)
    , specialGenerator_(specialGenerator)
    , specialProcessor_(specialProcessor)
    , animRecorder_(animRecorder)
    , fruitGenerator_(fruitGenerator)
    , scoreCalculator_(scoreCalculator)
{
}

GameCycleProcessor::~GameCycleProcessor() {
}

/**
 * @brief 处理一轮完整的游戏循环
 */
bool GameCycleProcessor::processMatchCycle(std::vector<std::vector<Fruit>>& map,
                                           std::vector<GameRound>& outRounds,
                                           int& outTotalScore) {
    outRounds.clear();
    outTotalScore = 0;
    bool hadElimination = false;
    bool isFirstMatch = true;  // 只有第一轮才生成特殊元素
    
    // 循环处理：匹配 → 消除 → 下落 → 再匹配
    while (true) {
        // 1. 检测匹配
        auto matches = matchDetector_.detectMatches(map);
        
        if (matches.empty()) {
            break;  // 没有匹配，结束循环
        }
        
        hadElimination = true;
        
        // 创建本轮 round
        GameRound round;
        
        // 2. 如果是第一次匹配，生成特殊元素
        std::set<std::pair<int, int>> specialPositions;
        if (isFirstMatch) {
            processSpecialGeneration(map, matches, specialPositions);
            isFirstMatch = false;
        }
        
        // 3. 计算得分
        int comboMultiplier = std::max(1, scoreCalculator_.getComboCount());
        int score = scoreCalculator_.calculateTotalScore(matches, comboMultiplier);
        outTotalScore += score;
        
        // 📌 保存本轮得分和连击数（用于分数浮动显示）
        round.scoreDelta = score;
        round.comboCount = scoreCalculator_.getComboCount();
        
        // 4. 标记匹配的水果为待消除（跳过刚生成的特殊元素和CANDY）
        markMatchesForElimination(map, matches, specialPositions);
        
        // 📌 保存每个匹配组的信息（用于多消成就检测）
        round.elimination.matchGroups.clear();
        for (const auto& match : matches) {
            MatchGroup group;
            group.count = match.matchCount;
            group.type = match.fruitType;
            round.elimination.matchGroups.push_back(group);
        }
        
        // 5. 触发特殊元素效果
        triggerSpecialEffects(map, specialPositions);
        
        // 6. 记录并执行消除
        animRecorder_.recordElimination(map, specialPositions, round.elimination);
        
        // 7. 处理下落和填充
        animRecorder_.recordFallAndRefill(map, fruitGenerator_, round.fall);
        
        // 8. 保存本轮
        outRounds.push_back(round);
        
        // 9. 增加连击
        scoreCalculator_.incrementCombo();
    }
    
    // 如果有消除，重置连击
    if (hadElimination) {
        scoreCalculator_.resetCombo();
    }
    
    return hadElimination;
}

/**
 * @brief 处理特殊元素生成
 */
void GameCycleProcessor::processSpecialGeneration(std::vector<std::vector<Fruit>>& map,
                                                   const std::vector<MatchResult>& matches,
                                                   std::set<std::pair<int, int>>& specialPositions) {
    for (const auto& match : matches) {
        SpecialType specialType = specialGenerator_.determineSpecialType(match);
        
        if (specialType != SpecialType::NONE) {
            auto pos = specialGenerator_.generateSpecialFruit(map, match, specialType);
            
            if (pos.first == -2 && pos.second == -2) {
                // 位置冲突，升级炸弹
                std::pair<int, int> origPos = {-1, -1};
                if (!match.positions.empty()) {
                    size_t midIndex = match.positions.size() / 2;
                    origPos = match.positions[midIndex];
                }
                
                if (origPos.first >= 0 && origPos.first < MAP_SIZE &&
                    origPos.second >= 0 && origPos.second < MAP_SIZE) {
                    
                    SpecialType existingSpecial = map[origPos.first][origPos.second].special;
                    
                    // 升级策略
                    auto getPriority = [](SpecialType t) {
                        switch (t) {
                            case SpecialType::RAINBOW: return 4;
                            case SpecialType::DIAMOND: return 3;
                            case SpecialType::LINE_H:
                            case SpecialType::LINE_V: return 2;
                            default: return 1;
                        }
                    };
                    
                    int existingPriority = getPriority(existingSpecial);
                    int newPriority = getPriority(specialType);
                    
                    SpecialType upgradedType = existingSpecial;
                    if (newPriority > existingPriority) {
                        upgradedType = specialType;
                    } else if (existingPriority == newPriority && existingPriority < 3) {
                        upgradedType = SpecialType::DIAMOND;
                    } else if (existingPriority == 3 && newPriority == 3) {
                        upgradedType = SpecialType::RAINBOW;
                    }
                    
                    // 应用升级
                    map[origPos.first][origPos.second].special = upgradedType;
                    if (upgradedType == SpecialType::RAINBOW) {
                        map[origPos.first][origPos.second].type = FruitType::CANDY;
                    }
                    
                    // 保留这个位置
                    specialPositions.insert(origPos);
                    map[origPos.first][origPos.second].isMatched = false;
                }
            } else if (pos.first >= 0) {
                // 正常生成
                specialPositions.insert(pos);
            }
        }
    }
}

/**
 * @brief 标记匹配的水果为待消除
 */
void GameCycleProcessor::markMatchesForElimination(std::vector<std::vector<Fruit>>& map,
                                                    const std::vector<MatchResult>& matches,
                                                    const std::set<std::pair<int, int>>& specialPositions) {
    for (const auto& match : matches) {
        for (const auto& pos : match.positions) {
            // 跳过刚生成的特殊元素
            if (specialPositions.find(pos) != specialPositions.end()) {
                continue;
            }
            // 跳过 CANDY（只能通过交换消耗）
            if (map[pos.first][pos.second].type == FruitType::CANDY) {
                continue;
            }
            map[pos.first][pos.second].isMatched = true;
        }
    }
}

/**
 * @brief 触发特殊元素效果
 */
void GameCycleProcessor::triggerSpecialEffects(std::vector<std::vector<Fruit>>& map,
                                                const std::set<std::pair<int, int>>& specialPositions) {
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map[row][col].isMatched && map[row][col].special != SpecialType::NONE) {
                std::set<std::pair<int, int>> affectedPositions;
                specialProcessor_.triggerSpecialEffect(map, row, col, affectedPositions);
                
                // 标记受影响的位置为消除
                for (const auto& pos : affectedPositions) {
                    // 跳过刚生成的特殊元素
                    if (specialPositions.find(pos) != specialPositions.end()) {
                        continue;
                    }
                    // 跳过 CANDY
                    if (map[pos.first][pos.second].type == FruitType::CANDY) {
                        continue;
                    }
                    if (!map[pos.first][pos.second].isMatched) {
                        map[pos.first][pos.second].isMatched = true;
                    }
                }
            }
        }
    }
}

/**
 * @brief 检测并处理死局
 */
void GameCycleProcessor::handleDeadlock(std::vector<std::vector<Fruit>>& map,
                                        bool& outShuffled,
                                        std::vector<std::vector<Fruit>>& outNewMap) {
    outShuffled = false;
    
    if (!matchDetector_.hasPossibleMoves(map)) {
        // 没有可移动，重排地图
        fruitGenerator_.shuffleMap(map, matchDetector_);
        outShuffled = true;
        outNewMap = map;
    }
}

/**
 * @brief 处理道具触发的单次消除（不循环，只消除一轮）
 */
bool GameCycleProcessor::processPropElimination(std::vector<std::vector<Fruit>>& map,
                                                 const std::set<std::pair<int, int>>& affectedPositions,
                                                 GameRound& outRound,
                                                 int& outScore) {
    if (affectedPositions.empty()) {
        return false;
    }
    
    // 1. 标记受影响的位置为待消除（跳过CANDY）
    for (const auto& pos : affectedPositions) {
        int row = pos.first;
        int col = pos.second;
        if (row >= 0 && row < MAP_SIZE && col >= 0 && col < MAP_SIZE) {
            if (map[row][col].type != FruitType::CANDY && map[row][col].type != FruitType::EMPTY) {
                map[row][col].isMatched = true;
            }
        }
    }
    
    // 2. 触发特殊元素效果（如果道具击中了炸弹）
    std::set<std::pair<int, int>> emptySpecialPositions;  // 道具模式不生成新的特殊元素
    triggerSpecialEffects(map, emptySpecialPositions);
    
    // 3. 计算得分（道具模式不用连击）
    std::vector<MatchResult> virtualMatches;
    for (const auto& pos : affectedPositions) {
        if (map[pos.first][pos.second].isMatched) {
            MatchResult match;
            match.positions.push_back(pos);
            match.fruitType = map[pos.first][pos.second].type;
            virtualMatches.push_back(match);
        }
    }
    outScore = scoreCalculator_.calculateTotalScore(virtualMatches, 1);  // 无连击
    
    // 4. 记录并执行消除
    animRecorder_.recordElimination(map, emptySpecialPositions, outRound.elimination);
    
    // 5. 处理下落和填充
    animRecorder_.recordFallAndRefill(map, fruitGenerator_, outRound.fall);
    
    return true;
}
