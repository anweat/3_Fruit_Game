#ifndef SWAPHANDLER_H
#define SWAPHANDLER_H

#include "FruitTypes.h"
#include "MatchDetector.h"
#include "SpecialEffectProcessor.h"
#include <vector>
#include <set>

// 前置声明GameEngine中的结构体
struct SwapStep;
struct GameRound;
struct BombEffect;
enum class BombEffectType;

/**
 * @brief 交换处理器 - 处理所有水果交换逻辑
 * 
 * 职责：
 * 1. 验证交换合法性
 * 2. 处理普通水果交换
 * 3. 处理 CANDY 特殊交换
 * 4. 处理炸弹元素交换（组合效果）
 * 5. 生成交换动画数据
 */
class SwapHandler {
public:
    SwapHandler(MatchDetector& matchDetector, 
                SpecialEffectProcessor& specialProcessor);
    ~SwapHandler();
    
    /**
     * @brief 执行交换操作
     * @param map 游戏地图（会被修改）
     * @param row1 第一个位置行
     * @param col1 第一个位置列
     * @param row2 第二个位置行
     * @param col2 第二个位置列
     * @param outSwapStep 输出交换步骤信息
     * @param outRounds 输出交换产生的消除轮次（炸弹组合/CANDY效果）
     * @return 交换是否成功
     */
    bool executeSwap(std::vector<std::vector<Fruit>>& map,
                     int row1, int col1, int row2, int col2,
                     SwapStep& outSwapStep,
                     std::vector<GameRound>& outRounds);
    
private:
    /**
     * @brief 验证交换是否合法
     * @param map 游戏地图
     * @param row1 第一个位置行
     * @param col1 第一个位置列
     * @param row2 第二个位置行
     * @param col2 第二个位置列
     * @return 是否合法
     */
    bool isValidSwap(const std::vector<std::vector<Fruit>>& map,
                     int row1, int col1, int row2, int col2) const;
    
    /**
     * @brief 处理普通交换（检测匹配）
     * @return 是否有匹配（无匹配则撤销交换）
     */
    bool handleNormalSwap(std::vector<std::vector<Fruit>>& map,
                          int row1, int col1, int row2, int col2);
    
    /**
     * @brief 处理 CANDY 特殊交换
     * - CANDY + 普通：消除所有该类型
     * - CANDY + 炸弹：转化所有该类型为随机炸弹并引爆
     * - CANDY + CANDY：清除全部
     */
    void handleCandySwap(std::vector<std::vector<Fruit>>& map,
                         int row1, int col1, int row2, int col2,
                         bool isCandy1, bool isCandy2,
                         std::vector<GameRound>& outRounds);
    
    /**
     * @brief 处理炸弹组合交换（两个都是特殊元素）
     */
    void handleSpecialCombo(std::vector<std::vector<Fruit>>& map,
                            int row1, int col1, int row2, int col2,
                            std::vector<GameRound>& outRounds);
    
    /**
     * @brief 记录炸弹特效
     */
    void recordBombEffect(int row, int col, SpecialType special,
                          std::vector<BombEffect>& bombEffects);
    
    MatchDetector& matchDetector_;
    SpecialEffectProcessor& specialProcessor_;
};

#endif // SWAPHANDLER_H
