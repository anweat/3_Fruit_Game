#ifndef STYLELOADER_H
#define STYLELOADER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QApplication>

/**
 * @brief 样式加载器类
 * 
 * 用于加载和管理应用程序的QSS样式表
 */
class StyleLoader
{
public:
    /**
     * @brief 加载所有样式表
     * @return 合并后的完整样式表字符串
     */
    static QString loadAllStyles()
    {
        QString combinedStyle;
        
        // 按顺序加载各个样式文件
        combinedStyle += loadStyleFile(":/styles/main.qss");
        combinedStyle += loadStyleFile(":/styles/scrollbar.qss");
        combinedStyle += loadStyleFile(":/styles/buttons.qss");
        combinedStyle += loadStyleFile(":/styles/login.qss");
        combinedStyle += loadStyleFile(":/styles/game.qss");
        combinedStyle += loadStyleFile(":/styles/achievement.qss");
        combinedStyle += loadStyleFile(":/styles/settings.qss");
        
        return combinedStyle;
    }
    
    /**
     * @brief 应用样式到应用程序
     */
    static void applyStyles()
    {
        QString styles = loadAllStyles();
        if (!styles.isEmpty()) {
            qApp->setStyleSheet(styles);
            qDebug() << "✅ Styles applied successfully";
        } else {
            qWarning() << "⚠️ No styles loaded";
        }
    }

private:
    /**
     * @brief 加载单个样式文件
     * @param filePath 样式文件路径（支持资源路径和文件系统路径）
     * @return 样式表字符串
     */
    static QString loadStyleFile(const QString& filePath)
    {
        QFile file(filePath);
        
        // 如果资源路径加载失败，尝试从文件系统加载
        if (!file.exists()) {
            QString fsPath = filePath;
            fsPath.replace(":/", "resources/");
            file.setFileName(fsPath);
        }
        
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            qWarning() << "Failed to load style file:" << filePath;
            return QString();
        }
        
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        QString content = stream.readAll();
        file.close();
        
        qDebug() << "Loaded style file:" << filePath;
        return content + "\n";
    }
};

#endif // STYLELOADER_H
