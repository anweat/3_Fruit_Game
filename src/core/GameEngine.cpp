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
    bool isFirstMatch = true;  // 标记是否是第一次匹配（只有第一次才生成特殊元素）
    
    // 循环处理：匹配 → 消除 → 下落 → 再匹配
    while (true) {
        // 1. 检测匹配
        state_ = GameState::MATCHING;
        auto matches = matchDetector_.detectMatches(map_);
        
        if (matches.empty()) {
            break;  // 没有匹配，结束循环
        }
        
        hadElimination = true;
        
        // 2. 如果是第一次匹配，生成特殊元素（在消除前）
        std::set<std::pair<int, int>> specialPositions;  // 记录生成特殊元素的位置
        if (isFirstMatch) {
            processSpecialGeneration(matches, specialPositions);
            isFirstMatch = false;
        }
        
        // 3. 计算得分
        int comboMultiplier = std::max(1, scoreCalculator_.getComboCount());
        int score = scoreCalculator_.calculateTotalScore(matches, comboMultiplier);
        currentScore_ += score;
        
        // 4. 消除匹配的水果（但跳过刚生成的特殊元素）
        eliminateMatches(matches, specialPositions);
        
        // 5. 统计
        totalMatches_ += matches.size();
        
        // 6. 处理下落和填充
        state_ = GameState::FALLING;
        processFallAndRefill();
        
        // 7. 增加连击
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
 * @brief 处理特殊元素生成
 * @param specialPositions 输出参数，记录生成特殊元素的位置
 */
void GameEngine::processSpecialGeneration(
    const std::vector<MatchResult>& matches,
    std::set<std::pair<int, int>>& specialPositions) {
    
    for (const auto& match : matches) {
        // 判断是否应该生成特殊元素
        SpecialType specialType = specialGenerator_.determineSpecialType(match);
        
        if (specialType != SpecialType::NONE) {
            // 在地图上生成特殊元素，并记录位置
            auto pos = specialGenerator_.generateSpecialFruit(map_, match, specialType);
            specialPositions.insert(pos);  // 记录特殊元素位置
        }
    }
}

/**
 * @brief 消除匹配的水果
 * @param specialPositions 需要保留的特殊元素位置（刚生成的）
 */
void GameEngine::eliminateMatches(
    const std::vector<MatchResult>& matches,
    const std::set<std::pair<int, int>>& specialPositions) {
    
    state_ = GameState::ELIMINATING;
    
    // 第一步：标记所有需要消除的位置（普通三消）
    for (const auto& match : matches) {
        for (const auto& pos : match.positions) {
            // 如果该位置是刚生成的特殊元素，跳过不消除
            if (specialPositions.find(pos) != specialPositions.end()) {
                continue;  // 保留刚生成的特殊元素
            }
            map_[pos.first][pos.second].isMatched = true;
        }
    }
    
    // 第二步：处理特殊元素效果（被消除的特殊元素会触发效果）
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map_[row][col].isMatched && map_[row][col].special != SpecialType::NONE) {
                std::set<std::pair<int, int>> affectedPositions;
                specialProcessor_.triggerSpecialEffect(map_, row, col, affectedPositions);
                
                // 标记受影响的位置为消除
                for (const auto& pos : affectedPositions) {
                    // 如果是刚生成的特殊元素，也不消除
                    if (specialPositions.find(pos) == specialPositions.end()) {
                        map_[pos.first][pos.second].isMatched = true;
                    }
                }
            }
        }
    }
    
    // 第三步：执行消除（清空所有被标记的位置）
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
