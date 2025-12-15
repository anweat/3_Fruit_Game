#include "SettingsDialog.h"
#include <QSettings>
#include <QApplication>

/**
 * @brief 构造函数
 */
SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , casualMapSize_(8)  // 默认值
{
    initUI();
    loadSettings();
    
    setWindowTitle("游戏设置");
    setModal(true);
    resize(500, 600);
}

/**
 * @brief 初始化UI
 */
void SettingsDialog::initUI()
{
    // 主布局
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setSpacing(15);
    mainLayout_->setContentsMargins(20, 20, 20, 20);

    // 创建滚动区域
    scrollArea_ = new QScrollArea(this);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_->setObjectName("settingsScrollArea");

    // 内容容器
    contentWidget_ = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget_);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(10, 10, 10, 10);

    // 添加设置组
    contentLayout->addWidget(createGameSettingsGroup());
    
    // 弹性空间（占据剩余空间）
    contentLayout->addStretch();

    scrollArea_->setWidget(contentWidget_);
    mainLayout_->addWidget(scrollArea_);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    saveButton_ = new QPushButton("保存设置", this);
    saveButton_->setObjectName("saveButton");
    saveButton_->setMinimumWidth(120);
    saveButton_->setMinimumHeight(40);
    connect(saveButton_, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);
    
    cancelButton_ = new QPushButton("取消", this);
    cancelButton_->setObjectName("cancelButton");
    cancelButton_->setMinimumWidth(120);
    cancelButton_->setMinimumHeight(40);
    connect(cancelButton_, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addSpacing(15);
    buttonLayout->addWidget(cancelButton_);
    buttonLayout->addStretch();
    
    mainLayout_->addLayout(buttonLayout);
}

/**
 * @brief 创建游戏设置组
 */
QWidget* SettingsDialog::createGameSettingsGroup()
{
    QGroupBox* groupBox = new QGroupBox("游戏设置");
    groupBox->setObjectName("settingsGroupBox");
    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 25, 20, 20);

    // ========== 休闲模式地图大小 ==========
    QLabel* mapSizeLabel = new QLabel("休闲模式地图大小：");
    mapSizeLabel->setObjectName("settingLabel");
    layout->addWidget(mapSizeLabel);

    // 滑动条和值显示
    QHBoxLayout* sliderLayout = new QHBoxLayout();
    
    mapSizeSlider_ = new QSlider(Qt::Horizontal);
    mapSizeSlider_->setObjectName("mapSizeSlider");
    mapSizeSlider_->setMinimum(8);
    mapSizeSlider_->setMaximum(60);
    mapSizeSlider_->setValue(8);
    mapSizeSlider_->setTickPosition(QSlider::TicksBelow);
    mapSizeSlider_->setTickInterval(4);
    connect(mapSizeSlider_, &QSlider::valueChanged, this, &SettingsDialog::onMapSizeChanged);
    
    mapSizeValueLabel_ = new QLabel("8 × 8");
    mapSizeValueLabel_->setObjectName("settingValueLabel");
    mapSizeValueLabel_->setMinimumWidth(80);
    mapSizeValueLabel_->setAlignment(Qt::AlignCenter);
    
    sliderLayout->addWidget(mapSizeSlider_);
    sliderLayout->addWidget(mapSizeValueLabel_);
    layout->addLayout(sliderLayout);

    // 说明文字
    QLabel* hintLabel = new QLabel("提示：较大的地图会增加游戏难度和趣味性");
    hintLabel->setObjectName("settingHintLabel");
    hintLabel->setWordWrap(true);
    layout->addWidget(hintLabel);

    return groupBox;
}

/**
 * @brief 地图大小滑动条值改变
 */
void SettingsDialog::onMapSizeChanged(int value)
{
    casualMapSize_ = value;
    mapSizeValueLabel_->setText(QString("%1 × %1").arg(value));
}

/**
 * @brief 设置休闲模式地图大小
 */
void SettingsDialog::setCasualMapSize(int size)
{
    if (size < 8) size = 8;
    if (size > 60) size = 60;
    
    casualMapSize_ = size;
    mapSizeSlider_->setValue(size);
    mapSizeValueLabel_->setText(QString("%1 × %1").arg(size));
}

/**
 * @brief 加载设置
 */
void SettingsDialog::loadSettings()
{
    QSettings settings("FruitCrush", "GameSettings");
    
    // 加载休闲模式地图大小（默认8）
    int mapSize = settings.value("casual/mapSize", 8).toInt();
    setCasualMapSize(mapSize);
}

/**
 * @brief 保存设置
 */
void SettingsDialog::saveSettings()
{
    QSettings settings("FruitCrush", "GameSettings");
    
    // 保存休闲模式地图大小
    settings.setValue("casual/mapSize", casualMapSize_);
    
    settings.sync();
}

/**
 * @brief 保存设置按钮点击
 */
void SettingsDialog::onSaveClicked()
{
    saveSettings();
    emit settingsSaved();
    accept();
}

/**
 * @brief 取消按钮点击
 */
void SettingsDialog::onCancelClicked()
{
    reject();
}
