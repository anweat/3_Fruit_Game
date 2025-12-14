#include "AchievementManager.h"
#include "../core/GameEngine.h"
#include "../data/Database.h"
#include "detectors/AchievementDetectorManager.h"
#include "detectors/DetectorFactory.h"
#include <QDebug>
#include <QDateTime>
#include <QSet>

// ==================== AchievementManager 实现 ====================

AchievementManager& AchievementManager::instance()
{
    static AchievementManager instance;
    return instance;
}

AchievementManager::AchievementManager()
    : workerThread_(nullptr)
    , worker_(nullptr)
{
}

AchievementManager::~AchievementManager()
{
    shutdown();
}

/**
 * @brief 初始化成就系统
 */
void AchievementManager::initialize()
{
    // 1. 加载所有成就定义
    loadAchievementDefinitions();
    
    // 2. 创建工作线程
    workerThread_ = new QThread();
    worker_ = new AchievementWorker(&achievements_);
    worker_->moveToThread(workerThread_);
    
    // 3. 连接信号
    connect(this, &AchievementManager::gameDataReceived,
            worker_, &AchievementWorker::onGameDataReceived);
    connect(this, &AchievementManager::gameStarted,
            worker_, &AchievementWorker::onGameStarted);
    connect(this, &AchievementManager::gameEnded,
            worker_, &AchievementWorker::onGameEnded);
    connect(worker_, &AchievementWorker::achievementUnlocked,
            this, &AchievementManager::handleAchievementUnlocked);
    
    // 4. 启动线程
    workerThread_->start();
}

/**
 * @brief 关闭成就系统
 */
void AchievementManager::shutdown()
{
    if (workerThread_) {
        workerThread_->quit();
        workerThread_->wait();
        delete worker_;
        worker_ = nullptr;
        delete workerThread_;
        workerThread_ = nullptr;
    }
}

/**
 * @brief 设置游戏引擎实例（用于添加成就奖励分数）
 */
void AchievementManager::setGameEngine(GameEngine* engine)
{
    gameEngine_ = engine;
    if (worker_) {
        worker_->setGameEngine(engine);
    }
}

/**
 * @brief 设置当前玩家ID（切换账号时调用）
 */
void AchievementManager::setCurrentPlayerId(const QString& playerId)
{
    if (worker_) {
        worker_->setCurrentPlayerId(playerId);
    }
}

/**
 * @brief 加载所有62个成就定义
 */
void AchievementManager::loadAchievementDefinitions()
{
    // === 新手入门系列 (6个) ===
    achievements_["ach_first_match"] = {"ach_first_match", "🌟 初来乍到", "完成首次三消", 
        AchievementCategory::BEGINNER, AchievementRarity::BRONZE, 5, 1, "first_match"};
    achievements_["ach_first_game"] = {"ach_first_game", "🌟 游戏启程", "完成首局游戏", 
        AchievementCategory::BEGINNER, AchievementRarity::BRONZE, 10, 1, "first_game"};
    achievements_["ach_score_100"] = {"ach_score_100", "🌟 百分新手", "首次得分超过100分", 
        AchievementCategory::BEGINNER, AchievementRarity::BRONZE, 5, 100, "score_threshold"};
    achievements_["ach_5_games"] = {"ach_5_games", "🌟 熟练玩家", "累计完成5局游戏", 
        AchievementCategory::BEGINNER, AchievementRarity::BRONZE, 20, 5, "total_games"};
    achievements_["ach_first_special"] = {"ach_first_special", "🌟 特殊发现", "首次生成特殊元素", 
        AchievementCategory::BEGINNER, AchievementRarity::BRONZE, 15, 1, "first_special"};
    achievements_["ach_tutorial"] = {"ach_tutorial", "🌟 入门毕业", "完成游戏教学", 
        AchievementCategory::BEGINNER, AchievementRarity::BRONZE, 10, 1, "tutorial_complete"};

    // === 连击系列 (8个) ===
    achievements_["ach_combo_3"] = {"ach_combo_3", "⚡ 连击新手", "单局达成3连击", 
        AchievementCategory::COMBO, AchievementRarity::BRONZE, 15, 3, "combo_single"};
    achievements_["ach_combo_5"] = {"ach_combo_5", "⚡ 连击学徒", "单局达成5连击", 
        AchievementCategory::COMBO, AchievementRarity::SILVER, 30, 5, "combo_single"};
    achievements_["ach_combo_8"] = {"ach_combo_8", "⚡ 连击达人", "单局达成8连击", 
        AchievementCategory::COMBO, AchievementRarity::GOLD, 60, 8, "combo_single"};
    achievements_["ach_combo_12"] = {"ach_combo_12", "⚡ 连击大师", "单局达成12连击", 
        AchievementCategory::COMBO, AchievementRarity::DIAMOND, 120, 12, "combo_single"};
    achievements_["ach_combo_15"] = {"ach_combo_15", "⚡ 连击传说", "单局达成15连击", 
        AchievementCategory::COMBO, AchievementRarity::DIAMOND, 200, 15, "combo_single"};
    achievements_["ach_combo_100"] = {"ach_combo_100", "⚡ 连击风暴", "累计达成100次连击(3+)", 
        AchievementCategory::COMBO, AchievementRarity::GOLD, 80, 100, "combo_total"};
    achievements_["ach_combo_500"] = {"ach_combo_500", "⚡ 连击狂人", "累计达成500次连击(3+)", 
        AchievementCategory::COMBO, AchievementRarity::DIAMOND, 200, 500, "combo_total"};
    achievements_["ach_combo_streak"] = {"ach_combo_streak", "⚡ 不间断", "连续10次操作都触发连击", 
        AchievementCategory::COMBO, AchievementRarity::DIAMOND, 150, 10, "combo_streak"};

    // === 多消系列 (10个) ===
    achievements_["ach_match4_first"] = {"ach_match4_first", "🎯 四消入门", "首次完成4消", 
        AchievementCategory::MULTI_MATCH, AchievementRarity::BRONZE, 15, 1, "match4_first"};
    achievements_["ach_match5_first"] = {"ach_match5_first", "🎯 五消解锁", "首次完成5消", 
        AchievementCategory::MULTI_MATCH, AchievementRarity::SILVER, 25, 1, "match5_first"};
    achievements_["ach_match6"] = {"ach_match6", "🎯 六消惊喜", "单次消除6个水果", 
        AchievementCategory::MULTI_MATCH, AchievementRarity::GOLD, 50, 6, "match_size"};
    achievements_["ach_match8"] = {"ach_match8", "🎯 完美一行", "单次消除整行/整列(8个)", 
        AchievementCategory::MULTI_MATCH, AchievementRarity::DIAMOND, 100, 8, "match_size"};
    achievements_["ach_match4_100"] = {"ach_match4_100", "🎯 四消收集家", "累计完成100次4消", 
        AchievementCategory::MULTI_MATCH, AchievementRarity::SILVER, 60, 100, "match4_total"};
    achievements_["ach_match5_50"] = {"ach_match5_50", "🎯 五消收集家", "累计完成50次5消", 
        AchievementCategory::MULTI_MATCH, AchievementRarity::GOLD, 100, 50, "match5_total"};
    achievements_["ach_match6_20"] = {"ach_match6_20", "🎯 六消大师", "累计完成20次6消", 
        AchievementCategory::MULTI_MATCH, AchievementRarity::GOLD, 150, 20, "match6_total"};
    achievements_["ach_match8_10"] = {"ach_match8_10", "🎯 完美主义者", "累计完成10次整行/整列消除", 
        AchievementCategory::MULTI_MATCH, AchievementRarity::DIAMOND, 200, 10, "match8_total"};
    achievements_["ach_match5plus_3"] = {"ach_match5plus_3", "🎯 消除艺术家", "单局完成3次5+消除", 
        AchievementCategory::MULTI_MATCH, AchievementRarity::GOLD, 120, 3, "match5plus_game"};
    achievements_["ach_match6plus_5"] = {"ach_match6plus_5", "🎯 多消狂人", "单局完成5次6+消除", 
        AchievementCategory::MULTI_MATCH, AchievementRarity::DIAMOND, 250, 5, "match6plus_game"};

    // === 特殊元素系列 (10个) ===
    achievements_["ach_special_first"] = {"ach_special_first", "💣 炸弹制造者", "首次生成特殊元素", 
        AchievementCategory::SPECIAL, AchievementRarity::BRONZE, 20, 1, "special_first"};
    achievements_["ach_line_50"] = {"ach_line_50", "💣 直线专家", "累计生成50个直线炸弹", 
        AchievementCategory::SPECIAL, AchievementRarity::SILVER, 60, 50, "line_total"};
    achievements_["ach_diamond_30"] = {"ach_diamond_30", "💣 菱形专家", "累计生成30个菱形炸弹", 
        AchievementCategory::SPECIAL, AchievementRarity::GOLD, 80, 30, "diamond_total"};
    achievements_["ach_rainbow_20"] = {"ach_rainbow_20", "💣 彩虹收集者", "累计生成20个万能炸弹", 
        AchievementCategory::SPECIAL, AchievementRarity::GOLD, 100, 20, "rainbow_total"};
    achievements_["ach_special_200"] = {"ach_special_200", "💣 炸弹狂人", "累计使用特殊元素200次", 
        AchievementCategory::SPECIAL, AchievementRarity::DIAMOND, 150, 200, "special_use_total"};
    achievements_["ach_combo_line_line"] = {"ach_combo_line_line", "💣 十字轰炸", "首次触发直线+直线组合", 
        AchievementCategory::SPECIAL, AchievementRarity::SILVER, 50, 1, "combo_line_line"};
    achievements_["ach_combo_line_diamond"] = {"ach_combo_line_diamond", "💣 范围打击", "首次触发直线+菱形组合", 
        AchievementCategory::SPECIAL, AchievementRarity::GOLD, 70, 1, "combo_line_diamond"};
    achievements_["ach_combo_diamond_diamond"] = {"ach_combo_diamond_diamond", "💣 超级范围", "首次触发菱形+菱形组合", 
        AchievementCategory::SPECIAL, AchievementRarity::GOLD, 100, 1, "combo_diamond_diamond"};
    achievements_["ach_combo_any_rainbow"] = {"ach_combo_any_rainbow", "💣 彩虹风暴", "首次触发任意+万能组合", 
        AchievementCategory::SPECIAL, AchievementRarity::DIAMOND, 120, 1, "combo_any_rainbow"};
    achievements_["ach_combo_rainbow_rainbow"] = {"ach_combo_rainbow_rainbow", "💣 全屏清空", "首次触发万能+万能组合", 
        AchievementCategory::SPECIAL, AchievementRarity::DIAMOND, 200, 1, "combo_rainbow_rainbow"};

    // === 得分系列 (8个) ===
    achievements_["ach_score_1k"] = {"ach_score_1k", "🏆 千分突破", "单局得分超过1000", 
        AchievementCategory::SCORE, AchievementRarity::BRONZE, 10, 1000, "score_single"};
    achievements_["ach_score_5k"] = {"ach_score_5k", "🏆 五千里程碑", "单局得分超过5000", 
        AchievementCategory::SCORE, AchievementRarity::SILVER, 30, 5000, "score_single"};
    achievements_["ach_score_10k"] = {"ach_score_10k", "🏆 万分俱乐部", "单局得分超过10000", 
        AchievementCategory::SCORE, AchievementRarity::SILVER, 60, 10000, "score_single"};
    achievements_["ach_score_20k"] = {"ach_score_20k", "🏆 两万高手", "单局得分超过20000", 
        AchievementCategory::SCORE, AchievementRarity::GOLD, 100, 20000, "score_single"};
    achievements_["ach_score_30k"] = {"ach_score_30k", "🏆 三万精英", "单局得分超过30000", 
        AchievementCategory::SCORE, AchievementRarity::GOLD, 150, 30000, "score_single"};
    achievements_["ach_score_50k"] = {"ach_score_50k", "🏆 五万王者", "单局得分超过50000", 
        AchievementCategory::SCORE, AchievementRarity::DIAMOND, 250, 50000, "score_single"};
    achievements_["ach_score_100k"] = {"ach_score_100k", "🏆 十万传奇", "单局得分超过100000", 
        AchievementCategory::SCORE, AchievementRarity::DIAMOND, 500, 100000, "score_single"};
    achievements_["ach_score_burst"] = {"ach_score_burst", "🏆 单次爆发", "单次消除得分超过500", 
        AchievementCategory::SCORE, AchievementRarity::GOLD, 80, 500, "score_eliminate"};

    // === 道具使用系列 (6个) ===
    achievements_["ach_prop_hammer_first"] = {"ach_prop_hammer_first", "🔧 锤子新手", "首次使用锤子道具", 
        AchievementCategory::PROP, AchievementRarity::BRONZE, 10, 1, "prop_hammer_first"};
    achievements_["ach_prop_clamp_first"] = {"ach_prop_clamp_first", "🔧 夹子新手", "首次使用夹子道具", 
        AchievementCategory::PROP, AchievementRarity::BRONZE, 10, 1, "prop_clamp_first"};
    achievements_["ach_prop_wand_first"] = {"ach_prop_wand_first", "🔧 魔法新手", "首次使用魔法棒道具", 
        AchievementCategory::PROP, AchievementRarity::BRONZE, 10, 1, "prop_wand_first"};
    achievements_["ach_prop_50"] = {"ach_prop_50", "🔧 道具达人", "累计使用道具50次", 
        AchievementCategory::PROP, AchievementRarity::SILVER, 60, 50, "prop_total"};
    achievements_["ach_prop_200"] = {"ach_prop_200", "🔧 道具大师", "累计使用道具200次", 
        AchievementCategory::PROP, AchievementRarity::GOLD, 150, 200, "prop_total"};
    achievements_["ach_prop_chain"] = {"ach_prop_chain", "🔧 精准打击", "用锤子敲掉特殊元素触发连锁消除10+个", 
        AchievementCategory::PROP, AchievementRarity::DIAMOND, 100, 10, "prop_chain"};

    // === 特殊挑战系列 (8个) ===
    achievements_["ach_challenge_60s"] = {"ach_challenge_60s", "🎮 速战速决", "60秒内得分超过10000", 
        AchievementCategory::CHALLENGE, AchievementRarity::GOLD, 80, 10000, "challenge_60s"};
    achievements_["ach_challenge_flash"] = {"ach_challenge_flash", "🎮 闪电手", "30秒内完成20次消除", 
        AchievementCategory::CHALLENGE, AchievementRarity::GOLD, 100, 20, "challenge_flash"};
    achievements_["ach_challenge_noprop"] = {"ach_challenge_noprop", "🎮 无道具挑战", "单局不使用道具得分超过20000", 
        AchievementCategory::CHALLENGE, AchievementRarity::DIAMOND, 150, 20000, "challenge_noprop"};
    achievements_["ach_challenge_perfect_start"] = {"ach_challenge_perfect_start", "🎮 完美开局", "前3次操作全部触发4+消除", 
        AchievementCategory::CHALLENGE, AchievementRarity::DIAMOND, 120, 3, "challenge_perfect_start"};
    achievements_["ach_challenge_chain5"] = {"ach_challenge_chain5", "🎮 连锁反应", "单次操作触发5次连锁消除", 
        AchievementCategory::CHALLENGE, AchievementRarity::DIAMOND, 180, 5, "challenge_chain"};
    achievements_["ach_challenge_30fruits"] = {"ach_challenge_30fruits", "🎮 地图清理", "单次操作消除超过30个水果", 
        AchievementCategory::CHALLENGE, AchievementRarity::DIAMOND, 200, 30, "challenge_eliminate"};
    achievements_["ach_challenge_lucky"] = {"ach_challenge_lucky", "🎮 幸运儿", "刷新后立即形成5+消除", 
        AchievementCategory::CHALLENGE, AchievementRarity::GOLD, 100, 1, "challenge_lucky"};
    achievements_["ach_challenge_marathon"] = {"ach_challenge_marathon", "🎮 持久战", "单局游戏持续超过10分钟", 
        AchievementCategory::CHALLENGE, AchievementRarity::SILVER, 80, 600, "challenge_marathon"};

    // === 收集与里程碑系列 (6个) ===
    achievements_["ach_milestone_10games"] = {"ach_milestone_10games", "📊 游戏次数10", "累计游玩10局", 
        AchievementCategory::MILESTONE, AchievementRarity::BRONZE, 30, 10, "total_games"};
    achievements_["ach_milestone_50games"] = {"ach_milestone_50games", "📊 游戏次数50", "累计游玩50局", 
        AchievementCategory::MILESTONE, AchievementRarity::SILVER, 80, 50, "total_games"};
    achievements_["ach_milestone_100games"] = {"ach_milestone_100games", "📊 游戏次数100", "累计游玩100局", 
        AchievementCategory::MILESTONE, AchievementRarity::GOLD, 150, 100, "total_games"};
    achievements_["ach_milestone_5000points"] = {"ach_milestone_5000points", "📊 点数富翁", "累计获得5000点数", 
        AchievementCategory::MILESTONE, AchievementRarity::GOLD, 200, 5000, "total_points"};
    achievements_["ach_milestone_allfruits"] = {"ach_milestone_allfruits", "📊 全类型消除", "单局消除过所有6种水果", 
        AchievementCategory::MILESTONE, AchievementRarity::SILVER, 50, 6, "all_fruits"};
    achievements_["ach_milestone_master"] = {"ach_milestone_master", "👑 水果大师", "解锁全部成就", 
        AchievementCategory::MILESTONE, AchievementRarity::DIAMOND, 1000, 61, "all_achievements"};
}

/**
 * @brief 获取单个成就定义
 */
AchievementDef AchievementManager::getAchievement(const QString& achievementId) const
{
    return achievements_.value(achievementId, AchievementDef());
}

// ==================== 游戏数据接收接口 ====================

void AchievementManager::onGameStart(const QString& mode)
{
    QMutexLocker locker(&dataMutex_);
    currentGameData_ = GameDataSnapshot();  // 重置
    currentGameData_.gameMode = mode;
    currentGameData_.gameStartTime = QDateTime::currentMSecsSinceEpoch();
    
    emit gameStarted(mode);
}

void AchievementManager::onGameEnd(const GameDataSnapshot& snapshot)
{
    emit gameEnded(snapshot);
}

void AchievementManager::onMatchEliminate(const GameDataSnapshot& snapshot)
{
    emit gameDataReceived(snapshot);
}

void AchievementManager::onSpecialGenerated(const QString& specialType)
{
    QMutexLocker locker(&dataMutex_);
    currentGameData_.specialGenerated = specialType;
    emit gameDataReceived(currentGameData_);
}

void AchievementManager::onSpecialUsed(const QString& specialType, const QString& comboType)
{
    QMutexLocker locker(&dataMutex_);
    currentGameData_.specialUsed = specialType;
    currentGameData_.comboPairType = comboType;
    emit gameDataReceived(currentGameData_);
}

void AchievementManager::onPropUsed(const QString& propType, int chainEliminate)
{
    QMutexLocker locker(&dataMutex_);
    currentGameData_.propUsedType = propType;
    currentGameData_.propChainEliminate = chainEliminate;
    currentGameData_.propUsed = true;
    emit gameDataReceived(currentGameData_);
}

/**
 * @brief 记录游戏会话开始/结束（新统一接口）
 * 
 * 这是主循环推荐调用的接口，让成就系统自主管理会话生命周期
 */
void AchievementManager::recordGameSession(const QString& mode, bool isStarting)
{
    if (isStarting) {
        onGameStart(mode);
    } else {
        // 游戏结束时传递当前快照
        onGameEnd(currentGameData_);
    }
}

/**
 * @brief 记录游戏数据快照（新统一接口）
 * 
 * 主循环在每次消除时调用此方法，成就系统完全自主处理
 * 包含所有游戏事件：消除、特殊元素生成/使用、道具使用等
 */
void AchievementManager::recordGameSnapshot(const GameDataSnapshot& snapshot)
{
    QMutexLocker locker(&dataMutex_);
    currentGameData_ = snapshot;
    emit gameDataReceived(currentGameData_);
}

/**
 * @brief 处理成就解锁通知（从工作线程接收）
 */
void AchievementManager::handleAchievementUnlocked(const AchievementNotification& notification)
{
    if (notificationCallback_) {
        notificationCallback_(notification);
    }
}

// ==================== AchievementWorker 实现 ====================

AchievementWorker::AchievementWorker(const QMap<QString, AchievementDef>* achievements)
    : achievements_(achievements)
    , currentPlayerId_(Database::instance().getCurrentPlayerId())
    , gameEngine_(nullptr)
    , detectorManager_(nullptr)
{
    // 创建检测器管理器
    detectorManager_ = DetectorFactory::createDetectorManager().release();
    
    // 注册玩家统计数据
    if (detectorManager_) {
        DetectorFactory::registerPlayerStats(
            detectorManager_,
            currentPlayerId_,
            cumulativeStats_.totalGames,
            cumulativeStats_.totalCombo3Plus,
            cumulativeStats_.totalMatch4,
            cumulativeStats_.totalMatch5,
            cumulativeStats_.totalMatch6,
            cumulativeStats_.totalMatch8,
            cumulativeStats_.totalSpecialGenerated,
            cumulativeStats_.totalLineGenerated,
            cumulativeStats_.totalDiamondGenerated,
            cumulativeStats_.totalRainbowGenerated,
            cumulativeStats_.totalSpecialUsed,
            cumulativeStats_.totalPropUsed
        );
    }
}

AchievementWorker::~AchievementWorker()
{
    if (detectorManager_) {
        delete detectorManager_;
        detectorManager_ = nullptr;
    }
}

/**
 * @brief 设置当前玩家ID（切换账号时调用）
 */
void AchievementWorker::setCurrentPlayerId(const QString& playerId)
{
    currentPlayerId_ = playerId;
    // 清空上一个玩家的缓存
    triggeredThisSession_.clear();
    // 重置累计统计
    cumulativeStats_ = CumulativeStats();
    
    // 🔴 关键修复：更新所有检测器中的玩家ID
    if (detectorManager_) {
        DetectorFactory::registerPlayerStats(
            detectorManager_,
            currentPlayerId_,
            cumulativeStats_.totalGames,
            cumulativeStats_.totalCombo3Plus,
            cumulativeStats_.totalMatch4,
            cumulativeStats_.totalMatch5,
            cumulativeStats_.totalMatch6,
            cumulativeStats_.totalMatch8,
            cumulativeStats_.totalSpecialGenerated,
            cumulativeStats_.totalLineGenerated,
            cumulativeStats_.totalDiamondGenerated,
            cumulativeStats_.totalRainbowGenerated,
            cumulativeStats_.totalSpecialUsed,
            cumulativeStats_.totalPropUsed
        );
    }
    
}

void AchievementWorker::onGameStarted(const QString& mode)
{
    triggeredThisSession_.clear();
    
    // 游客模式：不从数据库加载
    if (currentPlayerId_ == "guest") {
        return;
    }
    
    // 📌 关键修复: 每次游戏启动时，都要从数据库重新加载该玩家的成就状态
    // 确保已完成或已领取的成就不会在同一会话内重复触发
    
    QList<AchievementProgress> allProgress = Database::instance()
        .getAllAchievementProgress(currentPlayerId_);
    
    for (const auto& progress : allProgress) {
        if (progress.state == AchievementState::COMPLETED || 
            progress.state == AchievementState::CLAIMED) {
            triggeredThisSession_.insert(progress.achievementId);
        }
    }
}


void AchievementWorker::onGameDataReceived(const GameDataSnapshot& snapshot)
{
    checkAllAchievements(snapshot);
}

void AchievementWorker::onGameEnded(const GameDataSnapshot& snapshot)
{
    // 最后一次检测（比如游戏结束成就）
    checkAllAchievements(snapshot);
    
    // 统计本局数据
    cumulativeStats_.totalGames++;
}

/**
 * @brief 检测所有成就类别（使用模块化检测器）
 */
void AchievementWorker::checkAllAchievements(const GameDataSnapshot& snapshot)
{
    if (!detectorManager_) {
        qWarning() << "DetectorManager not initialized!";
        return;
    }
    
    // 使用检测器管理器进行所有成就检测
    detectorManager_->detectAll(snapshot, [this](const QString& achievementId, int currentValue, int targetValue) {
        updateProgress(achievementId, currentValue, targetValue);
    });
}

/**
 * @brief 更新成就进度
 * @return true 如果成就已完成或新完成
 */
bool AchievementWorker::updateProgress(const QString& achievementId, int currentValue, int targetValue)
{
    // 本局缓存优先
    if (triggeredThisSession_.contains(achievementId)) {
        return false;
    }
    
    // 游客模式：内存状态跟踪
    if (currentPlayerId_ == "guest") {
        if (currentValue >= targetValue) {
            triggeredThisSession_.insert(achievementId);
            notifyUnlock(achievementId);
            return true;
        }
        return false;
    }
    
    // 查询数据库中该成就的当前状态
    AchievementProgress progress = Database::instance()
        .getAchievementProgress(currentPlayerId_, achievementId);
    
    // 已领取或已完成的成就不再处理
    if (progress.state == AchievementState::CLAIMED) {
        return false;
    }
    if (progress.state == AchievementState::COMPLETED) {
        triggeredThisSession_.insert(achievementId);
        return false;
    }
    
    // 更新进度
    if (currentValue > progress.currentValue) {
        Database::instance().updateAchievementProgress(currentPlayerId_, achievementId, currentValue);
    }
    
    // 检查是否完成
    if (currentValue >= targetValue && progress.state == AchievementState::LOCKED) {
        Database::instance().completeAchievement(currentPlayerId_, achievementId);
        triggeredThisSession_.insert(achievementId);
        notifyUnlock(achievementId);
        return true;
    }
    
    return false;
}

/**
 * @brief 发送成就解锁通知
 */
void AchievementWorker::notifyUnlock(const QString& achievementId)
{
    if (!achievements_->contains(achievementId)) {
        qWarning() << "Unknown achievement:" << achievementId;
        return;
    }
    
    const AchievementDef& def = (*achievements_)[achievementId];
    
    AchievementNotification notification;
    notification.achievementId = achievementId;
    notification.achievementName = def.name;
    notification.description = def.description;
    notification.icon = def.icon;
    notification.reward = def.reward;
    notification.rarity = def.rarity;
    notification.canClaim = true;
    
    // 添加成就奖励分数到游戏分数
    if (gameEngine_ && def.reward > 0) {
        gameEngine_->addScore(def.reward);
    }
    
    emit achievementUnlocked(notification);
}
