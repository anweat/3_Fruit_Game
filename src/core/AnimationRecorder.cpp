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
    
    // 🔧 关键修复：直接使用 processFall 的返回值，它已经正确记录了所有移动
    auto allSteps = fallProcessor_.processFall(map, fruitGenerator, static_cast<int>(map.size()));
    
    // allSteps[0] 是下落移动，allSteps[1] 是新生成的水果（如果有）
    if (!allSteps.empty()) {
        // 第一步：下落移动
        for (const auto& move : allSteps[0]) {
            FallMove fm;
            fm.fromRow = move.first.first;
            fm.fromCol = move.first.second;
            fm.toRow = move.second.first;
            fm.toCol = move.second.second;
            // 🔧 关键：记录水果类型（此时map已经更新，从目标位置读取）
            fm.type = map[fm.toRow][fm.toCol].type;
            fm.special = map[fm.toRow][fm.toCol].special;
            outFallStep.moves.push_back(fm);
        }
        
        // 第二步：新生成的水果
        if (allSteps.size() > 1) {
            for (const auto& newPos : allSteps[1]) {
                NewFruit nf;
                nf.row = newPos.second.first;
                nf.col = newPos.second.second;
                nf.type = map[nf.row][nf.col].type;
                nf.special = map[nf.row][nf.col].special;
                outFallStep.newFruits.push_back(nf);
            }
        }
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
    
    // 注意：这个方法假设调用前已经标记�?isMatched
    // 这里只负责记录和执行消除，不负责标记
    
    // 处理特殊元素效果（被标记消除的特殊元素触发效果）
    for (int row = 0; row < static_cast<int>(map.size()); row++) {
        for (int col = 0; col < static_cast<int>(map.size()); col++) {
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
    for (int row = 0; row < static_cast<int>(map.size()); row++) {
        for (int col = 0; col < static_cast<int>(map.size()); col++) {
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
