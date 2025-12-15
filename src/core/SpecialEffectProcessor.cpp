#include "SpecialEffectProcessor.h"
#include <algorithm>

SpecialEffectProcessor::SpecialEffectProcessor() {
    // 构造函�?
}

SpecialEffectProcessor::~SpecialEffectProcessor() {
    // 析构函数
}

/**
 * @brief 触发特殊元素效果
 */
bool SpecialEffectProcessor::triggerSpecialEffect(
    std::vector<std::vector<Fruit>>& map,
    int row, int col,
    std::set<std::pair<int, int>>& affectedPositions) {
    
    // 使用一个集合来追踪已经触发过的特殊元素位置，防止无限递归
    std::set<std::pair<int, int>> triggeredSpecials;
    return triggerSpecialEffectInternal(map, row, col, affectedPositions, triggeredSpecials);
}

/**
 * @brief 内部递归函数，带有已触发特殊元素的追�?
 */
bool SpecialEffectProcessor::triggerSpecialEffectInternal(
    std::vector<std::vector<Fruit>>& map,
    int row, int col,
    std::set<std::pair<int, int>>& affectedPositions,
    std::set<std::pair<int, int>>& triggeredSpecials) {
    
    if (!isValidPosition(map, row, col)) {
        return false;
    }
    
    // 检查是否已经触发过这个位置的特殊元�?
    if (triggeredSpecials.count({row, col}) > 0) {
        return false;  // 已经触发过，避免死循�?
    }
    
    SpecialType specialType = map[row][col].special;
    
    if (specialType == SpecialType::NONE) {
        return false;  // 不是特殊元素
    }
    
    // 标记当前位置为已触发
    triggeredSpecials.insert({row, col});
    
    // 存储本次效果影响的位�?
    std::set<std::pair<int, int>> currentAffected;
    
    // 根据特殊元素类型触发对应效果
    switch (specialType) {
        case SpecialType::LINE_H:
            effectLineH(map, row, currentAffected);
            break;
        case SpecialType::LINE_V:
            effectLineV(map, col, currentAffected);
            break;
        case SpecialType::DIAMOND:
            effectDiamond(map, row, col, currentAffected);
            break;
        case SpecialType::RAINBOW:
            effectRainbow(map, row, col, currentAffected);
            break;
        default:
            return false;
    }
    
    // 将当前效果的位置加入总集�?
    affectedPositions.insert(currentAffected.begin(), currentAffected.end());
    
    // 检查受影响位置中是否有其他特殊元素，触发连锁反�?
    for (const auto& pos : currentAffected) {
        int r = pos.first;
        int c = pos.second;
        
        // 如果该位置有特殊元素且还未被触发�?
        if (isValidPosition(map, r, c) && 
            map[r][c].special != SpecialType::NONE && 
            triggeredSpecials.count({r, c}) == 0) {
            
            // 递归触发该特殊元素的效果
            triggerSpecialEffectInternal(map, r, c, affectedPositions, triggeredSpecials);
        }
    }
    
    return true;
}

/**
 * @brief 检测并触发特殊元素组合效果
 */
bool SpecialEffectProcessor::triggerCombinationEffect(
    std::vector<std::vector<Fruit>>& map,
    int row1, int col1,
    int row2, int col2,
    std::set<std::pair<int, int>>& affectedPositions) {
    
    if (!isValidPosition(map, row1, col1) || !isValidPosition(map, row2, col2)) {
        return false;
    }
    
    SpecialType special1 = map[row1][col1].special;
    SpecialType special2 = map[row2][col2].special;
    
    // 至少有一个是特殊元素
    if (special1 == SpecialType::NONE && special2 == SpecialType::NONE) {
        return false;
    }
    
    // 取中心点作为效果触发�?
    int centerRow = (row1 + row2) / 2;
    int centerCol = (col1 + col2) / 2;
    
    // 万能炸弹+万能炸弹 �?全屏消除
    if (special1 == SpecialType::RAINBOW && special2 == SpecialType::RAINBOW) {
        comboRainbowRainbow(map, affectedPositions);
        return true;
    }
    
    // 万能炸弹+其他特殊元素
    if (special1 == SpecialType::RAINBOW && special2 != SpecialType::NONE) {
        comboSpecialRainbow(map, special2, map[row2][col2].type, affectedPositions);
        return true;
    }
    if (special2 == SpecialType::RAINBOW && special1 != SpecialType::NONE) {
        comboSpecialRainbow(map, special1, map[row1][col1].type, affectedPositions);
        return true;
    }
    
    // 菱形+菱形 �?7×7大范�?
    if (special1 == SpecialType::DIAMOND && special2 == SpecialType::DIAMOND) {
        comboDiamondDiamond(map, centerRow, centerCol, affectedPositions);
        return true;
    }
    
    // 直线+菱形 �?3�?3�?
    if ((special1 == SpecialType::LINE_H || special1 == SpecialType::LINE_V) &&
        special2 == SpecialType::DIAMOND) {
        comboLineDiamond(map, centerRow, centerCol, affectedPositions);
        return true;
    }
    if ((special2 == SpecialType::LINE_H || special2 == SpecialType::LINE_V) &&
        special1 == SpecialType::DIAMOND) {
        comboLineDiamond(map, centerRow, centerCol, affectedPositions);
        return true;
    }
    
    // 直线+直线 �?十字消除
    if ((special1 == SpecialType::LINE_H || special1 == SpecialType::LINE_V) &&
        (special2 == SpecialType::LINE_H || special2 == SpecialType::LINE_V)) {
        comboLineLine(map, centerRow, centerCol, affectedPositions);
        return true;
    }
    
    return false;
}

/**
 * @brief 直线炸弹（横向）效果 - 消除整行
 */
void SpecialEffectProcessor::effectLineH(
    std::vector<std::vector<Fruit>>& map,
    int row,
    std::set<std::pair<int, int>>& affectedPositions) {
    
    for (int col = 0; col < static_cast<int>(map.size()); col++) {
        affectedPositions.insert({row, col});
    }
}

/**
 * @brief 直线炸弹（纵向）效果 - 消除整列
 */
void SpecialEffectProcessor::effectLineV(
    std::vector<std::vector<Fruit>>& map,
    int col,
    std::set<std::pair<int, int>>& affectedPositions) {
    
    for (int row = 0; row < static_cast<int>(map.size()); row++) {
        affectedPositions.insert({row, col});
    }
}

/**
 * @brief 菱形炸弹效果 - 消除5×5菱形范围（曼哈顿距离�?�?
 */
void SpecialEffectProcessor::effectDiamond(
    std::vector<std::vector<Fruit>>& map,
    int row, int col,
    std::set<std::pair<int, int>>& affectedPositions,
    int range) {
    
    for (int dr = -range; dr <= range; dr++) {
        for (int dc = -range; dc <= range; dc++) {
            // 菱形条件：曼哈顿距离 �?range
            if (std::abs(dr) + std::abs(dc) <= range) {
                int newRow = row + dr;
                int newCol = col + dc;
                if (isValidPosition(map, newRow, newCol)) {
                    affectedPositions.insert({newRow, newCol});
                }
            }
        }
    }
}

/**
 * @brief 万能炸弹效果 - 消除场上所有同类型水果
 */
void SpecialEffectProcessor::effectRainbow(
    std::vector<std::vector<Fruit>>& map,
    int row, int col,
    std::set<std::pair<int, int>>& affectedPositions) {
    
    // 找到交换目标的水果类型（需要外部传入，这里简化处理）
    // 遍历相邻位置，找到一个非空水果作为目�?
    FruitType targetType = FruitType::EMPTY;
    
    // 检查四个方�?
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (auto& dir : directions) {
        int newRow = row + dir[0];
        int newCol = col + dir[1];
        if (isValidPosition(map, newRow, newCol) && 
            map[newRow][newCol].type != FruitType::EMPTY) {
            targetType = map[newRow][newCol].type;
            break;
        }
    }
    
    if (targetType == FruitType::EMPTY) {
        return;  // 没有找到目标类型
    }
    
    // 消除所有该类型的水�?
    for (int r = 0; r < static_cast<int>(map.size()); r++) {
        for (int c = 0; c < static_cast<int>(map.size()); c++) {
            if (map[r][c].type == targetType) {
                affectedPositions.insert({r, c});
            }
        }
    }
}

/**
 * @brief 组合效果：直�?直线 �?十字消除
 */
void SpecialEffectProcessor::comboLineLine(
    std::vector<std::vector<Fruit>>& map,
    int row, int col,
    std::set<std::pair<int, int>>& affectedPositions) {
    
    // 消除整行
    effectLineH(map, row, affectedPositions);
    // 消除整列
    effectLineV(map, col, affectedPositions);
}

/**
 * @brief 组合效果：直�?菱形 �?3�?3列消�?
 */
void SpecialEffectProcessor::comboLineDiamond(
    std::vector<std::vector<Fruit>>& map,
    int row, int col,
    std::set<std::pair<int, int>>& affectedPositions) {
    
    // 消除中心行及上下各一行（�?行）
    for (int r = row - 1; r <= row + 1; r++) {
        if (r >= 0 && r < static_cast<int>(map.size())) {
            effectLineH(map, r, affectedPositions);
        }
    }
    
    // 消除中心列及左右各一列（�?列）
    for (int c = col - 1; c <= col + 1; c++) {
        if (c >= 0 && c < static_cast<int>(map.size())) {
            effectLineV(map, c, affectedPositions);
        }
    }
}

/**
 * @brief 组合效果：菱�?菱形 �?7×7大范围消�?
 */
void SpecialEffectProcessor::comboDiamondDiamond(
    std::vector<std::vector<Fruit>>& map,
    int row, int col,
    std::set<std::pair<int, int>>& affectedPositions) {
    
    // 使用更大的range�?代表7×7范围，曼哈顿距离�?�?
    effectDiamond(map, row, col, affectedPositions, 3);
}

/**
 * @brief 组合效果：任�?万能 �?将场上某类型全部变为该特殊元素并引爆
 */
void SpecialEffectProcessor::comboSpecialRainbow(
    std::vector<std::vector<Fruit>>& map,
    SpecialType specialType,
    FruitType targetType,
    std::set<std::pair<int, int>>& affectedPositions) {
    
    // 找到所有目标类型的水果，将它们变为特殊元素并引�?
    std::vector<std::pair<int, int>> targets;
    for (int r = 0; r < static_cast<int>(map.size()); r++) {
        for (int c = 0; c < static_cast<int>(map.size()); c++) {
            if (map[r][c].type == targetType) {
                targets.push_back({r, c});
            }
        }
    }
    
    // 在每个目标位置触发特殊效�?
    for (const auto& pos : targets) {
        int r = pos.first;
        int c = pos.second;
        
        // 根据特殊元素类型触发相应效果
        switch (specialType) {
            case SpecialType::LINE_H:
                effectLineH(map, r, affectedPositions);
                break;
            case SpecialType::LINE_V:
                effectLineV(map, c, affectedPositions);
                break;
            case SpecialType::DIAMOND:
                effectDiamond(map, r, c, affectedPositions);
                break;
            default:
                affectedPositions.insert({r, c});
                break;
        }
    }
}

/**
 * @brief 组合效果：万�?万能 �?全屏消除
 */
void SpecialEffectProcessor::comboRainbowRainbow(
    std::vector<std::vector<Fruit>>& map,
    std::set<std::pair<int, int>>& affectedPositions) {
    
    // 消除所有位�?
    for (int row = 0; row < static_cast<int>(map.size()); row++) {
        for (int col = 0; col < static_cast<int>(map.size()); col++) {
            affectedPositions.insert({row, col});
        }
    }
}

/**
 * @brief 检查位置是否有�?
 */
bool SpecialEffectProcessor::isValidPosition(const std::vector<std::vector<Fruit>>& map, int row, int col) const {
    return row >= 0 && row < static_cast<int>(map.size()) && col >= 0 && col < static_cast<int>(map.size());
}
