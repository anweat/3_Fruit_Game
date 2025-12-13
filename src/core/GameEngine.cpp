#include "GameEngine.h"
#include <algorithm>
#include <iostream>

GameEngine::GameEngine() 
    : state_(GameState::IDLE)
    , currentScore_(0)
    , totalMatches_(0)
    , swapHandler_(matchDetector_, specialProcessor_)
    , animRecorder_(fallProcessor_)
    , cycleProcessor_(matchDetector_, specialGenerator_, specialProcessor_,
                      animRecorder_, fruitGenerator_, scoreCalculator_)
{
    // 构造函数 - 初始化成员变量和模块
    lastAnimation_ = GameAnimationSequence{}; // 清零动画记录
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
    
    // 4. 清空最近动画记录
    lastAnimation_ = GameAnimationSequence{};
}

/**
 * @brief 尝试交换两个水果
 */
bool GameEngine::swapFruits(int row1, int col1, int row2, int col2) {
    // 清空动画记录
    lastAnimation_ = GameAnimationSequence{};
    
    // 1. 使用 SwapHandler 执行交换
    std::vector<GameRound> swapRounds;
    bool success = swapHandler_.executeSwap(map_, row1, col1, row2, col2,
                                             lastAnimation_.swap, swapRounds);
    
    if (!success) {
        // 交换失败，直接返回
        return false;
    }
    
    // 2. 如果交换产生了消除轮次（CANDY/炸弹组合），添加到 rounds
    for (const auto& round : swapRounds) {
        lastAnimation_.rounds.push_back(round);
        // 记录下落
        animRecorder_.recordFallAndRefill(map_, fruitGenerator_, 
                                           lastAnimation_.rounds.back().fall);
    }
    
    // 3. 如果是普通交换成功，处理游戏循环
    if (swapRounds.empty()) {
        state_ = GameState::SWAPPING;
        processGameCycle();
    }
    
    return true;
}

/**
 * @brief 处理一轮游戏循环
 */
bool GameEngine::processGameCycle() {
    // 使用 GameCycleProcessor 处理循环
    std::vector<GameRound> cycleRounds;
    int totalScore = 0;
    
    bool hadElimination = cycleProcessor_.processMatchCycle(map_, cycleRounds, totalScore);
    
    // 追加循环产生的轮次
    for (const auto& round : cycleRounds) {
        lastAnimation_.rounds.push_back(round);
    }
    
    // 更新分数
    currentScore_ += totalScore;
    lastAnimation_.totalScoreDelta += totalScore;
    
    // 检查死局
    if (!hadElimination) {
        bool shuffled = false;
        std::vector<std::vector<Fruit>> newMap;
        cycleProcessor_.handleDeadlock(map_, shuffled, newMap);
        
        if (shuffled) {
            lastAnimation_.shuffled = true;
            lastAnimation_.newMapAfterShuffle = newMap;
        }
    }
    
    state_ = GameState::IDLE;
    return hadElimination;
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
    return const_cast<MatchDetector&>(matchDetector_).hasPossibleMoves(map_);
}

/**
 * @brief 重置游戏
 */
void GameEngine::resetGame() {
    initializeGame();
}

/**
 * @brief 使用道具
 */
bool GameEngine::useProp(ClickMode mode, int row, int col) {
    if (state_ != GameState::IDLE) {
        return false;
    }
    
    std::set<std::pair<int, int>> affectedPositions;
    bool success = false;
    
    // 根据模式调用对应道具
    switch (mode) {
        case ClickMode::PROP_HAMMER:
            success = propManager_.useHammer(map_, row, col, affectedPositions);
            break;
        case ClickMode::PROP_MAGIC_WAND:
            success = propManager_.useMagicWand(map_, row, col, affectedPositions);
            break;
        case ClickMode::PROP_CLAMP:
            // 夹子需要两个位置，暂不支持单点击模式
            return false;
        default:
            return false;
    }
    
    if (!success) {
        return false;
    }
    
    // 初始化动画序列
    lastAnimation_ = GameAnimationSequence();
    lastAnimation_.swap.success = false;  // 道具模式不是交换
    
    // 处理道具的直接消除效果（第0轮）
    GameRound round0;
    int score0 = 0;
    cycleProcessor_.processPropElimination(map_, affectedPositions, round0, score0);
    lastAnimation_.rounds.push_back(round0);
    lastAnimation_.totalScoreDelta += score0;
    currentScore_ += score0;
    
    // 然后启动完整的游戏循环（处理下落后的连锁消除）
    state_ = GameState::SWAPPING;
    std::vector<GameRound> cycleRounds;
    int cycleScore = 0;
    
    bool hadMoreElimination = cycleProcessor_.processMatchCycle(map_, cycleRounds, cycleScore);
    
    // 追加循环产生的轮次
    for (const auto& round : cycleRounds) {
        lastAnimation_.rounds.push_back(round);
    }
    
    // 更新总分
    currentScore_ += cycleScore;
    lastAnimation_.totalScoreDelta += cycleScore;
    
    // 检查是否有死局
    if (!hadMoreElimination) {
        bool shuffled = false;
        std::vector<std::vector<Fruit>> newMap;
        cycleProcessor_.handleDeadlock(map_, shuffled, newMap);
        
        if (shuffled) {
            lastAnimation_.shuffled = true;
            lastAnimation_.newMapAfterShuffle = newMap;
        }
    }
    
    state_ = GameState::IDLE;
    return true;
}

/**
 * @brief 使用夹子道具（强制交换两个相邻元素）
 */
bool GameEngine::useClampProp(int row1, int col1, int row2, int col2) {
    if (state_ != GameState::IDLE) {
        return false;
    }
    
    // 验证夹子是否可用
    if (!propManager_.useClamp(map_, row1, col1, row2, col2)) {
        return false;
    }
    
    // 初始化动画序列
    lastAnimation_ = GameAnimationSequence();
    lastAnimation_.swap.row1 = row1;
    lastAnimation_.swap.col1 = col1;
    lastAnimation_.swap.row2 = row2;
    lastAnimation_.swap.col2 = col2;
    lastAnimation_.swap.success = true;  // 夹子强制交换总是成功
    
    // 执行纯粹的交换（不检测匹配）
    std::swap(map_[row1][col1], map_[row2][col2]);
    
    // 然后启动完整的游戏循环（处理交换后的匹配和连锁）
    state_ = GameState::SWAPPING;
    std::vector<GameRound> cycleRounds;
    int cycleScore = 0;
    
    bool hadElimination = cycleProcessor_.processMatchCycle(map_, cycleRounds, cycleScore);
    
    // 追加循环产生的轮次
    for (const auto& round : cycleRounds) {
        lastAnimation_.rounds.push_back(round);
    }
    
    // 更新总分
    currentScore_ += cycleScore;
    lastAnimation_.totalScoreDelta += cycleScore;
    
    // 检查是否有死局
    if (!hadElimination) {
        bool shuffled = false;
        std::vector<std::vector<Fruit>> newMap;
        cycleProcessor_.handleDeadlock(map_, shuffled, newMap);
        
        if (shuffled) {
            lastAnimation_.shuffled = true;
            lastAnimation_.newMapAfterShuffle = newMap;
        }
    }
    
    state_ = GameState::IDLE;
    return true;
}
