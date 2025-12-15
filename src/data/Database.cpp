#include "Database.h"
#include "../achievement/AchievementManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>
#include <QFile>

// 单例实例
Database& Database::instance()
{
    static Database instance;
    return instance;
}

Database::Database()
    : currentPlayerId_("admin")  // 默认玩家ID
{
}

Database::~Database()
{
    close();
}

/**
 * @brief 初始化数据库连接和表结构
 */
bool Database::initialize(const QString& dbPath)
{
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName(dbPath);
    
    if (!db_.open()) {
        qCritical() << "Failed to open database:" << db_.lastError().text();
        return false;
    }
    
    if (!createTables()) {
        qCritical() << "Failed to create tables";
        return false;
    }
    
    // 创建默认玩家(admin)
    if (!tableExists("players")) {
        qWarning() << "Players table doesn't exist after creation";
        return false;
    }
    
    // 检查是否已存在admin玩家
    QSqlQuery query(db_);
    query.prepare("SELECT player_id FROM players WHERE player_id = ?");
    query.addBindValue("admin");
    
    if (!query.exec()) {
        qCritical() << "Failed to query player:" << query.lastError().text();
        return false;
    }
    
    if (!query.next()) {
        // 不存在，创建admin玩家
        if (!createPlayer("admin", "Admin")) {
            qCritical() << "Failed to create default admin player";
            return false;
        }
    }
    
    // 📌 关键修复：总是确保成就被初始化（而不仅在玩家刚创建时）
    // 这样才能保证数据库中存在所有成就的记录
    if (!initializeAchievements("admin")) {
        qCritical() << "Failed to initialize achievements for admin";
        return false;
    }
    
    return true;
}


/**
 * @brief 关闭数据库连接
 */
void Database::close()
{
    if (db_.isOpen()) {
        db_.close();
    }
}

/**
 * @brief 检查表是否存在
 */
bool Database::tableExists(const QString& tableName)
{
    QSqlQuery query(db_);
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=?");
    query.addBindValue(tableName);
    
    if (!query.exec()) {
        qCritical() << "Failed to check table existence:" << query.lastError().text();
        return false;
    }
    
    return query.next();
}

/**
 * @brief 创建数据库表结构
 */
bool Database::createTables()
{
    QSqlQuery query(db_);
    
    // 1. 创建玩家表
    QString createPlayersTable = R"(
        CREATE TABLE IF NOT EXISTS players (
            player_id TEXT PRIMARY KEY,
            username TEXT NOT NULL,
            total_points INTEGER DEFAULT 0,
            hammer_count INTEGER DEFAULT 3,
            clamp_count INTEGER DEFAULT 3,
            magic_wand_count INTEGER DEFAULT 3,
            created_at TEXT NOT NULL,
            last_login TEXT NOT NULL
        )
    )";
    
    if (!query.exec(createPlayersTable)) {
        qCritical() << "Failed to create players table:" << query.lastError().text();
        return false;
    }
    
    // 尝试添加道具字段（如果表已存在但缺少这些字段）
    query.exec("ALTER TABLE players ADD COLUMN hammer_count INTEGER DEFAULT 3");
    query.exec("ALTER TABLE players ADD COLUMN clamp_count INTEGER DEFAULT 3");
    query.exec("ALTER TABLE players ADD COLUMN magic_wand_count INTEGER DEFAULT 3");
    
    // 2. 创建成就进度表
    QString createAchievementsTable = R"(
        CREATE TABLE IF NOT EXISTS achievement_progress (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            player_id TEXT NOT NULL,
            achievement_id TEXT NOT NULL,
            current_value INTEGER DEFAULT 0,
            target_value INTEGER NOT NULL,
            state INTEGER DEFAULT 0,
            completed_at TEXT,
            FOREIGN KEY (player_id) REFERENCES players(player_id),
            UNIQUE(player_id, achievement_id)
        )
    )";
    
    if (!query.exec(createAchievementsTable)) {
        qCritical() << "Failed to create achievement_progress table:" << query.lastError().text();
        return false;
    }
    
    // 3. 创建游戏记录表
    QString createGameRecordsTable = R"(
        CREATE TABLE IF NOT EXISTS game_records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            player_id TEXT NOT NULL,
            mode TEXT NOT NULL,
            score INTEGER NOT NULL,
            max_combo INTEGER DEFAULT 0,
            played_at TEXT NOT NULL,
            FOREIGN KEY (player_id) REFERENCES players(player_id)
        )
    )";
    
    if (!query.exec(createGameRecordsTable)) {
        qCritical() << "Failed to create game_records table:" << query.lastError().text();
        return false;
    }
    
    // 4. 创建比赛记录表（用于排行榜）
    QString createCompetitionRecordsTable = R"(
        CREATE TABLE IF NOT EXISTS competition_records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            player_id TEXT NOT NULL,
            player_name TEXT NOT NULL,
            score INTEGER NOT NULL,
            max_combo INTEGER DEFAULT 0,
            duration_type TEXT NOT NULL,
            played_at TEXT NOT NULL,
            FOREIGN KEY (player_id) REFERENCES players(player_id)
        )
    )";
    
    if (!query.exec(createCompetitionRecordsTable)) {
        qCritical() << "Failed to create competition_records table:" << query.lastError().text();
        return false;
    }
    
    // 创建索引以提升查询性能
    query.exec("CREATE INDEX IF NOT EXISTS idx_achievement_player ON achievement_progress(player_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_game_records_player ON game_records(player_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_competition_records_duration ON competition_records(duration_type)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_competition_records_score ON competition_records(score DESC)");
    
    return true;
}

// ==================== 玩家数据操作 ====================

/**
 * @brief 创建新玩家
 */
bool Database::createPlayer(const QString& playerId, const QString& username)
{
    QSqlQuery query(db_);
    query.prepare(R"(
        INSERT INTO players (player_id, username, total_points, hammer_count, clamp_count, magic_wand_count, created_at, last_login)
        VALUES (?, ?, 0, 3, 3, 3, ?, ?)
    )");
    
    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
    query.addBindValue(playerId);
    query.addBindValue(username);
    query.addBindValue(now);
    query.addBindValue(now);
    
    if (!query.exec()) {
        qCritical() << "Failed to create player:" << query.lastError().text();
        return false;
    }
    return true;
}

/**
 * @brief 获取玩家数据
 * @return 如果玩家存在返回其数据（playerId非空），不存在则返回空PlayerData
 */
PlayerData Database::getPlayer(const QString& playerId)
{
    PlayerData data;
    // 注意：playerId 初始为空，只有在数据库中找到玩家时才设置
    
    QSqlQuery query(db_);
    query.prepare("SELECT player_id, username, total_points, created_at, last_login FROM players WHERE player_id = ?");
    query.addBindValue(playerId);
    
    if (!query.exec()) {
        qCritical() << "Failed to get player:" << query.lastError().text();
        return data;  // 返回空 PlayerData
    }
    
    if (query.next()) {
        // 只有在找到记录时才设置 playerId
        data.playerId = query.value(0).toString();
        data.username = query.value(1).toString();
        data.totalPoints = query.value(2).toInt();
        data.createdAt = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        data.lastLogin = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
    }
    
    return data;
}

/**
 * @brief 更新玩家点数(增量)
 */
bool Database::updatePlayerPoints(const QString& playerId, int points)
{
    QSqlQuery query(db_);
    query.prepare("UPDATE players SET total_points = total_points + ? WHERE player_id = ?");
    query.addBindValue(points);
    query.addBindValue(playerId);
    
    if (!query.exec()) {
        qCritical() << "Failed to update player points:" << query.lastError().text();
        return false;
    }
    
    return true;
}

/**
 * @brief 更新最后登录时间
 */
bool Database::updateLastLogin(const QString& playerId)
{
    QSqlQuery query(db_);
    query.prepare("UPDATE players SET last_login = ? WHERE player_id = ?");
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    query.addBindValue(playerId);
    
    if (!query.exec()) {
        qCritical() << "Failed to update last login:" << query.lastError().text();
        return false;
    }
    
    return true;
}

/**
 * @brief 获取玩家当前分数（用于休闲模式分数恢复）
 */
int Database::getPlayerScore(const QString& playerId)
{
    QSqlQuery query(db_);
    query.prepare("SELECT total_points FROM players WHERE player_id = ?");
    query.addBindValue(playerId);
    
    if (!query.exec()) {
        qWarning() << "Failed to query player score:" << query.lastError().text();
        return 0;
    }
    
    if (query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

/**
 * @brief 保存玩家分数（用于休闲模式分数累积）
 */
bool Database::savePlayerScore(const QString& playerId, int score)
{
    QSqlQuery query(db_);
    query.prepare("UPDATE players SET total_points = ? WHERE player_id = ?");
    query.addBindValue(score);
    query.addBindValue(playerId);
    
    if (!query.exec()) {
        qCritical() << "Failed to save player score:" << query.lastError().text();
        return false;
    }
    
    if (query.numRowsAffected() == 0) {
        qWarning() << "No player found to update score:" << playerId;
        return false;
    }
    
    return true;
}

/**
 * @brief 获取玩家道具数量
 */
Database::PropData Database::getPlayerProps(const QString& playerId)
{
    PropData props;  // 默认值 3, 3, 3
    
    QSqlQuery query(db_);
    query.prepare("SELECT hammer_count, clamp_count, magic_wand_count FROM players WHERE player_id = ?");
    query.addBindValue(playerId);
    
    if (!query.exec()) {
        qWarning() << "Failed to query player props:" << query.lastError().text();
        return props;
    }
    
    if (query.next()) {
        props.hammerCount = query.value(0).toInt();
        props.clampCount = query.value(1).toInt();
        props.magicWandCount = query.value(2).toInt();
    }
    
    return props;
}

/**
 * @brief 保存玩家道具数量
 */
bool Database::savePlayerProps(const QString& playerId, int hammer, int clamp, int magicWand)
{
    QSqlQuery query(db_);
    query.prepare("UPDATE players SET hammer_count = ?, clamp_count = ?, magic_wand_count = ? WHERE player_id = ?");
    query.addBindValue(hammer);
    query.addBindValue(clamp);
    query.addBindValue(magicWand);
    query.addBindValue(playerId);
    
    if (!query.exec()) {
        qCritical() << "Failed to save player props:" << query.lastError().text();
        return false;
    }
    
    if (query.numRowsAffected() == 0) {
        qWarning() << "No player found to update props:" << playerId;
        return false;
    }
    
    return true;
}

// ==================== 成就进度操作 ====================

/**
 * @brief 初始化玩家的所有成就(从AchievementManager获取定义)
 */
bool Database::initializeAchievements(const QString& playerId)
{
    // 从AchievementManager获取所有成就定义
    const auto& achievements = AchievementManager::instance().getAllAchievements();
    
    if (achievements.isEmpty()) {
        qWarning() << "No achievements defined yet";
        return true;
    }
    
    QSqlQuery query(db_);
    
    for (auto it = achievements.constBegin(); it != achievements.constEnd(); ++it) {
        const QString& achievementId = it.key();
        const AchievementDef& def = it.value();
        
        // INSERT OR IGNORE 避免重复插入
        query.prepare(R"(
            INSERT OR IGNORE INTO achievement_progress 
            (player_id, achievement_id, current_value, target_value, state)
            VALUES (?, ?, 0, ?, 0)
        )");
        query.addBindValue(playerId);
        query.addBindValue(achievementId);
        query.addBindValue(def.targetValue);
        
        if (!query.exec()) {
            qCritical() << "Failed to initialize achievement" << achievementId << ":" << query.lastError().text();
            return false;
        }
    }
    
    return true;
}

/**
 * @brief 获取单个成就的进度
 */
AchievementProgress Database::getAchievementProgress(const QString& playerId, const QString& achievementId)
{
    AchievementProgress progress;
    progress.playerId = playerId;
    progress.achievementId = achievementId;
    progress.currentValue = 0;
    progress.targetValue = 0;
    progress.state = AchievementState::LOCKED;
    
    QSqlQuery query(db_);
    query.prepare(R"(
        SELECT current_value, target_value, state, completed_at
        FROM achievement_progress
        WHERE player_id = ? AND achievement_id = ?
    )");
    query.addBindValue(playerId);
    query.addBindValue(achievementId);
    
    if (!query.exec()) {
        qCritical() << "Failed to get achievement progress:" << query.lastError().text();
        return progress;
    }
    
    if (query.next()) {
        progress.currentValue = query.value(0).toInt();
        progress.targetValue = query.value(1).toInt();
        progress.state = static_cast<AchievementState>(query.value(2).toInt());
        
        QString completedAtStr = query.value(3).toString();
        if (!completedAtStr.isEmpty()) {
            progress.completedAt = QDateTime::fromString(completedAtStr, Qt::ISODate);
        }
    }
    
    return progress;
}

/**
 * @brief 获取玩家的所有成就进度
 */
QList<AchievementProgress> Database::getAllAchievementProgress(const QString& playerId)
{
    QList<AchievementProgress> progressList;
    
    QSqlQuery query(db_);
    query.prepare(R"(
        SELECT achievement_id, current_value, target_value, state, completed_at
        FROM achievement_progress
        WHERE player_id = ?
    )");
    query.addBindValue(playerId);
    
    if (!query.exec()) {
        qCritical() << "Failed to get all achievement progress:" << query.lastError().text();
        return progressList;
    }
    
    int recordCount = 0;
    while (query.next()) {
        recordCount++;
        AchievementProgress progress;
        progress.playerId = playerId;
        progress.achievementId = query.value(0).toString();
        progress.currentValue = query.value(1).toInt();
        progress.targetValue = query.value(2).toInt();
        progress.state = static_cast<AchievementState>(query.value(3).toInt());
        
        QString completedAtStr = query.value(4).toString();
        if (!completedAtStr.isEmpty()) {
            progress.completedAt = QDateTime::fromString(completedAtStr, Qt::ISODate);
        }
        
        progressList.append(progress);
    }
    
    return progressList;
}

/**
 * @brief 更新成就进度
 */
bool Database::updateAchievementProgress(const QString& playerId, const QString& achievementId, int currentValue)
{
    QSqlQuery query(db_);
    query.prepare(R"(
        UPDATE achievement_progress
        SET current_value = ?
        WHERE player_id = ? AND achievement_id = ?
    )");
    query.addBindValue(currentValue);
    query.addBindValue(playerId);
    query.addBindValue(achievementId);
    
    if (!query.exec()) {
        qCritical() << "Failed to update achievement progress:" << query.lastError().text();
        return false;
    }
    
    return true;
}

/**
 * @brief 标记成就为已完成
 */
bool Database::completeAchievement(const QString& playerId, const QString& achievementId)
{
    QSqlQuery query(db_);
    query.prepare(R"(
        UPDATE achievement_progress
        SET state = ?, completed_at = ?
        WHERE player_id = ? AND achievement_id = ?
    )");
    query.addBindValue(static_cast<int>(AchievementState::COMPLETED));
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    query.addBindValue(playerId);
    query.addBindValue(achievementId);
    
    if (!query.exec()) {
        qCritical() << "Failed to complete achievement:" << query.lastError().text();
        return false;
    }
    
    return true;
}


/**
 * @brief 领取成就奖励
 */
bool Database::claimAchievementReward(const QString& playerId, const QString& achievementId, int reward)
{
    // 开启事务
    db_.transaction();
    
    // 1. 更新成就状态为已领取
    QSqlQuery query1(db_);
    query1.prepare(R"(
        UPDATE achievement_progress
        SET state = ?
        WHERE player_id = ? AND achievement_id = ?
    )");
    query1.addBindValue(static_cast<int>(AchievementState::CLAIMED));
    query1.addBindValue(playerId);
    query1.addBindValue(achievementId);
    
    if (!query1.exec()) {
        db_.rollback();
        qCritical() << "Failed to claim achievement:" << query1.lastError().text();
        return false;
    }
    
    // 2. 增加玩家点数
    QSqlQuery query2(db_);
    query2.prepare("UPDATE players SET total_points = total_points + ? WHERE player_id = ?");
    query2.addBindValue(reward);
    query2.addBindValue(playerId);
    
    if (!query2.exec()) {
        db_.rollback();
        qCritical() << "Failed to update points:" << query2.lastError().text();
        return false;
    }
    
    // 提交事务
    db_.commit();
    return true;
}

// ==================== 游戏记录操作 ====================

/**
 * @brief 保存游戏记录
 */
bool Database::saveGameRecord(const GameRecord& record)
{
    QSqlQuery query(db_);
    query.prepare(R"(
        INSERT INTO game_records (player_id, mode, score, max_combo, played_at)
        VALUES (?, ?, ?, ?, ?)
    )");
    query.addBindValue(record.playerId);
    query.addBindValue(record.mode);
    query.addBindValue(record.score);
    query.addBindValue(record.maxCombo);
    query.addBindValue(record.playedAt.toString(Qt::ISODate));
    
    if (!query.exec()) {
        qCritical() << "Failed to save game record:" << query.lastError().text();
        return false;
    }
    
    return true;
}

/**
 * @brief 获取玩家的游戏记录
 */
QList<GameRecord> Database::getGameRecords(const QString& playerId, int limit)
{
    QList<GameRecord> records;
    
    QSqlQuery query(db_);
    query.prepare(R"(
        SELECT mode, score, max_combo, played_at
        FROM game_records
        WHERE player_id = ?
        ORDER BY played_at DESC
        LIMIT ?
    )");
    query.addBindValue(playerId);
    query.addBindValue(limit);
    
    if (!query.exec()) {
        qCritical() << "Failed to get game records:" << query.lastError().text();
        return records;
    }
    
    while (query.next()) {
        GameRecord record;
        record.playerId = playerId;
        record.mode = query.value(0).toString();
        record.score = query.value(1).toInt();
        record.maxCombo = query.value(2).toInt();
        record.playedAt = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        records.append(record);
    }
    
    return records;
}

// ==================== 统计查询 ====================

/**
 * @brief 获取玩家总游戏局数
 */
int Database::getTotalGamesPlayed(const QString& playerId)
{
    QSqlQuery query(db_);
    query.prepare("SELECT COUNT(*) FROM game_records WHERE player_id = ?");
    query.addBindValue(playerId);
    
    if (!query.exec() || !query.next()) {
        return 0;
    }
    
    return query.value(0).toInt();
}

/**
 * @brief 获取玩家最高分
 */
int Database::getHighestScore(const QString& playerId)
{
    QSqlQuery query(db_);
    query.prepare("SELECT MAX(score) FROM game_records WHERE player_id = ?");
    query.addBindValue(playerId);
    
    if (!query.exec() || !query.next()) {
        return 0;
    }
    
    return query.value(0).toInt();
}

/**
 * @brief 获取已完成的成就数量
 */
int Database::getCompletedAchievementCount(const QString& playerId)
{
    QSqlQuery query(db_);
    query.prepare(R"(
        SELECT COUNT(*) FROM achievement_progress
        WHERE player_id = ? AND state >= ?
    )");
    query.addBindValue(playerId);
    query.addBindValue(static_cast<int>(AchievementState::COMPLETED));
    
    if (!query.exec() || !query.next()) {
        return 0;
    }
    
    return query.value(0).toInt();
}
