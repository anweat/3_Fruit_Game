#ifndef BOMBANIMEFFECT_H
#define BOMBANIMEFFECT_H

#include "IAnimationEffect.h"
#include <vector>

/**
 * @brief 单个炸弹效果数据（用于动画系统）
 */
struct BombAnimData {
    BombEffectType type = BombEffectType::NONE;
    int row = 0;
    int col = 0;
    int range = 2;  // 菱形范围
};

/**
 * @brief 炸弹特效动画
 * 
 * 支持4种炸弹特效：
 * - LINE_H: 横排白色长条，逐渐变窄变淡
 * - LINE_V: 竖排白色长条，逐渐变窄变淡  
 * - DIAMOND: 白色正方形，逐渐放大变暗
 * - RAINBOW: 全屏白色闪光
 */
class BombAnimEffect : public IAnimationEffect {
public:
    /**
     * @brief 构造炸弹特效
     * @param effects 炸弹效果列表
     */
    explicit BombAnimEffect(const std::vector<BombAnimData>& effects);
    
    void render(const AnimationContext& ctx, float progress) override;
    float getDuration() const override { return 220.0f; }  // 220ms
    const char* getName() const override { return "BombAnimEffect"; }
    
private:
    void renderLineH(const AnimationContext& ctx, int row, float progress);
    void renderLineV(const AnimationContext& ctx, int col, float progress);
    void renderDiamond(const AnimationContext& ctx, int row, int col, int range, float progress);
    void renderRainbow(const AnimationContext& ctx, float progress);
    
    std::vector<BombAnimData> effects_;
};

#endif // BOMBANIMEFFECT_H
