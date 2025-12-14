#include "PropManager.h"
#include "../core/FruitTypes.h"
#include <iostream>

PropManager::PropManager()
    : hammerCount_(3)      // 初始时每种道具3个
    , clampCount_(3)
    , magicWandCount_(3)
{
    // PropManager 初始化完成
}

PropManager::~PropManager() {
    // PropManager 销毁
}

bool PropManager::hasProp(PropType propType) const {
    return getPropCount(propType) > 0;
}

int PropManager::getPropCount(PropType propType) const {
    switch (propType) {
        case PropType::HAMMER:
            return hammerCount_;
        case PropType::CLAMP:
            return clampCount_;
        case PropType::MAGIC_WAND:
            return magicWandCount_;
        default:
            return 0;
    }
}

void PropManager::addProp(PropType propType, int count) {
    if (count <= 0) return;
    
    switch (propType) {
        case PropType::HAMMER:
            hammerCount_ += count;
            break;
        case PropType::CLAMP:
            clampCount_ += count;
            break;
        case PropType::MAGIC_WAND:
            magicWandCount_ += count;
            break;
    }
}

void PropManager::setPropCount(PropType propType, int count) {
    if (count < 0) count = 0;
    
    switch (propType) {
        case PropType::HAMMER:
            hammerCount_ = count;
            break;
        case PropType::CLAMP:
            clampCount_ = count;
            break;
        case PropType::MAGIC_WAND:
            magicWandCount_ = count;
            break;
    }
}

void PropManager::setAllProps(int hammer, int clamp, int magicWand) {
    hammerCount_ = (hammer >= 0) ? hammer : 0;
    clampCount_ = (clamp >= 0) ? clamp : 0;
    magicWandCount_ = (magicWand >= 0) ? magicWand : 0;
}

/**
 * @brief 使用锤子道具：消除单个水果
 */
bool PropManager::useHammer(const std::vector<std::vector<Fruit>>& map,
                             int row, int col,
                             std::set<std::pair<int, int>>& outAffected) {
    if (!hasProp(PropType::HAMMER)) {
        return false;
    }
    
    // 检查目标位置是否有效
    if (row < 0 || row >= MAP_SIZE || col < 0 || col >= MAP_SIZE) {
        return false;
    }
    
    // 检查是否是空位
    if (map[row][col].type == FruitType::EMPTY) {
        return false;
    }
    
    // 消耗道具
    hammerCount_--;
    
    // 添加受影响的位置
    outAffected.clear();
    outAffected.insert({row, col});
    
    return true;
}

/**
 * @brief 使用夹子道具:强制交换任意相邻两个水果（不需要匹配）
 */
bool PropManager::useClamp(const std::vector<std::vector<Fruit>>& map,
                            int row1, int col1,
                            int row2, int col2) {
    if (!hasProp(PropType::CLAMP)) {
        return false;
    }
    
    // 检查位置是否有效
    if (row1 < 0 || row1 >= MAP_SIZE || col1 < 0 || col1 >= MAP_SIZE ||
        row2 < 0 || row2 >= MAP_SIZE || col2 < 0 || col2 >= MAP_SIZE) {
        return false;
    }
    
    // 检查是否相邻
    int distance = std::abs(row1 - row2) + std::abs(col1 - col2);
    if (distance != 1) {
        return false;
    }
    
    // 检查是否有空位
    if (map[row1][col1].type == FruitType::EMPTY || 
        map[row2][col2].type == FruitType::EMPTY) {
        return false;
    }
    
    // 消耗道具
    clampCount_--;
    
    return true;
}

/**
 * @brief 使用魔法棒道具：消除整个类型的水果
 */
bool PropManager::useMagicWand(const std::vector<std::vector<Fruit>>& map,
                                int row, int col,
                                std::set<std::pair<int, int>>& outAffected) {
    if (!hasProp(PropType::MAGIC_WAND)) {
        return false;
    }
    
    // 检查目标位置是否有效
    if (row < 0 || row >= MAP_SIZE || col < 0 || col >= MAP_SIZE) {
        return false;
    }
    
    // 检查是否是空位
    if (map[row][col].type == FruitType::EMPTY) {
        return false;
    }
    
    FruitType targetType = map[row][col].type;
    
    // 不能对CANDY使用
    if (targetType == FruitType::CANDY) {
        return false;
    }
    
    // 消耗道具
    magicWandCount_--;
    
    // 找到所有相同类型的水果
    outAffected.clear();
    for (int r = 0; r < MAP_SIZE; r++) {
        for (int c = 0; c < MAP_SIZE; c++) {
            if (map[r][c].type == targetType) {
                outAffected.insert({r, c});
            }
        }
    }
    
    return true;
}
