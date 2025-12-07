#ifndef SWAPEFFECT_H
#define SWAPEFFECT_H

#include "IAnimationEffect.h"

/**
 * @brief 交换动画效果
 * 
 * 两个水果之间的交换动画，支持成功交换和失败回弹
 */
class SwapEffect : public IAnimationEffect {
public:
    /**
     * @brief 构造交换动画
     * @param row1, col1 第一个水果的位置
     * @param row2, col2 第二个水果的位置
     * @param fruit1, fruit2 两个水果的数据（用于渲染）
     * @param success 交换是否成功（失败则回弹）
     */
    SwapEffect(int row1, int col1, int row2, int col2,
               const Fruit& fruit1, const Fruit& fruit2, bool success);
    
    void render(const AnimationContext& ctx, float progress) override;
    float getDuration() const override { return 200.0f; }  // 200ms
    const char* getName() const override { return "SwapEffect"; }
    
private:
    void drawFruitWithOffset(const AnimationContext& ctx, 
                             int row, int col, const Fruit& fruit,
                             float offsetX, float offsetY);
    
    int row1_, col1_, row2_, col2_;
    Fruit fruit1_, fruit2_;
    bool success_;
};

#endif // SWAPEFFECT_H
