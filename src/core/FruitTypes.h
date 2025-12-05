#ifndef FRUITTYPES_H
#define FRUITTYPES_H

#include <vector>
#include <utility>
#include <functional>
#include <string>
#include <ctime>

// ==================== 常量定义 ====================
const int MAP_SIZE = 8;                  // 游戏地图大小 8×8
const int FRUIT_TYPE_COUNT = 6;          // 水果类型数量

// ==================== 枚举定义 ====================

/**
 * @brief 水果类型枚举
 */
enum class FruitType {
    APPLE,      // 苹果 - 0
    ORANGE,     // 橙子 - 1
    GRAPE,      // 葡萄 - 2
    BANANA,     // 香蕉 - 3
    WATERMELON, // 西瓜 - 4
    STRAWBERRY, // 草莓 - 5
    EMPTY       // 空位 - 6 (用于消除后的空位)
};

/**
 * @brief 特殊元素类型枚举
 */
enum class SpecialType {
    NONE,           // 普通水果
    LINE_H,         // 横向直线炸弹 (4个水果横向消除)
    LINE_V,         // 纵向直线炸弹 (4个水果纵向消除)
    DIAMOND,        // 菱形炸弹 (L形或T形5个水果消除，消除5×5菱形范围)
    RAINBOW         // 万能炸弹/彩虹果 (5个及以上直线消除，消除场上所有同类型水果)
};

/**
 * @brief 动画类型枚举
 */
enum class AnimationType {
    SWAP,       // 交换动画
    ELIMINATE,  // 消除动画
    FALL,       // 下落动画
    GENERATE,   // 生成动画
    SPECIAL,    // 特殊效果动画 (炸弹爆炸等)
    COMBO       // 组合效果动画
};

/**
 * @brief 消除方向枚举 (用于判断生成的特殊元素类型)
 */
enum class MatchDirection {
    HORIZONTAL,  // 横向消除
    VERTICAL,    // 纵向消除
    L_SHAPE,     // L形消除
    T_SHAPE      // T形消除
};

// ==================== 结构体定义 ====================

/**
 * @brief 水果结构体 - 游戏地图的基本单元
 */
struct Fruit {
    FruitType type;           // 水果类型
    SpecialType special;      // 特殊类型
    int row;                  // 行坐标 (0-7)
    int col;                  // 列坐标 (0-7)
    bool isMatched;           // 是否被匹配 (标记待消除)
    bool isMoving;            // 是否正在移动 (动画状态)
    float animationProgress;  // 动画进度 [0.0, 1.0]
    
    // 构造函数
    Fruit() 
        : type(FruitType::EMPTY)
        , special(SpecialType::NONE)
        , row(0)
        , col(0)
        , isMatched(false)
        , isMoving(false)
        , animationProgress(0.0f) 
    {}
    
    Fruit(FruitType t, int r, int c)
        : type(t)
        , special(SpecialType::NONE)
        , row(r)
        , col(c)
        , isMatched(false)
        , isMoving(false)
        , animationProgress(0.0f)
    {}
};

/**
 * @brief 匹配结果结构体
 * 用于存储一次匹配检测的结果
 */
struct MatchResult {
    std::vector<std::pair<int, int>> positions;  // 匹配位置列表 (row, col)
    FruitType fruitType;                          // 匹配的水果类型
    SpecialType generateSpecial;                  // 应生成的特殊元素类型
    MatchDirection direction;                     // 消除方向
    int matchCount;                               // 匹配数量
    std::pair<int, int> specialPosition;          // 特殊元素生成位置 (用户点击的位置)
    
    MatchResult()
        : fruitType(FruitType::EMPTY)
        , generateSpecial(SpecialType::NONE)
        , direction(MatchDirection::HORIZONTAL)
        , matchCount(0)
        , specialPosition({-1, -1})
    {}
};

/**
 * @brief 动画任务结构体
 * 用于动画管理器的任务队列
 */
struct AnimationTask {
    AnimationType type;                           // 动画类型
    std::vector<std::pair<int, int>> targets;     // 目标位置列表
    float duration;                               // 动画时长 (秒)
    float elapsed;                                // 已执行时间 (秒)
    std::function<void()> onComplete;             // 完成回调函数
    
    AnimationTask()
        : type(AnimationType::SWAP)
        , duration(0.3f)
        , elapsed(0.0f)
        , onComplete(nullptr)
    {}
    
    AnimationTask(AnimationType t, float d = 0.3f)
        : type(t)
        , duration(d)
        , elapsed(0.0f)
        , onComplete(nullptr)
    {}
};

/**
 * @brief 玩家记录结构体
 * 用于排行榜
 */
struct PlayerRecord {
    std::string name;       // 玩家名称
    int score;              // 得分
    time_t timestamp;       // 时间戳
    std::string mode;       // 游戏模式 ("casual" 或 "competition")
    
    PlayerRecord()
        : name("")
        , score(0)
        , timestamp(0)
        , mode("casual")
    {}
    
    PlayerRecord(const std::string& n, int s, const std::string& m)
        : name(n)
        , score(s)
        , timestamp(std::time(nullptr))
        , mode(m)
    {}
    
    // 用于优先队列的比较 (最大堆)
    bool operator<(const PlayerRecord& other) const {
        return score < other.score;
    }
};

/**
 * @brief 成就数据结构体
 */
struct Achievement {
    int id;                     // 成就ID
    std::string name;           // 成就名称
    std::string description;    // 成就描述
    std::string category;       // 成就类别
    std::string rarity;         // 稀有度 (青铜/白银/黄金/钻石)
    int rewardPoints;           // 奖励点数
    bool unlocked;              // 是否已解锁
    int progress;               // 当前进度
    int target;                 // 目标值
    time_t unlockTime;          // 解锁时间
    
    Achievement()
        : id(0)
        , name("")
        , description("")
        , category("")
        , rarity("青铜")
        , rewardPoints(0)
        , unlocked(false)
        , progress(0)
        , target(1)
        , unlockTime(0)
    {}
};

/**
 * @brief 游戏统计数据结构体
 * 用于成就系统和数据展示
 */
struct GameStatistics {
    // 累计统计
    int totalGamesPlayed;           // 累计游玩局数
    int totalFruitsEliminated;      // 累计消除水果数
    int totalComboCount;            // 累计连击次数
    int totalToolsUsed;             // 累计道具使用次数
    
    // 特殊元素统计
    int lineBombsCreated;           // 累计生成直线炸弹数
    int diamondBombsCreated;        // 累计生成菱形炸弹数
    int rainbowBombsCreated;        // 累计生成万能炸弹数
    int specialElementsUsed;        // 累计使用特殊元素数
    
    // 单局记录
    int highestScore;               // 最高分
    int longestCombo;               // 最长连击
    int largestElimination;         // 单次最多消除数
    
    // 消除统计
    int match3Count;                // 3消次数
    int match4Count;                // 4消次数
    int match5Count;                // 5消次数
    int match6PlusCount;            // 6+消次数
    
    GameStatistics()
        : totalGamesPlayed(0)
        , totalFruitsEliminated(0)
        , totalComboCount(0)
        , totalToolsUsed(0)
        , lineBombsCreated(0)
        , diamondBombsCreated(0)
        , rainbowBombsCreated(0)
        , specialElementsUsed(0)
        , highestScore(0)
        , longestCombo(0)
        , largestElimination(0)
        , match3Count(0)
        , match4Count(0)
        , match5Count(0)
        , match6PlusCount(0)
    {}
};

// ==================== 辅助函数声明 ====================

/**
 * @brief 判断水果类型是否为空
 */
inline bool isEmpty(FruitType type) {
    return type == FruitType::EMPTY;
}

/**
 * @brief 判断是否为有效的地图坐标
 */
inline bool isValidPosition(int row, int col) {
    return row >= 0 && row < MAP_SIZE && col >= 0 && col < MAP_SIZE;
}

/**
 * @brief 判断两个位置是否相邻 (上下左右)
 */
inline bool isAdjacent(int row1, int col1, int row2, int col2) {
    int dr = std::abs(row1 - row2);
    int dc = std::abs(col1 - col2);
    return (dr == 1 && dc == 0) || (dr == 0 && dc == 1);
}

#endif // FRUITTYPES_H
