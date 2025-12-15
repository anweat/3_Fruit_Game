#include "RankManager.h"
#include "Database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

RankManager& RankManager::instance()
{
    static RankManager instance;
    return instance;
}

RankManager::RankManager()
{
}

RankManager::~RankManager()
{
}

QString RankManager::durationToString(CompetitionDuration duration)
{
    switch (duration) {
        case CompetitionDuration::SECONDS_60:
            return "competition_60";
        case CompetitionDuration::SECONDS_120:
            return "competition_120";
        case CompetitionDuration::SECONDS_180:
            return "competition_180";
        default:
            return "competition_60";
    }
}

bool RankManager::recordScore(const QString& playerId, 
                               const QString& playerName,
                               int score, 
                               int maxCombo,
                               CompetitionDuration duration)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO competition_records 
        (player_id, player_name, score, max_combo, duration_type, played_at)
        VALUES (?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(playerId);
    query.addBindValue(playerName);
    query.addBindValue(score);
    query.addBindValue(maxCombo);
    query.addBindValue(durationToString(duration));
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    
    if (!query.exec()) {
        qCritical() << "Failed to record competition score:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Competition score recorded:" << playerName << score << "points in" 
             << CompetitionMode::getDurationString(duration);
    return true;
}

QList<RankRecord> RankManager::getLeaderboard(CompetitionDuration duration, int limit)
{
    QList<RankRecord> records;
    
    QSqlQuery query;
    // 按分数降序排列，相同分数按时间升序（先达成的排前面）
    query.prepare(R"(
        SELECT player_id, player_name, score, max_combo, played_at
        FROM competition_records
        WHERE duration_type = ?
        ORDER BY score DESC, played_at ASC
        LIMIT ?
    )");
    
    query.addBindValue(durationToString(duration));
    query.addBindValue(limit);
    
    if (!query.exec()) {
        qCritical() << "Failed to get leaderboard:" << query.lastError().text();
        return records;
    }
    
    int rank = 1;
    while (query.next()) {
        RankRecord record;
        record.rank = rank++;
        record.playerId = query.value(0).toString();
        record.playerName = query.value(1).toString();
        record.score = query.value(2).toInt();
        record.maxCombo = query.value(3).toInt();
        record.duration = duration;
        record.playedAt = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
        records.append(record);
    }
    
    return records;
}

int RankManager::getPlayerRank(const QString& playerId, CompetitionDuration duration)
{
    // 获取玩家最高分
    int bestScore = getPlayerBestScore(playerId, duration);
    if (bestScore == 0) {
        return 0;
    }
    
    // 计算有多少玩家分数高于该玩家的最高分
    QSqlQuery query;
    query.prepare(R"(
        SELECT COUNT(DISTINCT player_id) 
        FROM competition_records 
        WHERE duration_type = ? AND score > ?
    )");
    
    query.addBindValue(durationToString(duration));
    query.addBindValue(bestScore);
    
    if (!query.exec() || !query.next()) {
        qCritical() << "Failed to get player rank:" << query.lastError().text();
        return 0;
    }
    
    return query.value(0).toInt() + 1;
}

int RankManager::getPlayerBestScore(const QString& playerId, CompetitionDuration duration)
{
    QSqlQuery query;
    query.prepare(R"(
        SELECT MAX(score) 
        FROM competition_records 
        WHERE player_id = ? AND duration_type = ?
    )");
    
    query.addBindValue(playerId);
    query.addBindValue(durationToString(duration));
    
    if (!query.exec() || !query.next()) {
        qCritical() << "Failed to get player best score:" << query.lastError().text();
        return 0;
    }
    
    return query.value(0).toInt();
}

bool RankManager::isPersonalBest(const QString& playerId, int score, CompetitionDuration duration)
{
    int bestScore = getPlayerBestScore(playerId, duration);
    return score > bestScore;
}

int RankManager::getTotalGames(CompetitionDuration duration)
{
    QSqlQuery query;
    query.prepare(R"(
        SELECT COUNT(*) 
        FROM competition_records 
        WHERE duration_type = ?
    )");
    
    query.addBindValue(durationToString(duration));
    
    if (!query.exec() || !query.next()) {
        qCritical() << "Failed to get total games:" << query.lastError().text();
        return 0;
    }
    
    return query.value(0).toInt();
}
