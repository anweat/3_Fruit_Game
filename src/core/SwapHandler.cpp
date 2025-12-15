#include "SwapHandler.h"
#include "GameEngine.h"  // 包含完整的结构体定义
#include <algorithm>

SwapHandler::SwapHandler(MatchDetector& matchDetector,
                         SpecialEffectProcessor& specialProcessor)
    : matchDetector_(matchDetector)
    , specialProcessor_(specialProcessor)
{
}

SwapHandler::~SwapHandler() {
}

/**
 * @brief 执行交换操作
 */
bool SwapHandler::executeSwap(std::vector<std::vector<Fruit>>& map,
                               int row1, int col1, int row2, int col2,
                               SwapStep& outSwapStep,
                               std::vector<GameRound>& outRounds) {
    // 清空输出
    outRounds.clear();
    
    // 记录交换位置
    outSwapStep.row1 = row1;
    outSwapStep.col1 = col1;
    outSwapStep.row2 = row2;
    outSwapStep.col2 = col2;
    outSwapStep.success = false;
    
    // 1. 验证交换合法�?
    if (!isValidSwap(map, row1, col1, row2, col2)) {
        return false;
    }
    
    // 2. 检查是否有 CANDY 参与
    bool isCandy1 = (map[row1][col1].type == FruitType::CANDY);
    bool isCandy2 = (map[row2][col2].type == FruitType::CANDY);
    
    if (isCandy1 || isCandy2) {
        // CANDY 特殊交换
        handleCandySwap(map, row1, col1, row2, col2, isCandy1, isCandy2, outRounds);
        outSwapStep.success = true;
        return true;
    }
    
    // 3. 检查是否两个都是特殊元�?
    bool hasSpecial1 = map[row1][col1].special != SpecialType::NONE;
    bool hasSpecial2 = map[row2][col2].special != SpecialType::NONE;
    
    if (hasSpecial1 && hasSpecial2) {
        // 炸弹组合效果
        handleSpecialCombo(map, row1, col1, row2, col2, outRounds);
        outSwapStep.success = true;
        return true;
    }
    
    // 4. 普通交�?
    bool hasMatch = handleNormalSwap(map, row1, col1, row2, col2);
    outSwapStep.success = hasMatch;
    return hasMatch;
}

/**
 * @brief 验证交换是否合法
 */
bool SwapHandler::isValidSwap(const std::vector<std::vector<Fruit>>& map, int row1, int col1, int row2, int col2) const {
    // 检查位置合法�?
    if (row1 < 0 || row1 >= static_cast<int>(map.size()) || col1 < 0 || col1 >= static_cast<int>(map.size()) || row2 < 0 || row2 >= static_cast<int>(map.size()) || col2 < 0 || col2 >= static_cast<int>(map.size())) {
        return false;
    }
    
    // 检查是否相�?
    int dr = abs(row1 - row2);
    int dc = abs(col1 - col2);
    return (dr == 1 && dc == 0) || (dr == 0 && dc == 1);
}

/**
 * @brief 处理普通交�?
 */
bool SwapHandler::handleNormalSwap(std::vector<std::vector<Fruit>>& map,
                                    int row1, int col1, int row2, int col2) {
    // 执行交换
    std::swap(map[row1][col1], map[row2][col2]);
    std::swap(map[row1][col1].row, map[row2][col2].row);
    std::swap(map[row1][col1].col, map[row2][col2].col);
    
    // 检测是否有匹配
    auto matches1 = matchDetector_.detectMatchesAt(map, row1, col1);
    auto matches2 = matchDetector_.detectMatchesAt(map, row2, col2);
    bool hasMatch = !matches1.empty() || !matches2.empty();
    
    if (!hasMatch) {
        // 没有匹配，撤销交换
        std::swap(map[row1][col1], map[row2][col2]);
        std::swap(map[row1][col1].row, map[row2][col2].row);
        std::swap(map[row1][col1].col, map[row2][col2].col);
    }
    
    return hasMatch;
}

/**
 * @brief 处理 CANDY 特殊交换
 */
void SwapHandler::handleCandySwap(std::vector<std::vector<Fruit>>& map,
                                   int row1, int col1, int row2, int col2,
                                   bool isCandy1, bool isCandy2,
                                   std::vector<GameRound>& outRounds) {
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
        // ========== CANDY + CANDY: 清除所有元�?==========
        recordRainbowEffect(row1, col1);
        recordRainbowEffect(row2, col2);
        
        for (int r = 0; r < static_cast<int>(map.size()); ++r) {
            for (int c = 0; c < static_cast<int>(map.size()); ++c) {
                if (map[r][c].type != FruitType::EMPTY) {
                    candyRound.elimination.positions.push_back({r, c});
                    map[r][c].type = FruitType::EMPTY;
                    map[r][c].special = SpecialType::NONE;
                }
            }
        }
    } else {
        // 确定 CANDY 和非 CANDY 的位�?
        int candyRow, candyCol, otherRow, otherCol;
        if (isCandy1) {
            candyRow = row1; candyCol = col1;
            otherRow = row2; otherCol = col2;
        } else {
            candyRow = row2; candyCol = col2;
            otherRow = row1; otherCol = col1;
        }
        
        recordRainbowEffect(candyRow, candyCol);
        
        SpecialType otherSpecial = map[otherRow][otherCol].special;
        FruitType targetType = map[otherRow][otherCol].type;
        
        if (otherSpecial != SpecialType::NONE && otherSpecial != SpecialType::RAINBOW) {
            // ========== CANDY + 炸弹: 转化所有该类型为随机炸弹并引爆 ==========
            std::vector<std::pair<int, int>> targets;
            for (int r = 0; r < static_cast<int>(map.size()); ++r) {
                for (int c = 0; c < static_cast<int>(map.size()); ++c) {
                    if (map[r][c].type == targetType) {
                        targets.push_back({r, c});
                    }
                }
            }
            
            // 消除 CANDY 和原炸弹
            candyRound.elimination.positions.push_back({candyRow, candyCol});
            candyRound.elimination.positions.push_back({otherRow, otherCol});
            map[candyRow][candyCol].type = FruitType::EMPTY;
            map[candyRow][candyCol].special = SpecialType::NONE;
            map[otherRow][otherCol].type = FruitType::EMPTY;
            map[otherRow][otherCol].special = SpecialType::NONE;
            
            // 随机炸弹类型
            SpecialType bombTypes[] = {SpecialType::LINE_H, SpecialType::LINE_V, SpecialType::DIAMOND};
            
            for (const auto& pos : targets) {
                int r = pos.first;
                int c = pos.second;
                if (map[r][c].type == FruitType::EMPTY) continue;
                
                // 随机选择炸弹类型
                SpecialType randBomb = bombTypes[rand() % 3];
                map[r][c].special = randBomb;
                
                // 记录炸弹特效
                recordBombEffect(r, c, randBomb, candyRound.elimination.bombEffects);
                
                // 触发炸弹效果
                std::set<std::pair<int, int>> affected;
                specialProcessor_.triggerSpecialEffect(map, r, c, affected);
                
                // 标记消除（跳�?CANDY�?
                for (const auto& apos : affected) {
                    if (map[apos.first][apos.second].type == FruitType::CANDY) {
                        continue;
                    }
                    if (map[apos.first][apos.second].type != FruitType::EMPTY) {
                        candyRound.elimination.positions.push_back(apos);
                        map[apos.first][apos.second].type = FruitType::EMPTY;
                        map[apos.first][apos.second].special = SpecialType::NONE;
                    }
                }
            }
        } else {
            // ========== CANDY + 普�? 消除所有该类型 ==========
            candyRound.elimination.positions.push_back({candyRow, candyCol});
            map[candyRow][candyCol].type = FruitType::EMPTY;
            map[candyRow][candyCol].special = SpecialType::NONE;
            
            for (int r = 0; r < static_cast<int>(map.size()); ++r) {
                for (int c = 0; c < static_cast<int>(map.size()); ++c) {
                    if (map[r][c].type == targetType) {
                        candyRound.elimination.positions.push_back({r, c});
                        map[r][c].type = FruitType::EMPTY;
                        map[r][c].special = SpecialType::NONE;
                    }
                }
            }
        }
    }
    
    outRounds.push_back(candyRound);
}

/**
 * @brief 处理炸弹组合交换
 */
void SwapHandler::handleSpecialCombo(std::vector<std::vector<Fruit>>& map,
                                      int row1, int col1, int row2, int col2,
                                      std::vector<GameRound>& outRounds) {
    GameRound bombRound;
    
    // 记录两个炸弹的特�?
    recordBombEffect(row1, col1, map[row1][col1].special, bombRound.elimination.bombEffects);
    recordBombEffect(row2, col2, map[row2][col2].special, bombRound.elimination.bombEffects);
    
    // 触发组合效果
    std::set<std::pair<int, int>> affectedPositions;
    specialProcessor_.triggerCombinationEffect(map, row1, col1, row2, col2, affectedPositions);
    
    // 消除受影响的位置（跳�?CANDY�?
    for (const auto& pos : affectedPositions) {
        if (map[pos.first][pos.second].type == FruitType::CANDY) {
            continue;
        }
        bombRound.elimination.positions.push_back(pos);
        map[pos.first][pos.second].type = FruitType::EMPTY;
        map[pos.first][pos.second].special = SpecialType::NONE;
    }
    
    outRounds.push_back(bombRound);
}

/**
 * @brief 记录炸弹特效
 */
void SwapHandler::recordBombEffect(int row, int col, SpecialType special,
                                    std::vector<BombEffect>& bombEffects) {
    BombEffect effect;
    effect.row = row;
    effect.col = col;
    
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
        bombEffects.push_back(effect);
    }
}
