#ifndef IANIMATIONRENDERER_H
#define IANIMATIONRENDERER_H

#include <QOpenGLFunctions>
#include <vector>
#include "FruitTypes.h"
#include "GameEngine.h"

class QOpenGLTexture;

/**
 * @brief 动画渲染器接口（抽象基类）
 * 
 * 定义所有动画渲染器的统一接口，遵循策略模式
 * 继承QOpenGLFunctions以直接调用OpenGL函数
 * 
 * 设计原则：
 * - 单一职责：每个渲染器只负责一种动画效果
 * - 开闭原则：对扩展开放，对修改封闭
 * - 依赖倒置：GameView依赖抽象接口，不依赖具体实现
 */
class IAnimationRenderer : protected QOpenGLFunctions
{
public:
    virtual ~IAnimationRenderer() = default;
    
    /**
     * @brief 初始化OpenGL函数（必须在OpenGL上下文中调用）
     */
    void initialize() {
        initializeOpenGLFunctions();
    }
    
    /**
     * @brief 渲染动画（纯虚函数）
     * 
     * @param animSeq 动画序列（包含所有轮次的消除、下落信息）
     * @param roundIndex 当前轮次索引
     * @param progress 动画进度 (0.0~1.0)
     * @param snapshot 快照地图（用于显示原始状态）
     * @param engineMap 引擎地图（用于获取最终状态）
     * @param gridStartX 网格起始X坐标
     * @param gridStartY 网格起始Y坐标
     * @param cellSize 格子大小
     * @param mapSize 地图大小
     * @param textures 纹理数组
     */
    virtual void render(
        const GameAnimationSequence& animSeq,
        int roundIndex,
        float progress,
        const std::vector<std::vector<Fruit>>& snapshot,
        const std::vector<std::vector<Fruit>>& engineMap,
        float gridStartX,
        float gridStartY,
        float cellSize,
        int mapSize,
        const std::vector<QOpenGLTexture*>& textures
    ) = 0;
    
protected:
    /**
     * @brief 绘制单个水果的辅助函数（供子类使用）
     */
    void drawFruit(
        int row, int col,
        const Fruit& fruit,
        float offsetX, float offsetY,
        float alpha,
        float scale,
        float gridStartX, float gridStartY, float cellSize,
        const std::vector<QOpenGLTexture*>& textures
    );
    
    /**
     * @brief 绘制四边形的辅助函数
     */
    void drawQuad(
        float x, float y, float width, float height
    );
};

#endif // IANIMATIONRENDERER_H
