#ifndef ELIMINATEEFFECT_H
#define ELIMINATEEFFECT_H

#include "IAnimationEffect.h"
#include <vector>

/**
 * @brief 消除动画效果
 * 
 * 水果缩放+淡出的消除动画
 */
class EliminateEffect : public IAnimationEffect {
public:
    /**
     * @brief 消除位置数据
     */
    struct Position {
        int row;
        int col;
        Fruit fruit;  // 被消除的水果
    };
    
    /**
     * @brief 构造消除动画
     * @param positions 需要消除的位置列表（包含水果数据）
     */
    explicit EliminateEffect(const std::vector<Position>& positions);
    
    void render(const AnimationContext& ctx, float progress) override;
    float getDuration() const override { return 220.0f; }  // 220ms
    const char* getName() const override { return "EliminateEffect"; }
    
private:
    std::vector<Position> positions_;
};

#endif // ELIMINATEEFFECT_H
