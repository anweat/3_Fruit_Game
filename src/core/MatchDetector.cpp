#include "MatchDetector.h"
#include <algorithm>
#include <map>

MatchDetector::MatchDetector() {
    // 默认构造函数
}

MatchDetector::~MatchDetector() {
    // 默认析构函数
}

std::vector<MatchResult> MatchDetector::detectMatches(const std::vector<std::vector<Fruit>>& map) {
    // 标记数组，记录哪些位置已被匹配
    bool matched[MAP_SIZE][MAP_SIZE] = {false};
    
    std::vector<MatchResult> results;
    
    // 先检测横向匹配
    auto horizontalMatches = detectHorizontalMatches(map, matched);
    results.insert(results.end(), horizontalMatches.begin(), horizontalMatches.end());
    
    // 再检测纵向匹配
    auto verticalMatches = detectVerticalMatches(map, matched);
    results.insert(results.end(), verticalMatches.begin(), verticalMatches.end());
    
    // 合并交叉匹配（L形、T形）
    results = mergeIntersections(results);
    
    return results;
}

std::vector<MatchResult> MatchDetector::detectMatchesAt(const std::vector<std::vector<Fruit>>& map,
                                                         int row, int col) {
    std::vector<MatchResult> results;
    
    // CANDY 类型不参与普通三消匹配
    if (!isValidPosition(row, col) || !isMatchableFruit(map[row][col].type)) {
        return results;
    }
    
    FruitType targetType = map[row][col].type;
    
    // 检测横向匹配（CANDY 不参与）
    int leftCount = 0, rightCount = 0;
    for (int c = col - 1; c >= 0 && isMatchableFruit(map[row][c].type) && map[row][c].type == targetType; c--) {
        leftCount++;
    }
    for (int c = col + 1; c < MAP_SIZE && isMatchableFruit(map[row][c].type) && map[row][c].type == targetType; c++) {
        rightCount++;
    }
    
    int horizontalTotal = leftCount + rightCount + 1;
    if (horizontalTotal >= 3) {
        MatchResult match;
        match.fruitType = targetType;
        match.direction = MatchDirection::HORIZONTAL;
        match.matchCount = horizontalTotal;
        match.specialPosition = {row, col};
        
        for (int c = col - leftCount; c <= col + rightCount; c++) {
            match.positions.push_back({row, c});
        }
        
        match.generateSpecial = determineSpecialType(horizontalTotal, MatchDirection::HORIZONTAL);
        results.push_back(match);
    }
    
    // 检测纵向匹配（CANDY 不参与）
    int upCount = 0, downCount = 0;
    for (int r = row - 1; r >= 0 && isMatchableFruit(map[r][col].type) && map[r][col].type == targetType; r--) {
        upCount++;
    }
    for (int r = row + 1; r < MAP_SIZE && isMatchableFruit(map[r][col].type) && map[r][col].type == targetType; r++) {
        downCount++;
    }
    
    int verticalTotal = upCount + downCount + 1;
    if (verticalTotal >= 3) {
        MatchResult match;
        match.fruitType = targetType;
        match.direction = MatchDirection::VERTICAL;
        match.matchCount = verticalTotal;
        match.specialPosition = {row, col};
        
        for (int r = row - upCount; r <= row + downCount; r++) {
            match.positions.push_back({r, col});
        }
        
        match.generateSpecial = determineSpecialType(verticalTotal, MatchDirection::VERTICAL);
        results.push_back(match);
    }
    
    return results;
}

bool MatchDetector::hasMatches(const std::vector<std::vector<Fruit>>& map) {
    return !detectMatches(map).empty();
}

bool MatchDetector::hasPossibleMoves(const std::vector<std::vector<Fruit>>& map) {
    // 遍历所有位置，尝试与相邻位置交换
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            // 尝试与右边交换
            if (col < MAP_SIZE - 1) {
                if (wouldMatchAfterSwap(map, row, col, row, col + 1)) {
                    return true;
                }
            }
            
            // 尝试与下边交换
            if (row < MAP_SIZE - 1) {
                if (wouldMatchAfterSwap(map, row, col, row + 1, col)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

std::vector<MatchResult> MatchDetector::detectHorizontalMatches(
    const std::vector<std::vector<Fruit>>& map,
    bool matched[MAP_SIZE][MAP_SIZE]) {
    
    std::vector<MatchResult> results;
    
    for (int row = 0; row < MAP_SIZE; row++) {
        int count = 1;
        FruitType currentType = map[row][0].type;
        
        for (int col = 1; col <= MAP_SIZE; col++) {
            FruitType nextType = (col < MAP_SIZE) ? map[row][col].type : FruitType::EMPTY;
            
            // CANDY 类型不参与普通三消匹配
            if (col < MAP_SIZE && nextType == currentType && isMatchableFruit(currentType)) {
                count++;
            } else {
                // 检查是否形成匹配
                if (count >= 3 && isMatchableFruit(currentType)) {
                    MatchResult match;
                    match.fruitType = currentType;
                    match.direction = MatchDirection::HORIZONTAL;
                    match.matchCount = count;
                    match.specialPosition = {row, col - 1}; // 默认最后一个位置
                    
                    // 添加所有匹配位置
                    for (int i = 0; i < count; i++) {
                        int c = col - 1 - i;
                        match.positions.push_back({row, c});
                        matched[row][c] = true;
                    }
                    
                    match.generateSpecial = determineSpecialType(count, MatchDirection::HORIZONTAL);
                    results.push_back(match);
                }
                
                // 重置计数
                count = 1;
                currentType = nextType;
            }
        }
    }
    
    return results;
}

std::vector<MatchResult> MatchDetector::detectVerticalMatches(
    const std::vector<std::vector<Fruit>>& map,
    bool matched[MAP_SIZE][MAP_SIZE]) {
    
    std::vector<MatchResult> results;
    
    for (int col = 0; col < MAP_SIZE; col++) {
        int count = 1;
        FruitType currentType = map[0][col].type;
        
        for (int row = 1; row <= MAP_SIZE; row++) {
            FruitType nextType = (row < MAP_SIZE) ? map[row][col].type : FruitType::EMPTY;
            
            // CANDY 类型不参与普通三消匹配
            if (row < MAP_SIZE && nextType == currentType && isMatchableFruit(currentType)) {
                count++;
            } else {
                // 检查是否形成匹配
                if (count >= 3 && isMatchableFruit(currentType)) {
                    MatchResult match;
                    match.fruitType = currentType;
                    match.direction = MatchDirection::VERTICAL;
                    match.matchCount = count;
                    match.specialPosition = {row - 1, col}; // 默认最后一个位置
                    
                    // 添加所有匹配位置
                    for (int i = 0; i < count; i++) {
                        int r = row - 1 - i;
                        match.positions.push_back({r, col});
                        matched[r][col] = true;
                    }
                    
                    match.generateSpecial = determineSpecialType(count, MatchDirection::VERTICAL);
                    results.push_back(match);
                }
                
                // 重置计数
                count = 1;
                currentType = nextType;
            }
        }
    }
    
    return results;
}

std::vector<MatchResult> MatchDetector::mergeIntersections(std::vector<MatchResult>& results) {
    // 如果结果数量小于2，无需合并
    if (results.size() < 2) {
        return results;
    }
    
    // 使用set记录已合并的索引
    std::set<int> merged;
    std::vector<MatchResult> finalResults;
    
    for (size_t i = 0; i < results.size(); i++) {
        if (merged.count(i)) {
            continue;
        }
        
        MatchResult current = results[i];
        bool foundIntersection = false;
        
        // 查找与当前匹配有交叉的其他匹配
        for (size_t j = i + 1; j < results.size(); j++) {
            if (merged.count(j)) {
                continue;
            }
            
            // 检查是否有交叉点
            bool hasIntersection = false;
            std::pair<int, int> intersectionPoint;
            
            for (const auto& pos1 : current.positions) {
                for (const auto& pos2 : results[j].positions) {
                    if (pos1 == pos2) {
                        hasIntersection = true;
                        intersectionPoint = pos1;
                        break;
                    }
                }
                if (hasIntersection) break;
            }
            
            if (hasIntersection) {
                // 合并两个匹配
                foundIntersection = true;
                
                // 合并位置列表（去重）
                std::set<std::pair<int, int>> uniquePositions(
                    current.positions.begin(), current.positions.end());
                uniquePositions.insert(results[j].positions.begin(), results[j].positions.end());
                
                current.positions.assign(uniquePositions.begin(), uniquePositions.end());
                current.matchCount = current.positions.size();
                current.specialPosition = intersectionPoint;
                
                // 判断形成的是L形还是T形
                if (current.direction == MatchDirection::HORIZONTAL && 
                    results[j].direction == MatchDirection::VERTICAL) {
                    current.direction = MatchDirection::L_SHAPE;
                    current.generateSpecial = SpecialType::DIAMOND; // L形或T形生成菱形炸弹
                } else if (current.direction == MatchDirection::VERTICAL && 
                           results[j].direction == MatchDirection::HORIZONTAL) {
                    current.direction = MatchDirection::T_SHAPE;
                    current.generateSpecial = SpecialType::DIAMOND;
                }
                
                merged.insert(j);
            }
        }
        
        finalResults.push_back(current);
        merged.insert(i);
    }
    
    return finalResults;
}

SpecialType MatchDetector::determineSpecialType(int count, MatchDirection direction) {
    // 根据消除数量和方向判断应生成的特殊元素类型
    if (count >= 5) {
        // 5个及以上直线消除 -> 万能炸弹（彩虹果）
        return SpecialType::RAINBOW;
    } else if (count == 4) {
        // 4个消除 -> 直线炸弹
        if (direction == MatchDirection::HORIZONTAL) {
            return SpecialType::LINE_H; // 横向直线炸弹
        } else if (direction == MatchDirection::VERTICAL) {
            return SpecialType::LINE_V; // 纵向直线炸弹
        }
    }
    // L形或T形在mergeIntersections中处理
    
    return SpecialType::NONE;
}

bool MatchDetector::wouldMatchAfterSwap(const std::vector<std::vector<Fruit>>& map,
                                        int row1, int col1, int row2, int col2) {
    // 创建临时地图副本
    auto tempMap = map;
    
    // 交换两个位置的水果
    std::swap(tempMap[row1][col1], tempMap[row2][col2]);
    
    // 更新位置坐标
    tempMap[row1][col1].row = row1;
    tempMap[row1][col1].col = col1;
    tempMap[row2][col2].row = row2;
    tempMap[row2][col2].col = col2;
    
    // 检查两个位置周围是否有匹配
    auto matches1 = detectMatchesAt(tempMap, row1, col1);
    auto matches2 = detectMatchesAt(tempMap, row2, col2);
    
    return !matches1.empty() || !matches2.empty();
}
