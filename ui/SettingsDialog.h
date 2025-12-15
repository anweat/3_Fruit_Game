#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QGroupBox>

/**
 * @brief 游戏设置对话框
 * 
 * 使用可滚动框架设计，支持后续添加更多设置项
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() = default;

    /**
     * @brief 获取休闲模式地图大小设置
     * @return 地图大小（8-60）
     */
    int getCasualMapSize() const { return casualMapSize_; }

    /**
     * @brief 设置休闲模式地图大小
     * @param size 地图大小
     */
    void setCasualMapSize(int size);

signals:
    /**
     * @brief 设置已保存信号
     */
    void settingsSaved();

private slots:
    /**
     * @brief 保存设置按钮点击
     */
    void onSaveClicked();

    /**
     * @brief 取消按钮点击
     */
    void onCancelClicked();

    /**
     * @brief 地图大小滑动条值改变
     */
    void onMapSizeChanged(int value);

private:
    /**
     * @brief 初始化UI
     */
    void initUI();

    /**
     * @brief 创建游戏设置组
     */
    QWidget* createGameSettingsGroup();

    /**
     * @brief 加载设置
     */
    void loadSettings();

    /**
     * @brief 保存设置
     */
    void saveSettings();

    // UI 组件
    QScrollArea* scrollArea_;
    QWidget* contentWidget_;
    QVBoxLayout* mainLayout_;
    
    // 游戏设置
    QSlider* mapSizeSlider_;
    QLabel* mapSizeValueLabel_;
    
    // 按钮
    QPushButton* saveButton_;
    QPushButton* cancelButton_;

    // 设置值
    int casualMapSize_;  ///< 休闲模式地图大小（默认8）
};

#endif // SETTINGSDIALOG_H
