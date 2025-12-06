#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QTimer>
#include <QMouseEvent>
#include <vector>
#include <memory>
#include "GameEngine.h"

/**
 * @brief 游戏OpenGL渲染视图
 * 
 * 使用OpenGL渲染8×8水果地图，支持纹理显示和动画效果
 */
class GameView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GameView(QWidget *parent = nullptr);
    ~GameView() override;
    
    /**
     * @brief 设置游戏引擎
     */
    void setGameEngine(GameEngine* engine);
    
    /**
     * @brief 更新显示
     */
    void updateDisplay();

protected:
    // OpenGL核心函数
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    // 鼠标事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    /**
     * @brief 动画定时器更新
     */
    void onAnimationTimer();

private:
    /**
     * @brief 加载所有水果纹理
     */
    void loadTextures();
    
    /**
     * @brief 绘制水果网格
     */
    void drawFruitGrid();
    
    /**
     * @brief 绘制单个水果
     */
    void drawFruit(int row, int col, const Fruit& fruit);
    
    /**
     * @brief 绘制选中框
     */
    void drawSelection();
    
    /**
     * @brief 屏幕坐标转换为网格坐标
     */
    bool screenToGrid(int x, int y, int& row, int& col);
    
    /**
     * @brief 绘制四边形
     */
    void drawQuad(float x, float y, float size);
    
    GameEngine* gameEngine_;                              // 游戏引擎
    std::vector<QOpenGLTexture*> fruitTextures_;         // 水果纹理数组
    
    // 网格布局参数
    float gridStartX_;                                   // 网格起始X坐标
    float gridStartY_;                                   // 网格起始Y坐标
    float cellSize_;                                     // 单元格大小
    
    // 选中状态
    int selectedRow_;                                    // 选中的行
    int selectedCol_;                                    // 选中的列
    bool hasSelection_;                                  // 是否有选中
    
    // 动画
    QTimer* animationTimer_;                             // 动画定时器
    int animationFrame_;                                 // 动画帧计数
};

#endif // GAMEVIEW_H
