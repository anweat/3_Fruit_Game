#include "FallProcessor.h"
#include <algorithm>

FallProcessor::FallProcessor() {
    // 构造函数 - 无需初始化
}

FallProcessor::~FallProcessor() {
    // 析构函数 - 无需清理
}

/**
 * @brief 处理地图上所有需要下落的水果
 * 
 * 实现逻辑：
 * 1. 对每一列进行扫描，从下往上处理
 * 2. 将非空水果向下移动填充空位
 * 3. 记录所有移动轨迹供动画使用
 */
std::vector<std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>> 
FallProcessor::processFall(std::vector<std::vector<Fruit>>& map, FruitGenerator& generator) {
    std::vector<std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>> allSteps;
    
    // 对每一列处理下落
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> currentStep;
    
    for (int col = 0; col < MAP_SIZE; col++) {
        // 从下往上扫描，寻找空位
        int emptyRow = MAP_SIZE - 1;  // 当前空位指针
        
        // 从下往上寻找第一个非空位置
        for (int row = MAP_SIZE - 1; row >= 0; row--) {
            if (map[row][col].type != FruitType::EMPTY) {
                // 找到非空水果
                if (row != emptyRow) {
                    // 需要下落
                    map[emptyRow][col] = map[row][col];
                    map[emptyRow][col].row = emptyRow;
                    map[emptyRow][col].col = col;
                    
                    // 记录移动
                    currentStep.push_back({{row, col}, {emptyRow, col}});
                    
                    // 原位置清空
                    map[row][col].type = FruitType::EMPTY;
                    map[row][col].special = SpecialType::NONE;
                }
                emptyRow--;  // 空位指针上移
            }
        }
    }
    
    if (!currentStep.empty()) {
        allSteps.push_back(currentStep);
    }
    
    // 填充新的水果
    auto newFruits = fillEmptySlots(map, generator);
    if (!newFruits.empty()) {
        std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> fillStep;
        for (const auto& pos : newFruits) {
            // 新水果从顶部上方出现
            fillStep.push_back({{-1, pos.second}, pos});
        }
        allSteps.push_back(fillStep);
    }
    
    return allSteps;
}

/**
 * @brief 处理单列的下落
 */
std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> 
FallProcessor::processColumnFall(std::vector<std::vector<Fruit>>& map, int col) {
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> moves;
    
    if (col < 0 || col >= MAP_SIZE) {
        return moves;
    }
    
    int emptyRow = MAP_SIZE - 1;
    
    for (int row = MAP_SIZE - 1; row >= 0; row--) {
        if (map[row][col].type != FruitType::EMPTY) {
            if (row != emptyRow) {
                // 需要下落
                map[emptyRow][col] = map[row][col];
                map[emptyRow][col].row = emptyRow;
                map[emptyRow][col].col = col;
                
                moves.push_back({{row, col}, {emptyRow, col}});
                
                map[row][col].type = FruitType::EMPTY;
                map[row][col].special = SpecialType::NONE;
            }
            emptyRow--;
        }
    }
    
    return moves;
}

/**
 * @brief 填充空位
 */
std::vector<std::pair<int, int>> 
FallProcessor::fillEmptySlots(std::vector<std::vector<Fruit>>& map, 
                               FruitGenerator& generator) {
    std::vector<std::pair<int, int>> newPositions;
    
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map[row][col].type == FruitType::EMPTY) {
                // 生成新水果
                FruitType newType = generator.generateRandomFruit();
                map[row][col] = Fruit(newType, row, col);
                newPositions.push_back({row, col});
            }
        }
    }
    
    return newPositions;
}

/**
 * @brief 检查地图是否有空位
 */
bool FallProcessor::hasEmptySlots(const std::vector<std::vector<Fruit>>& map) const {
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map[row][col].type == FruitType::EMPTY) {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 获取所有空位的位置
 */
std::vector<std::pair<int, int>> 
FallProcessor::getEmptySlots(const std::vector<std::vector<Fruit>>& map) const {
    std::vector<std::pair<int, int>> emptySlots;
    
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map[row][col].type == FruitType::EMPTY) {
                emptySlots.push_back({row, col});
            }
        }
    }
    
    return emptySlots;
}

/**
 * @brief 计算某个位置下方的空位数量
 */
int FallProcessor::countEmptySlotsBelow(const std::vector<std::vector<Fruit>>& map, 
                                        int row, int col) const {
    if (row < 0 || row >= MAP_SIZE || col < 0 || col >= MAP_SIZE) {
        return 0;
    }
    
    int count = 0;
    for (int r = row + 1; r < MAP_SIZE; r++) {
        if (map[r][col].type == FruitType::EMPTY) {
            count++;
        }
    }
    
    return count;
}
