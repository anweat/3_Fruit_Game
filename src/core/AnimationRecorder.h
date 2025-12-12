#ifndef ANIMATIONRECORDER_H
#define ANIMATIONRECORDER_H

#include "FruitTypes.h"
#include "FallProcessor.h"
#include "FruitGenerator.h"
#include <vector>
#include <tuple>
#include <set>

// 前置声明结构体
struct FallStep;
struct EliminationStep;
struct BombEffect;
enum class BombEffectType;

/**
 * @brief 动画记录器 - 记录游戏过程中的动画数据
 * 
 * 职责：
 * 1. 记录下落动画数据
 * 2. 记录填充动画数据
 * 3. 记录消除动画数据
 * 4. 记录炸弹特效数据
 */
class AnimationRecorder {
public:
    AnimationRecorder(FallProcessor& fallProcessor);
    ~AnimationRecorder();
    
    /**
     * @brief 记录下落和填充过程
     * @param map 游戏地图（会被修改）
     * @param fruitGenerator 水果生成器（用于填充新水果）
     * @param outFallStep 输出下落步骤数据
     */
    void recordFallAndRefill(std::vector<std::vector<Fruit>>& map,
                             FruitGenerator& fruitGenerator,
                             FallStep& outFallStep);
    
    /**
     * @brief 记录消除过程（带炸弹特效）
     * @param map 游戏地图（会被修改）
     * @param specialPositions 需要保留的特殊元素位置
     * @param outElimStep 输出消除步骤数据
     */
    void recordElimination(std::vector<std::vector<Fruit>>& map,
                           const std::set<std::pair<int, int>>& specialPositions,
                           EliminationStep& outElimStep);
    
    /**
     * @brief 记录炸弹特效
     */
    void recordBombEffect(int row, int col, SpecialType special,
                          std::vector<BombEffect>& bombEffects);
    
private:
    FallProcessor& fallProcessor_;
};

#endif // ANIMATIONRECORDER_H
