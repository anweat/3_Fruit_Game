#include "SpecialFruitGenerator.h"

SpecialFruitGenerator::SpecialFruitGenerator() {
    // 构造函数
}

SpecialFruitGenerator::~SpecialFruitGenerator() {
    // 析构函数
}

/**
 * @brief 根据匹配结果判断应该生成什么特殊元素
 */
SpecialType SpecialFruitGenerator::determineSpecialType(const MatchResult& match) const {
    // 1. 检查是否是L形或T形（优先级最高）
    if (match.direction == MatchDirection::L_SHAPE || match.direction == MatchDirection::T_SHAPE) {
        return SpecialType::DIAMOND;  // 菱形炸弹
    }
    
    // 2. 检查5个及以上直线消除 → 万能炸弹
    if (match.matchCount >= 5) {
        if (match.direction == MatchDirection::HORIZONTAL || match.direction == MatchDirection::VERTICAL) {
            return SpecialType::RAINBOW;  // 万能炸弹
        }
    }
    
    // 3. 检查4个水果消除 → 直线炸弹
    if (match.matchCount == 4) {
        if (match.direction == MatchDirection::HORIZONTAL) {
            return SpecialType::LINE_H;  // 横向直线炸弹
        } else if (match.direction == MatchDirection::VERTICAL) {
            return SpecialType::LINE_V;  // 纵向直线炸弹
        }
    }
    
    // 默认不生成特殊元素
    return SpecialType::NONE;
}

/**
 * @brief 在地图上生成特殊水果
 * 
 * 说明：
 * - RAINBOW 类型会将水果类型设为 CANDY（独立彩虹糖，不参与普通三消）
 * - 其他类型保持原有水果类型
 * - 如果目标位置已有炸弹，返回 {-2, -2} 表示需要触发组合效果
 */
std::pair<int, int> SpecialFruitGenerator::generateSpecialFruit(
    std::vector<std::vector<Fruit>>& map,
    const MatchResult& match,
    SpecialType specialType) {
    
    if (specialType == SpecialType::NONE) {
        return {-1, -1};  // 不生成特殊元素
    }
    
    // 计算生成位置（中心位置）
    std::pair<int, int> pos = calculateGeneratePosition(match);
    
    if (pos.first < 0 || pos.first >= MAP_SIZE || pos.second < 0 || pos.second >= MAP_SIZE) {
        return {-1, -1};  // 位置无效
    }
    
    // 检查目标位置是否已经有炸弹
    if (map[pos.first][pos.second].special != SpecialType::NONE) {
        // 返回特殊标记，表示需要触发组合效果
        return {-2, -2};
    }
    
    // 设置特殊属性
    map[pos.first][pos.second].special = specialType;
    map[pos.first][pos.second].isMatched = false;  // 不标记为已匹配，保留在地图上
    
    // RAINBOW 类型特殊处理：将水果类型改为 CANDY（独立彩虹糖）
    if (specialType == SpecialType::RAINBOW) {
        map[pos.first][pos.second].type = FruitType::CANDY;
    }
    
    return pos;
}

/**
 * @brief 检测L形或T形匹配
 */
bool SpecialFruitGenerator::detectLTShape(
    const std::vector<std::vector<Fruit>>& map,
    FruitType fruitType,
    int row, int col,
    std::vector<std::pair<int, int>>& positions) const {
    
    positions.clear();
    
    // 检测L形的8种可能形态
    // L形：一条横线和一条竖线的组合，有8种方向
    
    // 横向扫描
    std::vector<int> horizontalMatches;
    for (int c = 0; c < MAP_SIZE; c++) {
        if (map[row][c].type == fruitType && !map[row][c].isMatched) {
            horizontalMatches.push_back(c);
        } else if (!horizontalMatches.empty()) {
            break;  // 遇到不匹配的，停止
        }
    }
    
    // 纵向扫描
    std::vector<int> verticalMatches;
    for (int r = 0; r < MAP_SIZE; r++) {
        if (map[r][col].type == fruitType && !map[r][col].isMatched) {
            verticalMatches.push_back(r);
        } else if (!verticalMatches.empty()) {
            break;  // 遇到不匹配的，停止
        }
    }
    
    // 检查是否形成L形或T形（横线至少3个，竖线至少3个，且有交点）
    if (horizontalMatches.size() >= 3 && verticalMatches.size() >= 3) {
        // 检查交点
        bool hasIntersection = false;
        for (int c : horizontalMatches) {
            for (int r : verticalMatches) {
                if (r == row && c == col) {
                    hasIntersection = true;
                    break;
                }
            }
        }
        
        if (hasIntersection) {
            // 填充positions
            for (int c : horizontalMatches) {
                positions.push_back({row, c});
            }
            for (int r : verticalMatches) {
                if (r != row) {  // 避免重复添加交点
                    positions.push_back({r, col});
                }
            }
            return true;
        }
    }
    
    return false;
}

/**
 * @brief 计算特殊水果应该生成的位置（通常是匹配序列的中心位置）
 */
std::pair<int, int> SpecialFruitGenerator::calculateGeneratePosition(const MatchResult& match) const {
    if (match.positions.empty()) {
        return {-1, -1};
    }
    
    // 对于L形或T形，选择交叉点作为生成位置
    if (match.direction == MatchDirection::L_SHAPE || match.direction == MatchDirection::T_SHAPE) {
        // 简单策略：选择第一个位置（实际应该是交叉点，这里简化处理）
        // TODO: 更精确的交叉点计算
        return match.positions[0];
    }
    
    // 对于直线匹配，选择中间位置
    size_t midIndex = match.positions.size() / 2;
    return match.positions[midIndex];
}
