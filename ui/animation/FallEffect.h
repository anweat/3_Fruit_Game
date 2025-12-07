#ifndef FALLEFFECT_H
#define FALLEFFECT_H

#include "IAnimationEffect.h"
#include <vector>

/**
 * @brief 下落动画效果
 * 
 * 水果从上方位置下落到目标位置的动画
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
     */
    FallEffect(const std::vector<FallMove>& moves, const std::vector<NewFruit>& newFruits);
    
    void render(const AnimationContext& ctx, float progress) override;
    float getDuration() const override { return 180.0f; }  // 180ms
    const char* getName() const override { return "FallEffect"; }
    
private:
    void drawFruitAtOffset(const AnimationContext& ctx, 
                           int row, int col, const Fruit& fruit, float offsetY);
    
    std::vector<FallMove> moves_;
    std::vector<NewFruit> newFruits_;
};

#endif // FALLEFFECT_H
