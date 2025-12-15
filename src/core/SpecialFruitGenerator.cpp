#include "SpecialFruitGenerator.h"
#include <map>

SpecialFruitGenerator::SpecialFruitGenerator() {
    // 构造函�?
}

SpecialFruitGenerator::~SpecialFruitGenerator() {
    // 析构函数
}

/**
 * @brief 根据匹配结果判断应该生成什么特殊元�?
 */
SpecialType SpecialFruitGenerator::determineSpecialType(const MatchResult& match) const {
    // 1. 检查是否是L形或T形（优先级最高）
    if (match.direction == MatchDirection::L_SHAPE || match.direction == MatchDirection::T_SHAPE) {
        return SpecialType::DIAMOND;  // 菱形炸弹
    }
    
    // 2. 检�?个及以上直线消除 �?万能炸弹
    if (match.matchCount >= 5) {
        if (match.direction == MatchDirection::HORIZONTAL || match.direction == MatchDirection::VERTICAL) {
            return SpecialType::RAINBOW;  // 万能炸弹
        }
    }
    
    // 3. 检�?个水果消�?�?直线炸弹
    if (match.matchCount == 4) {
        if (match.direction == MatchDirection::HORIZONTAL) {
            return SpecialType::LINE_H;  // 横向直线炸弹
        } else if (match.direction == MatchDirection::VERTICAL) {
            return SpecialType::LINE_V;  // 纵向直线炸弹
        }
    }
    
    // 默认不生成特殊元�?
    return SpecialType::NONE;
}

/**
 * @brief 在地图上生成特殊水果
 * 
 * 说明�?
 * - RAINBOW 类型会将水果类型设为 CANDY（独立彩虹糖，不参与普通三消）
 * - 其他类型保持原有水果类型
 * - 如果目标位置已有炸弹，返�?{-2, -2} 表示需要触发组合效�?
 */
std::pair<int, int> SpecialFruitGenerator::generateSpecialFruit(
    std::vector<std::vector<Fruit>>& map,
    const MatchResult& match,
    SpecialType specialType) {
    
    if (specialType == SpecialType::NONE) {
        return {-1, -1};  // 不生成特殊元�?
    }
    
    // 计算生成位置（中心位置）
    std::pair<int, int> pos = calculateGeneratePosition(match);
    
    if (pos.first < 0 || pos.first >= static_cast<int>(map.size()) || pos.second < 0 || pos.second >= static_cast<int>(map.size())) {
        return {-1, -1};  // 位置无效
    }
    
    // 检查目标位置是否已经有炸弹
    if (map[pos.first][pos.second].special != SpecialType::NONE) {
        // 返回特殊标记，表示需要触发组合效�?
        return {-2, -2};
    }
    
    // 设置特殊属�?
    map[pos.first][pos.second].special = specialType;
    map[pos.first][pos.second].isMatched = false;  // 不标记为已匹配，保留在地图上
    
    // RAINBOW 类型特殊处理：将水果类型改为 CANDY（独立彩虹糖�?
    if (specialType == SpecialType::RAINBOW) {
        map[pos.first][pos.second].type = FruitType::CANDY;
    }
    
    return pos;
}

/**
 * @brief 检测L形或T形匹�?
 */
bool SpecialFruitGenerator::detectLTShape(
    const std::vector<std::vector<Fruit>>& map,
    FruitType fruitType,
    int row, int col,
    std::vector<std::pair<int, int>>& positions) const {
    
    positions.clear();
    
    // 检测L形的8种可能形�?
    // L形：一条横线和一条竖线的组合，有8种方�?
    
    // 横向扫描
    std::vector<int> horizontalMatches;
    for (int c = 0; c < static_cast<int>(map.size()); c++) {
        if (map[row][c].type == fruitType && !map[row][c].isMatched) {
            horizontalMatches.push_back(c);
        } else if (!horizontalMatches.empty()) {
            break;  // 遇到不匹配的，停�?
        }
    }
    
    // 纵向扫描
    std::vector<int> verticalMatches;
    for (int r = 0; r < static_cast<int>(map.size()); r++) {
        if (map[r][col].type == fruitType && !map[r][col].isMatched) {
            verticalMatches.push_back(r);
        } else if (!verticalMatches.empty()) {
            break;  // 遇到不匹配的，停�?
        }
    }
    
    // 检查是否形成L形或T形（横线至少3个，竖线至少3个，且有交点�?
    if (horizontalMatches.size() >= 3 && verticalMatches.size() >= 3) {
        // 检查交�?
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
 * @brief 计算特殊水果应该生成的位置（通常是匹配序列的中心位置�?
 * 
 * 策略�?
 * - 直线匹配：选择中间位置
 * - L�?T形：选择交叉点（横竖线的交点�?
 */
std::pair<int, int> SpecialFruitGenerator::calculateGeneratePosition(const MatchResult& match) const {
    if (match.positions.empty()) {
        return {-1, -1};
    }
    
    // 对于L形或T形，精确计算交叉点位�?
    if (match.direction == MatchDirection::L_SHAPE || match.direction == MatchDirection::T_SHAPE) {
        // 统计每个行和列的出现次数
        std::map<int, int> rowCount;  // 行号 -> 出现次数
        std::map<int, int> colCount;  // 列号 -> 出现次数
        
        for (const auto& pos : match.positions) {
            rowCount[pos.first]++;
            colCount[pos.second]++;
        }
        
        // 找到出现次数 >= 3 的行和列
        std::vector<int> majorRows;  // 主要行（�?个及以上元素�?
        std::vector<int> majorCols;  // 主要列（�?个及以上元素�?
        
        for (const auto& pair : rowCount) {
            if (pair.second >= 3) {
                majorRows.push_back(pair.first);
            }
        }
        for (const auto& pair : colCount) {
            if (pair.second >= 3) {
                majorCols.push_back(pair.first);
            }
        }
        
        // 交叉点必定是主要行和主要列的交点
        // T形：会有1�?�?�?2�?�?
        // L形：会有1�?�?
        if (!majorRows.empty() && !majorCols.empty()) {
            int targetRow = majorRows[0];
            int targetCol = majorCols[0];
            
            // 检查这个交叉点是否在匹配位置列表中
            for (const auto& pos : match.positions) {
                if (pos.first == targetRow && pos.second == targetCol) {
                    return pos;  // 找到交叉�?
                }
            }
            
            // 如果没找到（理论上不应该发生），检查其他可能的交叉�?
            for (int row : majorRows) {
                for (int col : majorCols) {
                    for (const auto& pos : match.positions) {
                        if (pos.first == row && pos.second == col) {
                            return pos;
                        }
                    }
                }
            }
        }
        
        // 如果上述逻辑都失败，退回到中心位置
        size_t midIndex = match.positions.size() / 2;
        return match.positions[midIndex];
    }
    
    // 对于直线匹配（HORIZONTAL/VERTICAL），选择中间位置
    if (match.direction == MatchDirection::HORIZONTAL || match.direction == MatchDirection::VERTICAL) {
        size_t midIndex = match.positions.size() / 2;
        return match.positions[midIndex];
    }
    
    // 其他情况（理论上不应该到这里），返回第一个位�?
    return match.positions[0];
}
