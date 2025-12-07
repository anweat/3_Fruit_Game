#ifndef IANIMATIONEFFECT_H
#define IANIMATIONEFFECT_H

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <vector>
#include "FruitTypes.h"
#include "GameEngine.h"  // for BombEffectType

/**
 * @brief 动画渲染上下文 - 传递给动画效果的渲染参数
 */
struct AnimationContext {
    float gridStartX;       ///< 网格起始X坐标
    float gridStartY;       ///< 网格起始Y坐标
    float cellSize;         ///< 单元格大小
    int mapSize;            ///< 地图尺寸
    std::vector<QOpenGLTexture*>* textures;  ///< 纹理数组指针
    
    /// 计算单元格屏幕坐标
    float cellX(int col) const { return gridStartX + col * cellSize; }
    float cellY(int row) const { return gridStartY + row * cellSize; }
    float cellCenterX(int col) const { return cellX(col) + cellSize * 0.5f; }
    float cellCenterY(int row) const { return cellY(row) + cellSize * 0.5f; }
};

/**
 * @brief 动画效果基类（策略模式）
 * 
 * 所有动画效果都实现此接口，可以独立测试和扩展
 */
class IAnimationEffect {
public:
    virtual ~IAnimationEffect() = default;
    
    /**
     * @brief 渲染动画效果
     * @param ctx 渲染上下文
     * @param progress 动画进度 (0.0 ~ 1.0)
     */
    virtual void render(const AnimationContext& ctx, float progress) = 0;
    
    /**
     * @brief 获取动画持续时间（毫秒）
     */
    virtual float getDuration() const = 0;
    
    /**
     * @brief 获取动画类型名称（用于调试）
     */
    virtual const char* getName() const = 0;
};

/**
 * @brief 动画效果工厂 - 创建动画效果实例
 */
class AnimationEffectFactory {
public:
    static AnimationEffectFactory& instance();
    
    // 禁用拷贝
    AnimationEffectFactory(const AnimationEffectFactory&) = delete;
    AnimationEffectFactory& operator=(const AnimationEffectFactory&) = delete;
    
private:
    AnimationEffectFactory() = default;
};

#endif // IANIMATIONEFFECT_H
