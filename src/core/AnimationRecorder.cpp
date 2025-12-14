#include "AnimationRecorder.h"
#include "GameEngine.h"  // 包含完整的结构体定义
#include <algorithm>

AnimationRecorder::AnimationRecorder(FallProcessor& fallProcessor)
    : fallProcessor_(fallProcessor)
{
}

AnimationRecorder::~AnimationRecorder() {
}

/**
 * @brief 记录下落和填充过程
 */
void AnimationRecorder::recordFallAndRefill(std::vector<std::vector<Fruit>>& map,
                                             FruitGenerator& fruitGenerator,
                                             FallStep& outFallStep) {
    outFallStep.moves.clear();
    outFallStep.newFruits.clear();
    
    // 1. 处理下落
    bool hasFall = fallProcessor_.hasEmptySlots(map);
    
    if (hasFall) {
        for (int col = 0; col < MAP_SIZE; col++) {
            // 记录下落前的位置
            std::vector<std::tuple<int, int, Fruit>> before;
            for (int row = 0; row < MAP_SIZE; ++row) {
                if (map[row][col].type != FruitType::EMPTY) {
                    before.emplace_back(row, col, map[row][col]);
                }
            }
            
            // 执行下落
            fallProcessor_.processColumnFall(map, col);
            
            // 记录下落后的位置
            std::vector<std::pair<int, int>> after;
            for (int row = 0; row < MAP_SIZE; ++row) {
                if (map[row][col].type != FruitType::EMPTY) {
                    after.emplace_back(row, col);
                }
            }
            
            // 生成移动记录
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
                    move.toRow = toRow;
                    move.toCol = toCol;
                    outFallStep.moves.push_back(move);
                }
            }
        }
    }
    
    // 2. 填充空位
    auto newSlots = fallProcessor_.fillEmptySlots(map, fruitGenerator);
    for (const auto& pos : newSlots) {
        NewFruit nf;
        nf.row = pos.first;
        nf.col = pos.second;
        nf.type = map[pos.first][pos.second].type;
        nf.special = map[pos.first][pos.second].special;
        outFallStep.newFruits.push_back(nf);
    }
}

/**
 * @brief 记录消除过程
 */
void AnimationRecorder::recordElimination(std::vector<std::vector<Fruit>>& map,
                                           const std::set<std::pair<int, int>>& specialPositions,
                                           EliminationStep& outElimStep) {
    outElimStep.positions.clear();
    outElimStep.types.clear();
    outElimStep.bombEffects.clear();
    
    // 注意：这个方法假设调用前已经标记了 isMatched
    // 这里只负责记录和执行消除，不负责标记
    
    // 处理特殊元素效果（被标记消除的特殊元素触发效果）
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map[row][col].isMatched) {
                // 记录消除位置和原始类型（在消除前保存，用于成就检测）
                outElimStep.positions.push_back({row, col});
                outElimStep.types.push_back(map[row][col].type);
                
                // 如果是特殊元素，记录炸弹特效
                if (map[row][col].special != SpecialType::NONE) {
                    recordBombEffect(row, col, map[row][col].special, outElimStep.bombEffects);
                }
            }
        }
    }
    
    // 执行消除
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map[row][col].isMatched) {
                map[row][col].type = FruitType::EMPTY;
                map[row][col].special = SpecialType::NONE;
                map[row][col].isMatched = false;
            }
        }
    }
}

/**
 * @brief 记录炸弹特效
 */
void AnimationRecorder::recordBombEffect(int row, int col, SpecialType special,
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
