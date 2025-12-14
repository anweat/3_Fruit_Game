#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QString>
#include <QDateTime>
#include <memory>

/**
 * @brief 成就完成状态
 */
enum class AchievementState {
    LOCKED = 0,      // 未完成
    COMPLETED = 1,   // 已完成但未领取
    CLAIMED = 2      // 已领取奖励
};

/**
 * @brief 玩家数据结构
 */
struct PlayerData {
    QString playerId;        // 玩家ID
    QString username;        // 玩家名称
    int totalPoints;         // 总点数
    QDateTime createdAt;     // 创建时间
    QDateTime lastLogin;     // 最后登录时间
};

/**
 * @brief 成就进度数据
 */
struct AchievementProgress {
    QString playerId;        // 玩家ID
    QString achievementId;   // 成就ID
    int currentValue;        // 当前进度
    int targetValue;         // 目标值
    AchievementState state;  // 完成状态
    QDateTime completedAt;   // 完成时间
};

/**
 * @brief 游戏记录数据
 */
struct GameRecord {
    QString playerId;        // 玩家ID
    QString mode;            // 游戏模式
    int score;               // 得分
    int maxCombo;            // 最大连击
    QDateTime playedAt;      // 游戏时间
};

/**
 * @brief 数据库管理类
 * 
 * 负责管理SQLite数据库的连接和操作，包括：
 * - 玩家数据的增删改查
 * - 成就进度的跟踪和更新
 * - 游戏记录的保存
 */
class Database {
public:
    static Database& instance();
    
    // 初始化数据库
    bool initialize(const QString& dbPath = "fruitcrush.db");
    void close();
    
    // 玩家数据操作
    bool createPlayer(const QString& playerId, const QString& username);
    PlayerData getPlayer(const QString& playerId);
    bool updatePlayerPoints(const QString& playerId, int points);
    bool updateLastLogin(const QString& playerId);
    QString getCurrentPlayerId() const { return currentPlayerId_; }
    void setCurrentPlayerId(const QString& playerId) { currentPlayerId_ = playerId; }
    
    // 📌 修复问题 #3: 休闲模式分数持久化
    int getPlayerScore(const QString& playerId);      // 获取玩家当前分数
    bool savePlayerScore(const QString& playerId, int score);  // 保存玩家分数
    
    // 道具数据操作
    struct PropData {
        int hammerCount = 3;
        int clampCount = 3;
        int magicWandCount = 3;
    };
    PropData getPlayerProps(const QString& playerId);  // 获取玩家道具数量
    bool savePlayerProps(const QString& playerId, int hammer, int clamp, int magicWand);  // 保存玩家道具
    
    // 成就进度操作
    bool initializeAchievements(const QString& playerId);
    AchievementProgress getAchievementProgress(const QString& playerId, const QString& achievementId);
    QList<AchievementProgress> getAllAchievementProgress(const QString& playerId);
    bool updateAchievementProgress(const QString& playerId, const QString& achievementId, int currentValue);
    bool completeAchievement(const QString& playerId, const QString& achievementId);
    bool claimAchievementReward(const QString& playerId, const QString& achievementId, int reward);
    
    // 游戏记录操作
    bool saveGameRecord(const GameRecord& record);
    QList<GameRecord> getGameRecords(const QString& playerId, int limit = 10);
    
    // 统计查询
    int getTotalGamesPlayed(const QString& playerId);
    int getHighestScore(const QString& playerId);
    int getCompletedAchievementCount(const QString& playerId);

private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    bool createTables();
    bool tableExists(const QString& tableName);
    
    QSqlDatabase db_;
    QString currentPlayerId_;  // 当前玩家ID（会话级别）
};

#endif // DATABASE_H
