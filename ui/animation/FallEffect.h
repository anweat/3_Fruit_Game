#ifndef FALLEFFECT_H
#define FALLEFFECT_H

#include "IAnimationEffect.h"
#include <vector>

/**
 * @brief 下落动画效果
 * 
 * 水果从上方位置下落到目标位置的动画
 * 关键设计：
 * - 所有新水果从网格顶部上方的**同一位置**开始下落（-mapSize 格处）
 * - 所有元素在**同一时间内**完成动画（基于网格总高度）
 * - 这样保证了视觉上的流畅性和一致性
 */
class FallEffect : public IAnimationEffect {
public:
    /**
     * @brief 下落移动数据
     */
    struct FallMove {
        int fromRow, fromCol;
        int toRow, toCol;
        Fruit fruit;
    };
    
    /**
     * @brief 新生成水果数据
     */
    struct NewFruit {
        int row, col;
        Fruit fruit;
    };
    
    /**
     * @brief 构造下落动画
     * @param moves 下落移动列表
     * @param newFruits 新生成水果列表
     * @param cellSize 网格格子大小
     * @param mapSize 地图大小（用于计算新水果的起始位置和总下落距离）
     */
    FallEffect(const std::vector<FallMove>& moves, const std::vector<NewFruit>& newFruits, 
               float cellSize = 40.0f, int mapSize = 8);
    
    void render(const AnimationContext& ctx, float progress) override;
    float getDuration() const override { return duration_; }  // 动态计算
    const char* getName() const override { return "FallEffect"; }
    
    /**
     * @brief 获取总下落距离（用于计算动画时长）
     */
    float getTotalFallDistance() const { return totalFallDistance_; }
    
private:
    void drawFruitAtOffset(const AnimationContext& ctx, 
                           int row, int col, const Fruit& fruit, float offsetY);
    
    std::vector<FallMove> moves_;
    std::vector<NewFruit> newFruits_;
    float totalFallDistance_ = 0.0f;  ///< 网格总下落距离（从上方到底部）
    float duration_ = 180.0f;          ///< 动画时长（毫秒）
    int mapSize_ = 8;                  ///< 地图大小
    float cellSize_ = 40.0f;           ///< 格子大小
    static constexpr float FALL_SPEED = 0.6f;  ///< 下落速度：像素/毫秒（600像素/秒）
};

#endif // FALLEFFECT_H
