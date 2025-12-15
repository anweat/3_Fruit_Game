#ifndef RANKMANAGER_H
#define RANKMANAGER_H

#include <QString>
#include <QList>
#include <QDateTime>
#include "../mode/CompetitionMode.h"

/**
 * @brief 排行榜记录结构
 */
struct RankRecord {
    int rank = 0;               ///< 排名
    QString playerId;           ///< 玩家ID
    QString playerName;         ///< 玩家名称
    int score = 0;              ///< 得分
    int maxCombo = 0;           ///< 最大连击
    CompetitionDuration duration = CompetitionDuration::SECONDS_60;  ///< 比赛时长
    QDateTime playedAt;         ///< 游戏时间
};

/**
 * @brief 排行榜管理器
 * 
 * 负责管理比赛模式的排行榜数据：
 * - 记录比赛成绩
 * - 查询排行榜
 * - 获取玩家排名
 */
class RankManager {
public:
    static RankManager& instance();

    /**
     * @brief 记录比赛成绩
     * @param playerId 玩家ID
     * @param playerName 玩家名称
     * @param score 得分
     * @param maxCombo 最大连击
     * @param duration 比赛时长类型
     * @return 是否记录成功
     */
    bool recordScore(const QString& playerId, 
                     const QString& playerName,
                     int score, 
                     int maxCombo,
                     CompetitionDuration duration);

    /**
     * @brief 获取排行榜（指定时长）
     * @param duration 比赛时长类型
     * @param limit 返回数量限制（默认10）
     * @return 排行榜记录列表
     */
    QList<RankRecord> getLeaderboard(CompetitionDuration duration, int limit = 10);

    /**
     * @brief 获取玩家在指定时长的排名
     * @param playerId 玩家ID
     * @param duration 比赛时长类型
     * @return 排名（0表示未上榜）
     */
    int getPlayerRank(const QString& playerId, CompetitionDuration duration);

    /**
     * @brief 获取玩家的最高分
     * @param playerId 玩家ID
     * @param duration 比赛时长类型
     * @return 最高分（0表示无记录）
     */
    int getPlayerBestScore(const QString& playerId, CompetitionDuration duration);

    /**
     * @brief 检查是否打破个人记录
     * @param playerId 玩家ID
     * @param score 当前得分
     * @param duration 比赛时长类型
     * @return 是否打破个人记录
     */
    bool isPersonalBest(const QString& playerId, int score, CompetitionDuration duration);

    /**
     * @brief 获取指定时长的总参赛次数
     * @param duration 比赛时长类型
     * @return 总参赛次数
     */
    int getTotalGames(CompetitionDuration duration);

private:
    RankManager();
    ~RankManager();
    RankManager(const RankManager&) = delete;
    RankManager& operator=(const RankManager&) = delete;

    /**
     * @brief 将时长枚举转换为数据库字符串
     */
    static QString durationToString(CompetitionDuration duration);
};

#endif // RANKMANAGER_H
