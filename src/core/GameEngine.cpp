#include "GameEngine.h"
#include "AchievementManager.h"
#include "Database.h"
#include <algorithm>
#include <iostream>

GameEngine::GameEngine() 
    : state_(GameState::IDLE)
    , currentScore_(0)
    , totalMatches_(0)
    , mapSize_(MAP_SIZE)  // 默认使用全局常量
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
void GameEngine::initializeGame(int initialScore, int mapSize) {
    // 0. 设置地图大小
    mapSize_ = mapSize;
    
    // 1. 初始化地图（确保无三连且有可移动）
    fruitGenerator_.initializeMap(map_, mapSize_);
    
    // 2. 确保地图可玩
    fruitGenerator_.ensurePlayable(map_, matchDetector_, mapSize_);
    
    // 3. 重置游戏状态
    state_ = GameState::IDLE;
    currentScore_ = initialScore;  // 📌 修复问题 #3: 支持初始分数（休闲模式）
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
    
    // 统计：移动次数+1
    sessionStats_.totalMoves++;
    
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
    
    // 追加循环产生的轮次并统计消除数据
    for (const auto& round : cycleRounds) {
        lastAnimation_.rounds.push_back(round);
        
        // 统计消除数据
        if (round.elimination.positions.size() > 0) {
            sessionStats_.totalEliminates++;
            
            // 统计消除的水果类型
            for (size_t i = 0; i < round.elimination.types.size(); ++i) {
                int typeVal = static_cast<int>(round.elimination.types[i]);
                if (typeVal > 0) {
                    sessionStats_.eliminatedFruitTypes.insert(typeVal);
                }
            }
            
            // 📌 核心修复：遍历每个独立的匹配组，为每个4+消发送成就快照
            for (const auto& matchGroup : round.elimination.matchGroups) {
                int matchSize = matchGroup.count;
                
                // 统计单局消除次数
                if (matchSize == 4) sessionStats_.match4Count++;
                if (matchSize == 5) sessionStats_.match5Count++;
                if (matchSize >= 6) sessionStats_.match6Count++;
                
                // 为每个匹配组发送成就快照
                GameDataSnapshot snapshot;
                snapshot.currentScore = currentScore_;
                snapshot.lastMatchSize = matchSize;
                snapshot.lastMatchElementType = static_cast<int>(matchGroup.type);
                snapshot.lastMatchSameElement = true;  // 每个匹配组内部必然是同类型
                snapshot.currentCombo = scoreCalculator_.getComboCount();
                snapshot.gameMode = sessionStats_.gameMode;
                snapshot.gameStartTime = sessionStats_.startTime;
                
                AchievementManager::instance().recordGameSnapshot(snapshot);
            }
        }
    }
    
    // 更新分数
    currentScore_ += totalScore;
    lastAnimation_.totalScoreDelta += totalScore;
    
    // 更新最大连击
    sessionStats_.maxCombo = std::max(sessionStats_.maxCombo, 
                                       scoreCalculator_.getComboCount());
    
    // 检查死局
    if (!hadElimination) {
        bool shuffled = false;
        std::vector<std::vector<Fruit>> newMap;
        cycleProcessor_.handleDeadlock(map_, shuffled, newMap, mapSize_);
        
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
        cycleProcessor_.handleDeadlock(map_, shuffled, newMap, mapSize_);
        
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
        cycleProcessor_.handleDeadlock(map_, shuffled, newMap, mapSize_);
        
        if (shuffled) {
            lastAnimation_.shuffled = true;
            lastAnimation_.newMapAfterShuffle = newMap;
        }
    }
    
    state_ = GameState::IDLE;
    return true;
}

// ==================== 成就系统集成 ====================

/**
 * @brief 开始游戏会话
 */
void GameEngine::startGameSession(const QString& mode)
{
    // 重置统计数据
    sessionStats_ = GameSessionStats();
    sessionStats_.gameMode = mode;
    sessionStats_.startTime = QDateTime::currentMSecsSinceEpoch();
    
    // 通知成就系统（使用统一接口）
    AchievementManager::instance().recordGameSession(mode, true);
}

/**
 * @brief 结束游戏会话
 */
void GameEngine::endGameSession()
{
    // 构建完整快照
    GameDataSnapshot snapshot;
    snapshot.currentScore = currentScore_;
    snapshot.maxCombo = sessionStats_.maxCombo;
    snapshot.gameMode = sessionStats_.gameMode;
    snapshot.gameStartTime = sessionStats_.startTime;
    snapshot.moveCount = sessionStats_.totalMoves;
    snapshot.eliminateCount = sessionStats_.totalEliminates;
    snapshot.match4Count = sessionStats_.match4Count;
    snapshot.match5Count = sessionStats_.match5Count;
    snapshot.match6Count = sessionStats_.match6Count;
    snapshot.fruitTypesEliminated = sessionStats_.eliminatedFruitTypes;
    snapshot.propUsed = (sessionStats_.propUsed > 0);
    
    // 保存数据到数据库（仅非游客模式）
    QString playerId = Database::instance().getCurrentPlayerId();
    if (sessionStats_.gameMode == "Casual" && playerId != "guest") {
        Database::instance().savePlayerScore(playerId, currentScore_);
        Database::instance().savePlayerProps(
            playerId,
            propManager_.getPropCount(PropType::HAMMER),
            propManager_.getPropCount(PropType::CLAMP),
            propManager_.getPropCount(PropType::MAGIC_WAND)
        );
    }
    
    // 通知成就系统结束会话
    AchievementManager::instance().recordGameSession(sessionStats_.gameMode, false);
}
