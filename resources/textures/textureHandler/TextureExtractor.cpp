#include "TextureExtractor.h"
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <cmath>

TextureExtractor::TextureExtractor()
    : rows_(3), cols_(4) {
}

TextureExtractor::~TextureExtractor() {
}

bool TextureExtractor::loadImage(const QString& imagePath) {
    sourceImage_ = QImage(imagePath);
    
    if (sourceImage_.isNull()) {
        return false;
    }
    
    // 确保图像有 Alpha 通道
    if (sourceImage_.format() != QImage::Format_ARGB32 && 
        sourceImage_.format() != QImage::Format_ARGB32_Premultiplied) {
        sourceImage_ = sourceImage_.convertToFormat(QImage::Format_ARGB32);
    }
    
    return true;
}

bool TextureExtractor::extractElements(int rows, int cols) {
    if (sourceImage_.isNull()) {
        return false;
    }
    
    rows_ = rows;
    cols_ = cols;
    
    // 清空之前的数据
    elements_.clear();
    textures_.clear();
    
    // 计算每个网格单元的大小
    int cellWidth = sourceImage_.width() / cols_;
    int cellHeight = sourceImage_.height() / rows_;
    
    // 遍历每个网格单元
    for (int row = 0; row < rows_; ++row) {
        for (int col = 0; col < cols_; ++col) {
            // 提取当前网格单元的图像
            QRect cellRect(col * cellWidth, row * cellHeight, cellWidth, cellHeight);
            QImage cellImage = sourceImage_.copy(cellRect);
            
            // 检测边界框（相对于单元格）
            QRect localBoundingBox = detectBoundingBox(cellImage);
            
            // 转换为全局坐标
            QRect globalBoundingBox = localBoundingBox.translated(cellRect.topLeft());
            
            // 计算中心点（全局坐标）
            QPoint center = calculateCenter(globalBoundingBox);
            
            // 保存元素信息
            ElementInfo info;
            info.boundingBox = globalBoundingBox;
            info.center = center;
            info.gridRow = row;
            info.gridCol = col;
            elements_.push_back(info);
        }
    }
    
    // 计算统一尺寸
    calculateUnifiedSize();
    
    // 根据中心点提取材质
    for (const auto& element : elements_) {
        QImage texture = extractCenteredRegion(sourceImage_, element.center, unifiedSize_);
        textures_.push_back(texture);
    }
    
    return true;
}

int TextureExtractor::saveTextures(const QString& outputDir, const QString& prefix) {
    if (textures_.empty()) {
        return 0;
    }
    
    // 确保输出目录存在
    QDir dir(outputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    int savedCount = 0;
    for (size_t i = 0; i < textures_.size(); ++i) {
        QString filename = QString("%1/%2%3.png").arg(outputDir).arg(prefix).arg(i);
        if (textures_[i].save(filename)) {
            ++savedCount;
        }
    }
    
    return savedCount;
}

QRect TextureExtractor::detectBoundingBox(const QImage& cellImage, int alphaThreshold) {
    if (cellImage.isNull()) {
        return QRect();
    }
    
    int minX = cellImage.width();
    int minY = cellImage.height();
    int maxX = -1;
    int maxY = -1;
    
    // 遍历每个像素，找到非透明区域
    for (int y = 0; y < cellImage.height(); ++y) {
        for (int x = 0; x < cellImage.width(); ++x) {
            QRgb pixel = cellImage.pixel(x, y);
            int alpha = qAlpha(pixel);
            
            // 如果像素不是完全透明
            if (alpha > alphaThreshold) {
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }
    }
    
    // 如果没有找到非透明像素
    if (maxX < 0 || maxY < 0) {
        return QRect(0, 0, 0, 0);
    }
    
    // 返回边界框
    return QRect(minX, minY, maxX - minX + 1, maxY - minY + 1);
}

QPoint TextureExtractor::calculateCenter(const QRect& rect) {
    return rect.center();
}

void TextureExtractor::calculateUnifiedSize() {
    if (elements_.empty()) {
        unifiedSize_ = QSize(0, 0);
        return;
    }
    
    int maxWidth = 0;
    int maxHeight = 0;
    
    // 找到最大的宽度和高度
    for (const auto& element : elements_) {
        maxWidth = std::max(maxWidth, element.boundingBox.width());
        maxHeight = std::max(maxHeight, element.boundingBox.height());
    }
    
    // 使用正方形，取最大边
    int maxSize = std::max(maxWidth, maxHeight);
    
    // 添加一些边距（可选，让材质不会太紧凑）
    maxSize = static_cast<int>(maxSize * 1.05);// 增加10%边距
    
    unifiedSize_ = QSize(maxSize, maxSize);
}

QImage TextureExtractor::extractCenteredRegion(const QImage& source, const QPoint& center, const QSize& size) {
    if (source.isNull() || size.isEmpty()) {
        return QImage();
    }
    
    // 创建一个透明背景的新图像
    QImage result(size, QImage::Format_ARGB32);
    result.fill(Qt::transparent);
    
    // 计算源图像中要提取的区域
    int halfWidth = size.width() / 2;
    int halfHeight = size.height() / 2;
    
    QRect sourceRect(
        center.x() - halfWidth,
        center.y() - halfHeight,
        size.width(),
        size.height()
    );
    
    // 计算源区域和目标图像的交集
    QRect validSourceRect = sourceRect.intersected(source.rect());
    
    if (validSourceRect.isEmpty()) {
        return result;
    }
    
    // 计算在目标图像中的偏移
    int offsetX = validSourceRect.x() - sourceRect.x();
    int offsetY = validSourceRect.y() - sourceRect.y();
    
    // 复制像素
    for (int y = 0; y < validSourceRect.height(); ++y) {
        for (int x = 0; x < validSourceRect.width(); ++x) {
            QRgb pixel = source.pixel(validSourceRect.x() + x, validSourceRect.y() + y);
            result.setPixel(offsetX + x, offsetY + y, pixel);
        }
    }
    
    return result;
}
