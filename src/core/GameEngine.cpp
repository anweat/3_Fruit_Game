#include "GameEngine.h"
#include <algorithm>

GameEngine::GameEngine() 
    : state_(GameState::IDLE)
    , currentScore_(0)
    , totalMatches_(0) {
    // 构造函数 - 初始化成员变量
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
}

/**
 * @brief 尝试交换两个水果
 */
bool GameEngine::swapFruits(int row1, int col1, int row2, int col2) {
    // 1. 验证交换是否合法（相邻）
    if (!isValidSwap(row1, col1, row2, col2)) {
        return false;
    }
    
    // 2. 检查是否有特殊元素参与交换
    bool hasSpecial1 = map_[row1][col1].special != SpecialType::NONE;
    bool hasSpecial2 = map_[row2][col2].special != SpecialType::NONE;
    
    // 3. 如果两个都是特殊元素，触发组合效果
    if (hasSpecial1 && hasSpecial2) {
        std::set<std::pair<int, int>> affectedPositions;
        specialProcessor_.triggerCombinationEffect(map_, row1, col1, row2, col2, affectedPositions);
        
        // 消除受影响的位置
        for (const auto& pos : affectedPositions) {
            map_[pos.first][pos.second].type = FruitType::EMPTY;
            map_[pos.first][pos.second].special = SpecialType::NONE;
        }
        
        // 处理后续（下落、填充、再匹配）
        processGameCycle();
        return true;
    }
    
    // 4. 执行交换
    std::swap(map_[row1][col1], map_[row2][col2]);
    // 更新坐标
    std::swap(map_[row1][col1].row, map_[row2][col2].row);
    std::swap(map_[row1][col1].col, map_[row2][col2].col);
    
    // 5. 检测交换后是否有匹配
    // 直接检测交换位置的匹配
    auto matches1 = matchDetector_.detectMatchesAt(map_, row1, col1);
    auto matches2 = matchDetector_.detectMatchesAt(map_, row2, col2);
    bool hasMatch = !matches1.empty() || !matches2.empty();
    
    if (!hasMatch) {
        // 如果没有匹配，撤销交换
        std::swap(map_[row1][col1], map_[row2][col2]);
        std::swap(map_[row1][col1].row, map_[row2][col2].row);
        std::swap(map_[row1][col1].col, map_[row2][col2].col);
        return false;
    }
    
    // 6. 有匹配，处理游戏循环
    state_ = GameState::SWAPPING;
    processGameCycle();
    
    return true;
}

/**
 * @brief 处理一轮游戏循环
 */
bool GameEngine::processGameCycle() {
    bool hadElimination = false;
    
    // 循环处理：匹配 → 消除 → 下落 → 再匹配
    while (true) {
        // 1. 检测并处理匹配
        state_ = GameState::MATCHING;
        int matchCount = detectAndProcessMatches();
        
        if (matchCount == 0) {
            // 没有匹配，结束循环
            break;
        }
        
        hadElimination = true;
        
        // 2. 处理下落和填充
        state_ = GameState::FALLING;
        processFallAndRefill();
        
        // 3. 增加连击
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
 * @brief 检测并处理所有匹配
 */
int GameEngine::detectAndProcessMatches() {
    // 1. 检测匹配
    auto matches = matchDetector_.detectMatches(map_);
    
    if (matches.empty()) {
        return 0;
    }
    
    // 2. 处理特殊元素生成（在消除之前）
    processSpecialGeneration(matches);
    
    // 3. 计算得分
    int comboMultiplier = std::max(1, scoreCalculator_.getComboCount());
    int score = scoreCalculator_.calculateTotalScore(matches, comboMultiplier);
    currentScore_ += score;
    
    // 4. 消除匹配的水果
    eliminateMatches(matches);
    
    // 5. 统计
    totalMatches_ += matches.size();
    
    return matches.size();
}

/**
 * @brief 处理特殊元素生成
 */
void GameEngine::processSpecialGeneration(const std::vector<MatchResult>& matches) {
    for (const auto& match : matches) {
        // 判断是否应该生成特殊元素
        SpecialType specialType = specialGenerator_.determineSpecialType(match);
        
        if (specialType != SpecialType::NONE) {
            // 在地图上生成特殊元素
            specialGenerator_.generateSpecialFruit(map_, match, specialType);
        }
    }
}

/**
 * @brief 消除匹配的水果
 */
void GameEngine::eliminateMatches(const std::vector<MatchResult>& matches) {
    state_ = GameState::ELIMINATING;
    
    // 标记所有需要消除的位置
    for (const auto& match : matches) {
        for (const auto& pos : match.positions) {
            // 检查是否是刚生成的特殊元素（不消除）
            if (map_[pos.first][pos.second].special != SpecialType::NONE && 
                !map_[pos.first][pos.second].isMatched) {
                // 跳过特殊元素（它们会保留在地图上）
                continue;
            }
            
            // 标记为已匹配
            map_[pos.first][pos.second].isMatched = true;
        }
    }
    
    // 处理特殊元素效果（如果有的话）
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map_[row][col].isMatched && map_[row][col].special != SpecialType::NONE) {
                std::set<std::pair<int, int>> affectedPositions;
                specialProcessor_.triggerSpecialEffect(map_, row, col, affectedPositions);
                
                // 标记受影响的位置
                for (const auto& pos : affectedPositions) {
                    map_[pos.first][pos.second].isMatched = true;
                }
            }
        }
    }
    
    // 执行消除
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

/**
 * @brief 处理下落和填充空位
 */
bool GameEngine::processFallAndRefill() {
    // 1. 处理下落
    bool hasFall = fallProcessor_.hasEmptySlots(map_);
    
    if (hasFall) {
        // 处理每一列的下落
        for (int col = 0; col < MAP_SIZE; col++) {
            fallProcessor_.processColumnFall(map_, col);
        }
    }
    
    // 2. 填充空位
    fallProcessor_.fillEmptySlots(map_, fruitGenerator_);
    
    return hasFall;
}

/**
 * @brief 检查并处理死局
 */
void GameEngine::checkAndHandleDeadlock() {
    if (!matchDetector_.hasPossibleMoves(map_)) {
        // 没有可移动，重排地图
        fruitGenerator_.shuffleMap(map_, matchDetector_);
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
