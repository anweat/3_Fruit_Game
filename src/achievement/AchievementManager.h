#ifndef ACHIEVEMENTMANAGER_H
#define ACHIEVEMENTMANAGER_H

#include "AchievementDef.h"
#include <QObject>
#include <QMap>
#include <QThread>
#include <QMutex>
#include <QString>
#include <QSet>
#include <functional>

// 前向声明
class AchievementWorker;
class GameEngine;

/**
 * @brief 游戏数据快照（从游戏循环传递到成就系统）
 */
struct GameDataSnapshot {
    // 单局数据
    int currentScore = 0;           // 当前得分
    int currentCombo = 0;           // 当前连击
    int maxCombo = 0;               // 最大连击
    int moveCount = 0;              // 操作次数
    int eliminateCount = 0;         // 消除次数
    QString gameMode;               // 游戏模式
    qint64 gameStartTime = 0;       // 游戏开始时间(ms)
    bool propUsed = false;          // 本局是否使用道具
    
    // 本次消除数据
    int lastMatchSize = 0;          // 本次消除数量
    int lastMatchScore = 0;         // 本次消除得分
    bool isComboTrigger = false;    // 是否触发连击
    int chainReactionCount = 0;     // 连锁反应次数
    int lastMatchElementType = -1;  // 📌 修复问题 #1: 本次消除的元素类型（-1表示混合）
    bool lastMatchSameElement = false;  // 本次消除是否全为相同元素
    
    // 特殊元素数据
    QString specialGenerated;       // 生成的特殊元素类型(LINE_H/LINE_V/DIAMOND/RAINBOW)
    QString specialUsed;            // 使用的特殊元素类型
    QString comboPairType;          // 组合类型(如"LINE_LINE")
    int specialEliminateCount = 0;  // 特殊元素消除数量
    
    // 道具使用数据
    QString propUsedType;           // 使用的道具类型(Hammer/Clamp/MagicWand)
    int propChainEliminate = 0;     // 道具触发的连锁消除数量
    
    // 4/5/6消统计（本局累计）
    int match4Count = 0;            // 4消次数
    int match5Count = 0;            // 5消次数
    int match6Count = 0;            // 6消次数
    int match5PlusCount = 0;        // 5+消次数
    int match6PlusCount = 0;        // 6+消次数
    
    // 水果种类统计
    QSet<int> fruitTypesEliminated; // 本局消除过的水果类型
};

/**
 * @brief 成就通知数据
 */
struct AchievementNotification {
    QString achievementId;          // 成就ID
    QString achievementName;        // 成就名称
    QString description;            // 成就描述
    QString icon;                   // 成就图标 (emoji或文本)
    int reward;                     // 奖励点数
    AchievementRarity rarity;       // 稀有度
    bool canClaim;                  // 是否可以立即领取
};

/**
 * @brief 成就管理器（主线程）
 * 
 * 负责：
 * - 管理所有成就定义
 * - 启动成就监听线程
 * - 接收游戏数据快照
 * - 发送成就解锁通知
 */
class AchievementManager : public QObject {
    Q_OBJECT
    
public:
    static AchievementManager& instance();
    
    // 初始化和清理
    void initialize();
    void shutdown();
    
    // 成就定义管理
    const QMap<QString, AchievementDef>& getAllAchievements() const { return achievements_; }
    AchievementDef getAchievement(const QString& achievementId) const;
    
    // 游戏数据接收（统一入口，从主循环调用）
    void recordGameSession(const QString& mode, bool isStarting);  // 游戏开始/结束
    void recordGameSnapshot(const GameDataSnapshot& snapshot);     // 游戏数据快照（每次消除时调用）
    
    // 游戏事件通知（向后兼容）
    void onGameStart(const QString& mode);
    void onGameEnd(const GameDataSnapshot& snapshot);
    void onMatchEliminate(const GameDataSnapshot& snapshot);
    void onSpecialGenerated(const QString& specialType);
    void onSpecialUsed(const QString& specialType, const QString& comboType = "");
    void onPropUsed(const QString& propType, int chainEliminate = 0);
    
    // 设置通知回调（UI层注册）
    void setNotificationCallback(std::function<void(const AchievementNotification&)> callback) {
        notificationCallback_ = callback;
    }
    
    // 设置游戏引擎实例（用于成就奖励分数）
    void setGameEngine(GameEngine* engine);
    
    // 设置当前玩家ID（切换账号时调用）
    void setCurrentPlayerId(const QString& playerId);
    
signals:
    // 发送数据到工作线程
    void gameDataReceived(const GameDataSnapshot& snapshot);
    void gameStarted(const QString& mode);
    void gameEnded(const GameDataSnapshot& snapshot);
    
    // 接收工作线程的通知
    void achievementUnlocked(const AchievementNotification& notification);
    
private slots:
    void handleAchievementUnlocked(const AchievementNotification& notification);
    
private:
    AchievementManager();
    ~AchievementManager();
    AchievementManager(const AchievementManager&) = delete;
    AchievementManager& operator=(const AchievementManager&) = delete;
    
    void loadAchievementDefinitions();
    
    QMap<QString, AchievementDef> achievements_;        // 所有成就定义
    QThread* workerThread_;                            // 工作线程
    AchievementWorker* worker_;                        // 工作对象
    GameEngine* gameEngine_;                           // 游戏引擎（用于成就奖励）
    
    std::function<void(const AchievementNotification&)> notificationCallback_;
    
    GameDataSnapshot currentGameData_;                 // 当前游戏会话数据
    QMutex dataMutex_;                                 // 数据保护锁
};

/**
 * @brief 成就检测工作线程（独立线程）
 * 
 * 负责：
 * - 接收游戏数据
 * - 调用检测器进行成就检测
 * - 更新数据库中的成就进度
 * - 发送成就解锁通知
 */
class AchievementWorker : public QObject {
    Q_OBJECT
    
public:
    explicit AchievementWorker(const QMap<QString, AchievementDef>* achievements);
    ~AchievementWorker();
    
    // 设置游戏引擎实例（在成就完成时用于添加分数奖励）
    void setGameEngine(GameEngine* engine) {
        gameEngine_ = engine;
    }
    
    // 设置当前玩家ID（切换账号时调用）
    void setCurrentPlayerId(const QString& playerId);
    
public slots:
    void onGameDataReceived(const GameDataSnapshot& snapshot);
    void onGameStarted(const QString& mode);
    void onGameEnded(const GameDataSnapshot& snapshot);
    
signals:
    void achievementUnlocked(const AchievementNotification& notification);
    
private:
    void checkAllAchievements(const GameDataSnapshot& snapshot);
    
    bool updateProgress(const QString& achievementId, int currentValue, int targetValue);
    void notifyUnlock(const QString& achievementId);
    
    const QMap<QString, AchievementDef>* achievements_;
    QString currentPlayerId_;
    GameEngine* gameEngine_;                        // 游戏引擎实例（用于添加奖励分数）
    
    // 已触发成就缓存（本局）
    QSet<QString> triggeredThisSession_;
    
    // 累计统计数据（跨局）
    struct CumulativeStats {
        int totalGames = 0;
        int totalCombo3Plus = 0;        // 累计3+连击次数
        int totalMatch4 = 0;
        int totalMatch5 = 0;
        int totalMatch6 = 0;
        int totalMatch8 = 0;            // 累计整行/整列消除
        int totalSpecialGenerated = 0;
        int totalSpecialUsed = 0;
        int totalPropUsed = 0;
        int totalLineGenerated = 0;
        int totalDiamondGenerated = 0;
        int totalRainbowGenerated = 0;
    };
    CumulativeStats cumulativeStats_;
    
    // 模块化检测器
    class AchievementDetectorManager* detectorManager_;
};

#endif // ACHIEVEMENTMANAGER_H
