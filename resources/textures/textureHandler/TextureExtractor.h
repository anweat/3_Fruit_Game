#ifndef TEXTUREEXTRACTOR_H
#define TEXTUREEXTRACTOR_H

#include <QImage>
#include <QString>
#include <QRect>
#include <QPoint>
#include <vector>

/**
 * @brief 材质提取器 - 用于从4*3网格材质图中提取独立元素
 * 
 * 功能：
 * - 分析透明背景的材质图
 * - 自动检测每个元素的边界框
 * - 计算元素中心点
 * - 提取并保存独立材质
 */
class TextureExtractor {
public:
    /**
     * @brief 元素信息结构体
     */
    struct ElementInfo {
        QRect boundingBox;      // 最小包围框
        QPoint center;          // 中心点
        int gridRow;            // 网格行（0-2）
        int gridCol;            // 网格列（0-3）
    };

    /**
     * @brief 构造函数
     */
    TextureExtractor();

    /**
     * @brief 析构函数
     */
    ~TextureExtractor();

    /**
     * @brief 加载材质图像
     * @param imagePath 图像路径
     * @return 是否加载成功
     */
    bool loadImage(const QString& imagePath);

    /**
     * @brief 提取所有元素的材质
     * @param rows 网格行数（默认3）
     * @param cols 网格列数（默认4）
     * @return 是否提取成功
     */
    bool extractElements(int rows = 3, int cols = 4);

    /**
     * @brief 保存提取的材质到文件
     * @param outputDir 输出目录
     * @param prefix 文件名前缀（默认"fruit_"）
     * @return 成功保存的数量
     */
    int saveTextures(const QString& outputDir, const QString& prefix = "fruit_");

    /**
     * @brief 获取所有元素信息
     * @return 元素信息列表
     */
    const std::vector<ElementInfo>& getElementInfos() const { return elements_; }

    /**
     * @brief 获取统一的元素尺寸（最大包围框）
     * @return 统一尺寸
     */
    QSize getUnifiedSize() const { return unifiedSize_; }

    /**
     * @brief 获取提取的材质图像列表
     * @return 材质图像列表
     */
    const std::vector<QImage>& getTextures() const { return textures_; }

private:
    /**
     * @brief 检测单个网格单元的边界框
     * @param cellImage 单元格图像
     * @param alphaThreshold 透明度阈值（0-255）
     * @return 边界框（相对于单元格的坐标）
     */
    QRect detectBoundingBox(const QImage& cellImage, int alphaThreshold = 10);

    /**
     * @brief 计算边界框的中心点
     * @param rect 边界框
     * @return 中心点
     */
    QPoint calculateCenter(const QRect& rect);

    /**
     * @brief 计算统一的元素尺寸（能包含所有元素的最小正方形）
     */
    void calculateUnifiedSize();

    /**
     * @brief 从图像中心提取指定尺寸的区域
     * @param source 源图像
     * @param center 中心点
     * @param size 目标尺寸
     * @return 提取的图像
     */
    QImage extractCenteredRegion(const QImage& source, const QPoint& center, const QSize& size);

private:
    QImage sourceImage_;                    // 源图像
    std::vector<ElementInfo> elements_;     // 元素信息列表
    std::vector<QImage> textures_;          // 提取的材质列表
    QSize unifiedSize_;                     // 统一尺寸
    int rows_;                              // 网格行数
    int cols_;                              // 网格列数
};

#endif // TEXTUREEXTRACTOR_H
