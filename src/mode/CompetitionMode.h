#ifndef COMPETITIONMODE_H
#define COMPETITIONMODE_H

#include <QObject>
#include <QTimer>

/**
 * @brief 比赛时长类型枚举
 */
enum class CompetitionDuration {
    SECONDS_60 = 60,    ///< 60秒赛
    SECONDS_120 = 120,  ///< 120秒赛
    SECONDS_180 = 180   ///< 180秒赛
};

/**
 * @brief 比赛模式配置
 */
struct CompetitionConfig {
    CompetitionDuration duration = CompetitionDuration::SECONDS_60;
    int mapSize = 8;          ///< 固定8x8地图
    int hammerCount = 2;      ///< 锤子数量
    int clampCount = 1;       ///< 夹子数量
    int magicWandCount = 1;   ///< 魔法棒数量
};

/**
 * @brief 比赛模式管理类
 * 
 * 管理比赛模式的：
 * - 倒计时
 * - 道具配给
 * - 比赛状态
 */
class CompetitionMode : public QObject {
    Q_OBJECT

public:
    explicit CompetitionMode(QObject* parent = nullptr);
    ~CompetitionMode();

    /**
     * @brief 设置比赛配置
     * @param config 比赛配置
     */
    void setConfig(const CompetitionConfig& config);

    /**
     * @brief 获取比赛配置
     */
    const CompetitionConfig& getConfig() const { return config_; }

    /**
     * @brief 开始比赛
     */
    void startCompetition();

    /**
     * @brief 暂停比赛
     */
    void pauseCompetition();

    /**
     * @brief 恢复比赛
     */
    void resumeCompetition();

    /**
     * @brief 结束比赛（正常结束，记录成绩）
     */
    void endCompetition();

    /**
     * @brief 放弃比赛（不记录成绩）
     */
    void abandonCompetition();

    /**
     * @brief 获取剩余时间（秒）
     */
    int getRemainingTime() const { return remainingTime_; }

    /**
     * @brief 获取比赛是否进行中
     */
    bool isRunning() const { return isRunning_; }

    /**
     * @brief 获取比赛是否暂停
     */
    bool isPaused() const { return isPaused_; }

    /**
     * @brief 获取比赛时长（秒）
     */
    int getDurationSeconds() const { return static_cast<int>(config_.duration); }

    /**
     * @brief 获取比赛时长类型的字符串表示
     */
    static QString getDurationString(CompetitionDuration duration);

signals:
    /**
     * @brief 时间更新信号（每秒触发）
     * @param remainingSeconds 剩余秒数
     */
    void timeUpdated(int remainingSeconds);

    /**
     * @brief 比赛结束信号（正常结束）
     */
    void competitionEnded();

    /**
     * @brief 比赛放弃信号（中途退出）
     */
    void competitionAbandoned();

    /**
     * @brief 比赛开始信号
     */
    void competitionStarted();

private slots:
    /**
     * @brief 定时器触发
     */
    void onTimerTick();

private:
    CompetitionConfig config_;
    QTimer* timer_;
    int remainingTime_;      ///< 剩余时间（秒）
    bool isRunning_;
    bool isPaused_;
};

#endif // COMPETITIONMODE_H
