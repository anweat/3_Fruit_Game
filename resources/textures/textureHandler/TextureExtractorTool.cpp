#include "TextureExtractor.h"
#include <QCoreApplication>
#include <QDebug>

/**
 * @brief 材质提取工具使用示例
 * 
 * 使用方法：
 * 1. 直接运行：提取到默认输出目录 resources/textures/
 * 2. 命令行参数：TextureExtractorTool <输入图片> <输出目录> <前缀>
 */
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // 默认参数
    QString inputPath = "D:/codeproject/DataSructure/06-General/Game_Qoder_2/resources/texture.png";
    QString outputDir = "resources/textures";
    QString prefix = "fruit_";
    
    // 解析命令行参数
    if (argc > 1) {
        inputPath = argv[1];
    }
    if (argc > 2) {
        outputDir = argv[2];
    }
    if (argc > 3) {
        prefix = argv[3];
    }
    
    qDebug() << "=== 材质提取工具 ===";
    qDebug() << "输入图像:" << inputPath;
    qDebug() << "输出目录:" << outputDir;
    qDebug() << "文件前缀:" << prefix;
    qDebug() << "";
    
    // 创建提取器
    TextureExtractor extractor;
    
    // 步骤1：加载图像
    qDebug() << "步骤1: 加载图像...";
    if (!extractor.loadImage(inputPath)) {
        qDebug() << "错误: 无法加载图像" << inputPath;
        return 1;
    }
    qDebug() << "✓ 图像加载成功";
    qDebug() << "";
    
    // 步骤2：提取元素（4列 x 3行）
    qDebug() << "步骤2: 分析并提取元素...";
    if (!extractor.extractElements(3, 4)) {
        qDebug() << "错误: 提取失败";
        return 1;
    }
    qDebug() << "✓ 提取成功";
    qDebug() << "";
    
    // 步骤3：显示元素信息
    qDebug() << "步骤3: 元素信息:";
    const auto& elements = extractor.getElementInfos();
    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& elem = elements[i];
        qDebug() << QString("  元素 %1 [行%2, 列%3]:").arg(i).arg(elem.gridRow).arg(elem.gridCol);
        qDebug() << QString("    边界框: (%1, %2, %3x%4)")
                    .arg(elem.boundingBox.x())
                    .arg(elem.boundingBox.y())
                    .arg(elem.boundingBox.width())
                    .arg(elem.boundingBox.height());
        qDebug() << QString("    中心点: (%1, %2)")
                    .arg(elem.center.x())
                    .arg(elem.center.y());
    }
    qDebug() << "";
    
    // 显示统一尺寸
    QSize unifiedSize = extractor.getUnifiedSize();
    qDebug() << QString("统一材质尺寸: %1x%2").arg(unifiedSize.width()).arg(unifiedSize.height());
    qDebug() << "";
    
    // 步骤4：保存材质
    qDebug() << "步骤4: 保存材质到文件...";
    int savedCount = extractor.saveTextures(outputDir, prefix);
    qDebug() << QString("✓ 成功保存 %1 个材质").arg(savedCount);
    qDebug() << "";
    
    qDebug() << "=== 完成 ===";
    qDebug() << QString("所有材质已保存到: %1").arg(outputDir);
    qDebug() << QString("文件命名格式: %1<索引>.png (例如: %10.png, %11.png, ...)").arg(prefix).arg(prefix).arg(prefix);
    
    return 0;
}
