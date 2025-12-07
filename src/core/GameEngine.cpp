#include "GameEngine.h"
#include <algorithm>

GameEngine::GameEngine() 
    : state_(GameState::IDLE)
    , currentScore_(0)
    , totalMatches_(0)
{
    // 构造函数 - 初始化成员变量
    lastAnimation_ = GameAnimationSequence{}; // 清零动画记录
}

GameEngine::~GameEngine() {
    // 析构函数 - 无需清理
}

/**
 * @brief 初始化游戏
 */
void GameEngine::initializeGame() {
    // 1. 初始化地图（确保无三连且有可移动）
    fruitGenerator_.initializeMap(map_);
    
    // 2. 确保地图可玩
    fruitGenerator_.ensurePlayable(map_, matchDetector_);
    
    // 3. 重置游戏状态
    state_ = GameState::IDLE;
    currentScore_ = 0;
    totalMatches_ = 0;
    scoreCalculator_.resetCombo();
    
    // 4. 清空最近动画记录
    lastAnimation_ = GameAnimationSequence{};
}

/**
 * @brief 尝试交换两个水果
 */
bool GameEngine::swapFruits(int row1, int col1, int row2, int col2) {
    // 1. 验证交换是否合法（相邻）
    if (!isValidSwap(row1, col1, row2, col2)) {
        // 非法交换，清空本次动画记录，并记录失败交换信息
        lastAnimation_ = GameAnimationSequence{};
        lastAnimation_.swap.row1 = row1;
        lastAnimation_.swap.col1 = col1;
        lastAnimation_.swap.row2 = row2;
        lastAnimation_.swap.col2 = col2;
        lastAnimation_.swap.success = false;
        return false;
    }
    
    // 2. 检查是否有 CANDY（Rainbow）元素参与交换
    bool isCandy1 = (map_[row1][col1].type == FruitType::CANDY);
    bool isCandy2 = (map_[row2][col2].type == FruitType::CANDY);
    
    if (isCandy1 || isCandy2) {
        // CANDY 元素特殊交换处理
        return handleCandySwap(row1, col1, row2, col2, isCandy1, isCandy2);
    }
    
    // 3. 检查是否有其他特殊元素参与交换
    bool hasSpecial1 = map_[row1][col1].special != SpecialType::NONE;
    bool hasSpecial2 = map_[row2][col2].special != SpecialType::NONE;
    
    // 4. 如果两个都是特殊元素，触发组合效果
    if (hasSpecial1 && hasSpecial2) {
        // 清空动画记录
        lastAnimation_ = GameAnimationSequence{};
        lastAnimation_.swap.row1 = row1;
        lastAnimation_.swap.col1 = col1;
        lastAnimation_.swap.row2 = row2;
        lastAnimation_.swap.col2 = col2;
        lastAnimation_.swap.success = true;
        
        std::set<std::pair<int, int>> affectedPositions;
        specialProcessor_.triggerCombinationEffect(map_, row1, col1, row2, col2, affectedPositions);
        
        // 创建本轮 round 记录爆炸消除
        GameRound bombRound;
        
        // 记录炸弹特效信息
        auto recordBombEffect = [&](int r, int c, SpecialType special) {
            BombEffect effect;
            effect.row = r;
            effect.col = c;
            switch (special) {
                case SpecialType::LINE_H:
                    effect.type = BombEffectType::LINE_H;
                    break;
                case SpecialType::LINE_V:
                    effect.type = BombEffectType::LINE_V;
                    break;
                case SpecialType::DIAMOND:
                    effect.type = BombEffectType::DIAMOND;
                    effect.range = 2;
                    break;
                case SpecialType::RAINBOW:
                    effect.type = BombEffectType::RAINBOW;
                    break;
                default:
                    break;
            }
            if (effect.type != BombEffectType::NONE) {
                bombRound.elimination.bombEffects.push_back(effect);
            }
        };
        recordBombEffect(row1, col1, map_[row1][col1].special);
        recordBombEffect(row2, col2, map_[row2][col2].special);
        
        // 消除受影响的位置，并记录到 elimination（但跳过CANDY）
        for (const auto& pos : affectedPositions) {
            // CANDY类型不能被炸弹波及消除
            if (map_[pos.first][pos.second].type == FruitType::CANDY) {
                continue;  // 保留CANDY
            }
            bombRound.elimination.positions.push_back(pos);
            map_[pos.first][pos.second].type = FruitType::EMPTY;
            map_[pos.first][pos.second].special = SpecialType::NONE;
        }
        
        // 处理下落和填充
        state_ = GameState::FALLING;
        recordFallAndRefill(bombRound.fall);
        
        // 保存爆炸轮次
        lastAnimation_.rounds.push_back(bombRound);
        
        // 处理后续连锁（可能新掉落的水果形成三消）
        processGameCycle();
        return true;
    }
    
    // 5. 执行交换
    std::swap(map_[row1][col1], map_[row2][col2]);
    // 更新坐标
    std::swap(map_[row1][col1].row, map_[row2][col2].row);
    std::swap(map_[row1][col1].col, map_[row2][col2].col);
    
    // 6. 检测交换后是否有匹配
    // 直接检测交换位置的匹配
    auto matches1 = matchDetector_.detectMatchesAt(map_, row1, col1);
    auto matches2 = matchDetector_.detectMatchesAt(map_, row2, col2);
    bool hasMatch = !matches1.empty() || !matches2.empty();
    
    if (!hasMatch) {
        // 如果没有匹配，撤销交换
        std::swap(map_[row1][col1], map_[row2][col2]);
        std::swap(map_[row1][col1].row, map_[row2][col2].row);
        std::swap(map_[row1][col1].col, map_[row2][col2].col);
        
        // 记录本次交换失败（用于交换失败动画），清空其他动画步骤
        lastAnimation_ = GameAnimationSequence{};
        lastAnimation_.swap.row1 = row1;
        lastAnimation_.swap.col1 = col1;
        lastAnimation_.swap.row2 = row2;
        lastAnimation_.swap.col2 = col2;
        lastAnimation_.swap.success = false;
        return false;
    }
    
    // 7. 有匹配，记录本次交换成功（清空其他动画步骤，由 processGameCycle 填充）
    lastAnimation_ = GameAnimationSequence{};
    lastAnimation_.swap.row1 = row1;
    lastAnimation_.swap.col1 = col1;
    lastAnimation_.swap.row2 = row2;
    lastAnimation_.swap.col2 = col2;
    lastAnimation_.swap.success = true;
    
    // 8. 有匹配，处理游戏循环
    state_ = GameState::SWAPPING;
    processGameCycle();
    
    return true;
}

/**
 * @brief 处理 CANDY（Rainbow）元素的特殊交换逻辑
 * 
 * 交互规则：
 * - CANDY + 普通元素：消除场上所有该普通元素类型
 * - CANDY + 炸弹元素：把场上所有对应普通类型元素转化为随机类型炸弹并按顺序引爆
 * - CANDY + CANDY：清除场上所有元素
 */
bool GameEngine::handleCandySwap(int row1, int col1, int row2, int col2,
                                  bool isCandy1, bool isCandy2) {
    // 清空动画记录
    lastAnimation_ = GameAnimationSequence{};
    lastAnimation_.swap.row1 = row1;
    lastAnimation_.swap.col1 = col1;
    lastAnimation_.swap.row2 = row2;
    lastAnimation_.swap.col2 = col2;
    lastAnimation_.swap.success = true;
    
    // 创建本轮 round 记录消除
    GameRound candyRound;
    
    // 记录 RAINBOW 特效
    auto recordRainbowEffect = [&](int r, int c) {
        BombEffect effect;
        effect.type = BombEffectType::RAINBOW;
        effect.row = r;
        effect.col = c;
        candyRound.elimination.bombEffects.push_back(effect);
    };
    
    if (isCandy1 && isCandy2) {
        // ================== CANDY + CANDY: 清除场上所有元素 ==================
        recordRainbowEffect(row1, col1);
        recordRainbowEffect(row2, col2);
        
        // 消除所有非空元素
        for (int r = 0; r < MAP_SIZE; ++r) {
            for (int c = 0; c < MAP_SIZE; ++c) {
                if (map_[r][c].type != FruitType::EMPTY) {
                    candyRound.elimination.positions.push_back({r, c});
                    map_[r][c].type = FruitType::EMPTY;
                    map_[r][c].special = SpecialType::NONE;
                }
            }
        }
    } else {
        // 确定 CANDY 和非 CANDY 的位置
        int candyRow, candyCol, otherRow, otherCol;
        if (isCandy1) {
            candyRow = row1; candyCol = col1;
            otherRow = row2; otherCol = col2;
        } else {
            candyRow = row2; candyCol = col2;
            otherRow = row1; otherCol = col1;
        }
        
        recordRainbowEffect(candyRow, candyCol);
        
        // 获取另一个元素的特殊类型和水果类型
        SpecialType otherSpecial = map_[otherRow][otherCol].special;
        FruitType targetType = map_[otherRow][otherCol].type;
        
        if (otherSpecial != SpecialType::NONE && otherSpecial != SpecialType::RAINBOW) {
            // ================== CANDY + 炸弹: 把场上所有对应类型转化为随机炸弹并引爆 ==================
            
            // 收集所有目标类型的位置
            std::vector<std::pair<int, int>> targets;
            for (int r = 0; r < MAP_SIZE; ++r) {
                for (int c = 0; c < MAP_SIZE; ++c) {
                    if (map_[r][c].type == targetType) {
                        targets.push_back({r, c});
                    }
                }
            }
            
            // 先消除 CANDY 和原炸弹
            candyRound.elimination.positions.push_back({candyRow, candyCol});
            candyRound.elimination.positions.push_back({otherRow, otherCol});
            map_[candyRow][candyCol].type = FruitType::EMPTY;
            map_[candyRow][candyCol].special = SpecialType::NONE;
            map_[otherRow][otherCol].type = FruitType::EMPTY;
            map_[otherRow][otherCol].special = SpecialType::NONE;
            
            // 随机炸弹类型列表（不包括 RAINBOW）
            SpecialType bombTypes[] = {SpecialType::LINE_H, SpecialType::LINE_V, SpecialType::DIAMOND};
            
            // 将每个目标转化为随机炸弹并引爆
            for (const auto& pos : targets) {
                int r = pos.first;
                int c = pos.second;
                if (map_[r][c].type == FruitType::EMPTY) continue;  // 已被消除
                
                // 随机选择炸弹类型
                SpecialType randBomb = bombTypes[rand() % 3];
                map_[r][c].special = randBomb;
                
                // 记录炸弹特效
                BombEffect effect;
                effect.row = r;
                effect.col = c;
                switch (randBomb) {
                    case SpecialType::LINE_H:
                        effect.type = BombEffectType::LINE_H;
                        break;
                    case SpecialType::LINE_V:
                        effect.type = BombEffectType::LINE_V;
                        break;
                    case SpecialType::DIAMOND:
                        effect.type = BombEffectType::DIAMOND;
                        effect.range = 2;
                        break;
                    default:
                        break;
                }
                candyRound.elimination.bombEffects.push_back(effect);
                
                // 触发炸弹效果
                std::set<std::pair<int, int>> affected;
                specialProcessor_.triggerSpecialEffect(map_, r, c, affected);
                
                // 标记受影响位置为消除（但跳过CANDY）
                for (const auto& apos : affected) {
                    // CANDY类型不能被炸弹波及消除
                    if (map_[apos.first][apos.second].type == FruitType::CANDY) {
                        continue;  // 保留CANDY
                    }
                    if (map_[apos.first][apos.second].type != FruitType::EMPTY) {
                        candyRound.elimination.positions.push_back(apos);
                        map_[apos.first][apos.second].type = FruitType::EMPTY;
                        map_[apos.first][apos.second].special = SpecialType::NONE;
                    }
                }
            }
        } else {
            // ================== CANDY + 普通元素: 消除场上所有该类型 ==================
            
            // 消除 CANDY 本身
            candyRound.elimination.positions.push_back({candyRow, candyCol});
            map_[candyRow][candyCol].type = FruitType::EMPTY;
            map_[candyRow][candyCol].special = SpecialType::NONE;
            
            // 消除所有目标类型的水果
            for (int r = 0; r < MAP_SIZE; ++r) {
                for (int c = 0; c < MAP_SIZE; ++c) {
                    if (map_[r][c].type == targetType) {
                        candyRound.elimination.positions.push_back({r, c});
                        map_[r][c].type = FruitType::EMPTY;
                        map_[r][c].special = SpecialType::NONE;
                    }
                }
            }
        }
    }
    
    // 处理下落和填充
    state_ = GameState::FALLING;
    recordFallAndRefill(candyRound.fall);
    
    // 保存 CANDY 轮次
    lastAnimation_.rounds.push_back(candyRound);
    
    // 处理后续连锁（新掉落的水果可能形成三消）
    processGameCycle();
    
    return true;
}

/**
 * @brief 处理一轮游戏循环
 * 
 * 注意：调用前 swap 信息已由 swapFruits 填充，这里不清空 rounds，而是追加
 */
bool GameEngine::processGameCycle() {
    bool hadElimination = false;
    bool isFirstMatch = (lastAnimation_.rounds.empty());  // 只有第一轮才生成特殊元素
    
    // 不清空 rounds，而是在现有基础上追加
    // lastAnimation_.rounds.clear();  // 不清空，可能已有爆炸轮次
    lastAnimation_.totalScoreDelta = 0;
    
    // 循环处理：匹配 → 消除 → 下落 → 再匹配
    while (true) {
        // 1. 检测匹配
        state_ = GameState::MATCHING;
        auto matches = matchDetector_.detectMatches(map_);
        
        if (matches.empty()) {
            break;  // 没有匹配，结束循环
        }
        
        hadElimination = true;
        
        // 创建本轮 round
        GameRound round;
        
        // 2. 如果是第一次匹配，生成特殊元素（在消除前）
        std::set<std::pair<int, int>> specialPositions;  // 记录生成特殊元素的位置
        if (isFirstMatch) {
            processSpecialGeneration(matches, specialPositions);
            isFirstMatch = false;
        }
        
        // 3. 计算得分
        int comboMultiplier = std::max(1, scoreCalculator_.getComboCount());
        int score = scoreCalculator_.calculateTotalScore(matches, comboMultiplier);
        currentScore_ += score;
        lastAnimation_.totalScoreDelta += score;
        
        // 4. 消除匹配的水果（但跳过刚生成的特殊元素），记录到 round.elimination
        recordAndEliminateMatches(matches, specialPositions, round.elimination);
        
        // 5. 统计
        totalMatches_ += matches.size();
        
        // 6. 处理下落和填充，记录到 round.fall
        state_ = GameState::FALLING;
        recordFallAndRefill(round.fall);
        
        // 7. 保存本轮
        lastAnimation_.rounds.push_back(round);
        
        // 8. 增加连击
        scoreCalculator_.incrementCombo();
    }
    
    // 如果有消除，重置连击；否则检查死局
    if (hadElimination) {
        scoreCalculator_.resetCombo();
    } else {
        checkAndHandleDeadlock();
    }
    
    state_ = GameState::IDLE;
    return hadElimination;
}

/**
 * @brief 处理特殊元素生成
 * @param matches 匹配结果列表
 * @param specialPositions 输出参数，记录生成特殊元素的位置
 */
void GameEngine::processSpecialGeneration(
    const std::vector<MatchResult>& matches,
    std::set<std::pair<int, int>>& specialPositions) {
    
    for (const auto& match : matches) {
        // 判断是否应该生成特殊元素
        SpecialType specialType = specialGenerator_.determineSpecialType(match);
        
        if (specialType != SpecialType::NONE) {
            // 尝试在地图上生成特殊元素
            auto pos = specialGenerator_.generateSpecialFruit(map_, match, specialType);
            
            if (pos.first == -2 && pos.second == -2) {
                // 目标位置已有炸弹，升级炸弹而不是触发爆炸
                // 计算原始生成位置
                std::pair<int, int> origPos = {-1, -1};
                if (!match.positions.empty()) {
                    size_t midIndex = match.positions.size() / 2;
                    origPos = match.positions[midIndex];
                }
                
                if (origPos.first >= 0 && origPos.first < MAP_SIZE &&
                    origPos.second >= 0 && origPos.second < MAP_SIZE) {
                    
                    SpecialType existingSpecial = map_[origPos.first][origPos.second].special;
                    
                    // 升级策略：取两个炸弹类型中更高级的
                    // 优先级： RAINBOW > DIAMOND > LINE_H/LINE_V
                    SpecialType upgradedType = existingSpecial;
                    
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
                    
                    if (newPriority > existingPriority) {
                        upgradedType = specialType;
                    } else if (existingPriority == newPriority && existingPriority < 3) {
                        // 同级别且都是直线炸弹，升级为菱形炸弹
                        upgradedType = SpecialType::DIAMOND;
                    } else if (existingPriority == 3 && newPriority == 3) {
                        // 两个菱形炸弹，升级为RAINBOW
                        upgradedType = SpecialType::RAINBOW;
                    }
                    
                    // 应用升级
                    map_[origPos.first][origPos.second].special = upgradedType;
                    if (upgradedType == SpecialType::RAINBOW) {
                        map_[origPos.first][origPos.second].type = FruitType::CANDY;
                    }
                    
                    // 保留这个位置，不要被消除
                    specialPositions.insert(origPos);
                    map_[origPos.first][origPos.second].isMatched = false;
                }
            } else if (pos.first >= 0) {
                // 正常生成，记录特殊元素位置
                specialPositions.insert(pos);
            }
        }
    }
}

/**
 * @brief 消除匹配的水果，并记录到 elimStep
 * @param specialPositions 需要保留的特殊元素位置（刚生成的）
 * @param elimStep 输出参数，记录本轮消除的位置和炸弹特效
 */
void GameEngine::recordAndEliminateMatches(
    const std::vector<MatchResult>& matches,
    const std::set<std::pair<int, int>>& specialPositions,
    EliminationStep& elimStep) {
    
    state_ = GameState::ELIMINATING;
    
    // 第一步：标记所有需要消除的位置（普通三消）
    for (const auto& match : matches) {
        for (const auto& pos : match.positions) {
            // 如果该位置是刚生成的特殊元素，跳过不消除
            if (specialPositions.find(pos) != specialPositions.end()) {
                continue;  // 保留刚生成的特殊元素
            }
            // CANDY类型不能被普通三消消除（只能通过交换消耗）
            if (map_[pos.first][pos.second].type == FruitType::CANDY) {
                continue;  // 保留CANDY
            }
            map_[pos.first][pos.second].isMatched = true;
            elimStep.positions.push_back(pos);
        }
    }
    
    // 第二步：处理特殊元素效果（被消除的特殊元素会触发效果）
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map_[row][col].isMatched && map_[row][col].special != SpecialType::NONE) {
                // 记录炸弹特效信息
                BombEffect effect;
                effect.row = row;
                effect.col = col;
                switch (map_[row][col].special) {
                    case SpecialType::LINE_H:
                        effect.type = BombEffectType::LINE_H;
                        break;
                    case SpecialType::LINE_V:
                        effect.type = BombEffectType::LINE_V;
                        break;
                    case SpecialType::DIAMOND:
                        effect.type = BombEffectType::DIAMOND;
                        effect.range = 2;  // 5×5 菱形
                        break;
                    case SpecialType::RAINBOW:
                        effect.type = BombEffectType::RAINBOW;
                        break;
                    default:
                        break;
                }
                if (effect.type != BombEffectType::NONE) {
                    elimStep.bombEffects.push_back(effect);
                }
                
                std::set<std::pair<int, int>> affectedPositions;
                specialProcessor_.triggerSpecialEffect(map_, row, col, affectedPositions);
                
                // 标记受影响的位置为消除
                for (const auto& pos : affectedPositions) {
                    // 如果是刚生成的特殊元素，也不消除
                    if (specialPositions.find(pos) == specialPositions.end()) {
                        // CANDY类型不能被炸弹波及消除（只能通过交换消耗）
                        if (map_[pos.first][pos.second].type == FruitType::CANDY) {
                            continue;  // 保留CANDY
                        }
                        if (!map_[pos.first][pos.second].isMatched) {
                            map_[pos.first][pos.second].isMatched = true;
                            elimStep.positions.push_back(pos);
                        }
                    }
                }
            }
        }
    }
    
    // 第三步：执行消除（清空所有被标记的位置）
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map_[row][col].isMatched) {
                map_[row][col].type = FruitType::EMPTY;
                map_[row][col].special = SpecialType::NONE;
                map_[row][col].isMatched = false;
            }
        }
    }
}

// 保留原有接口以兼容其他调用
void GameEngine::eliminateMatches(
    const std::vector<MatchResult>& matches,
    const std::set<std::pair<int, int>>& specialPositions) {
    EliminationStep dummyStep;
    recordAndEliminateMatches(matches, specialPositions, dummyStep);
}

/**
 * @brief 处理下落和填充空位，并记录到 fallStep
 * @param fallStep 输出参数，记录本轮下落和新生成信息
 */
void GameEngine::recordFallAndRefill(FallStep& fallStep) {
    // 1. 处理下落
    bool hasFall = fallProcessor_.hasEmptySlots(map_);
    
    if (hasFall) {
        // 处理每一列的下落
        for (int col = 0; col < MAP_SIZE; col++) {
            // 记录该列下落前的位置和水果
            std::vector<std::tuple<int, int, Fruit>> before;
            for (int row = 0; row < MAP_SIZE; ++row) {
                if (map_[row][col].type != FruitType::EMPTY) {
                    before.emplace_back(row, col, map_[row][col]);
                }
            }
            
            fallProcessor_.processColumnFall(map_, col);
            
            // 记录该列下落后的位置，并生成移动记录
            std::vector<std::pair<int,int>> after;
            for (int row = 0; row < MAP_SIZE; ++row) {
                if (map_[row][col].type != FruitType::EMPTY) {
                    after.emplace_back(row, col);
                }
            }
            
            // before.size() 与 after.size() 理论上相同，对位生成 FallMove
            size_t count = std::min(before.size(), after.size());
            for (size_t i = 0; i < count; ++i) {
                int fromRow = std::get<0>(before[i]);
                int fromCol = std::get<1>(before[i]);
                int toRow = after[i].first;
                int toCol = after[i].second;
                
                if (fromRow != toRow || fromCol != toCol) {
                    FallMove move;
                    move.fromRow = fromRow;
                    move.fromCol = fromCol;
                    move.toRow   = toRow;
                    move.toCol   = toCol;
                    fallStep.moves.push_back(move);
                }
            }
        }
    }
    
    // 2. 填充空位，记录新生成的水果（包含类型信息）
    auto newSlots = fallProcessor_.fillEmptySlots(map_, fruitGenerator_);
    for (const auto& pos : newSlots) {
        NewFruit nf;
        nf.row = pos.first;
        nf.col = pos.second;
        nf.type = map_[pos.first][pos.second].type;
        nf.special = map_[pos.first][pos.second].special;
        fallStep.newFruits.push_back(nf);
    }
}

// 保留原有接口以兼容其他调用
bool GameEngine::processFallAndRefill() {
    FallStep dummyStep;
    recordFallAndRefill(dummyStep);
    return !dummyStep.moves.empty() || !dummyStep.newFruits.empty();
}

/**
 * @brief 检查并处理死局
 * 
 * 如果没有可移动，重排地图并记录动画消息
 */
void GameEngine::checkAndHandleDeadlock() {
    if (!matchDetector_.hasPossibleMoves(map_)) {
        // 没有可移动，重排地图
        fruitGenerator_.shuffleMap(map_, matchDetector_);
        
        // 记录重排动画消息
        lastAnimation_.shuffled = true;
        lastAnimation_.newMapAfterShuffle = map_;  // 保存重排后的新地图
    }
}

/**
 * @brief 验证交换是否合法
 */
bool GameEngine::isValidSwap(int row1, int col1, int row2, int col2) const {
    // 1. 检查位置是否合法
    if (!isValidPosition(row1, col1) || !isValidPosition(row2, col2)) {
        return false;
    }
    
    // 2. 检查是否相邻
    return isAdjacent(row1, col1, row2, col2);
}

/**
 * @brief 检查地图是否有可移动
 */
bool GameEngine::hasValidMoves() const {
    // 使用非 const 的 MatchDetector 实例，或者修改 hasPossibleMoves 为 const 方法
    // 这里我们直接使用 const_cast 来解决
    return const_cast<MatchDetector&>(matchDetector_).hasPossibleMoves(map_);
}

/**
 * @brief 重置游戏
 */
void GameEngine::resetGame() {
    initializeGame();
}
