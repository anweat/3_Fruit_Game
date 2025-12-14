#include "AchievementNotificationWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QMutexLocker>

AchievementNotificationWidget::AchievementNotificationWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
    , isAnimating_(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_NoSystemBackground);
    setFixedSize(WIDGET_WIDTH, WIDGET_HEIGHT);
    
    setupUI();
    setupAnimations();
}

AchievementNotificationWidget::~AchievementNotificationWidget()
{
    qDebug() << "[Notification] Destructor";
}

void AchievementNotificationWidget::setupUI()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(12);
    
    // Icon
    iconLabel_ = new QLabel(this);
    iconLabel_->setFixedSize(80, 80);
    iconLabel_->setAlignment(Qt::AlignCenter);
    iconLabel_->setStyleSheet("font-size: 48px; font-weight: bold;");
    mainLayout->addWidget(iconLabel_);
    
    // Info
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    
    titleLabel_ = new QLabel(this);
    titleLabel_->setStyleSheet("font-size: 16px; font-weight: bold; color: white;");
    titleLabel_->setWordWrap(false);
    infoLayout->addWidget(titleLabel_);
    
    rarityLabel_ = new QLabel(this);
    rarityLabel_->setStyleSheet("font-size: 12px; color: #E0E0E0;");
    infoLayout->addWidget(rarityLabel_);
    
    rewardLabel_ = new QLabel(this);
    rewardLabel_->setStyleSheet("font-size: 13px; font-weight: bold; color: #FFD700;");
    infoLayout->addWidget(rewardLabel_);
    
    infoLayout->addStretch();
    mainLayout->addLayout(infoLayout);
}

void AchievementNotificationWidget::setupAnimations()
{
    slideAnimation_ = new QPropertyAnimation(this, "pos");
    slideAnimation_->setDuration(SLIDE_DURATION);
    slideAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    connect(slideAnimation_, &QPropertyAnimation::finished, this, &AchievementNotificationWidget::onSlideInFinished);
    
    fadeAnimation_ = new QPropertyAnimation(this, "windowOpacity");
    fadeAnimation_->setDuration(FADE_DURATION);
    fadeAnimation_->setStartValue(1.0);
    fadeAnimation_->setEndValue(0.0);
    fadeAnimation_->setEasingCurve(QEasingCurve::InCubic);
    connect(fadeAnimation_, &QPropertyAnimation::finished, this, &AchievementNotificationWidget::onFadeOutFinished);
    
    displayTimer_ = new QTimer(this);
    displayTimer_->setSingleShot(true);
    connect(displayTimer_, &QTimer::timeout, this, &AchievementNotificationWidget::onDisplayTimeout);
}

void AchievementNotificationWidget::enqueueNotification(const AchievementNotification& notification)
{
    QMutexLocker locker(&queueMutex_);
    notificationQueue_.enqueue(notification);
    
    if (!isAnimating_) {
        QTimer::singleShot(0, this, &AchievementNotificationWidget::processQueue);
    }
}

void AchievementNotificationWidget::processQueue()
{
    if (isAnimating_) {
        return;
    }
    
    QMutexLocker locker(&queueMutex_);
    
    if (notificationQueue_.isEmpty()) {
        return;
    }
    
    currentDisplaying_ = notificationQueue_.dequeue();
    locker.unlock();
    
    showCurrentNotification();
}

void AchievementNotificationWidget::showCurrentNotification()
{
    isAnimating_ = true;
    
    // Extract icon from name if it contains emoji
    QString displayName = currentDisplaying_.achievementName;
    QString icon = currentDisplaying_.icon;
    
    // If no explicit icon, try to extract from name
    if (icon.isEmpty() && !displayName.isEmpty()) {
        // Name format: "emoji name" - extract emoji
        if (displayName.contains(" ")) {
            icon = displayName.split(" ").first();
            displayName = displayName.split(" ", Qt::SkipEmptyParts).mid(1).join(" ");
        }
    }
    
    // Update labels
    iconLabel_->setText(icon.isEmpty() ? "A" : icon);
    titleLabel_->setText(displayName);
    rarityLabel_->setText(getRarityName(currentDisplaying_.rarity));
    rewardLabel_->setText(QString("+%1 pts").arg(currentDisplaying_.reward));
    
    updateLabelStyles();
    setWindowOpacity(1.0);
    
    // Force repaint
    repaint();
    
    startSlideIn();
}

void AchievementNotificationWidget::startSlideIn()
{
    QWidget* parent = parentWidget();
    if (!parent) {
        qWarning() << "[Notification] No parent!";
        isAnimating_ = false;
        return;
    }
    
    QPoint parentGlobalPos = parent->mapToGlobal(QPoint(0, 0));
    int parentWidth = parent->width();
    
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    
    // Position: top-right
    int targetX = parentGlobalPos.x() + parentWidth - WIDGET_WIDTH - 20;
    int targetY = parentGlobalPos.y() + 100;
    
    // Clamp to screen
    targetX = qBound(screenGeometry.left() + 10, targetX, screenGeometry.right() - WIDGET_WIDTH - 10);
    targetY = qBound(screenGeometry.top() + 10, targetY, screenGeometry.bottom() - WIDGET_HEIGHT - 10);
    
    int startX = targetX + WIDGET_WIDTH + 100;
    int startY = targetY;
    
    move(startX, startY);
    show();
    raise();
    repaint();
    
    slideAnimation_->setStartValue(QPoint(startX, startY));
    slideAnimation_->setEndValue(QPoint(targetX, targetY));
    slideAnimation_->start();
}

void AchievementNotificationWidget::onSlideInFinished()
{
    displayTimer_->start(DISPLAY_DURATION);
}

void AchievementNotificationWidget::onDisplayTimeout()
{
    startFadeOut();
}

void AchievementNotificationWidget::startFadeOut()
{
    fadeAnimation_->start();
}

void AchievementNotificationWidget::onFadeOutFinished()
{
    hide();
    isAnimating_ = false;
    
    // Process next
    QTimer::singleShot(100, this, &AchievementNotificationWidget::processQueue);
}

void AchievementNotificationWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    if (currentDisplaying_.achievementId.isEmpty()) {
        return;
    }
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QColor bgColor = QColor(getRarityColor(currentDisplaying_.rarity));
    bgColor.setAlpha(220);
    
    QPainterPath path;
    path.addRoundedRect(0, 0, width(), height(), 10, 10);
    
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, bgColor);
    gradient.setColorAt(1, bgColor.darker(130));
    
    painter.fillPath(path, gradient);
    
    QPen pen(bgColor.lighter(150), 2);
    painter.setPen(pen);
    painter.drawPath(path);
}

QString AchievementNotificationWidget::getRarityColor(AchievementRarity rarity)
{
    switch (rarity) {
        case AchievementRarity::BRONZE: return "#8B6914";
        case AchievementRarity::SILVER: return "#7B8794";
        case AchievementRarity::GOLD:   return "#C79C0D";
        case AchievementRarity::DIAMOND: return "#3B6BA3";
        default: return "#555555";
    }
}

QString AchievementNotificationWidget::getRarityName(AchievementRarity rarity)
{
    switch (rarity) {
        case AchievementRarity::BRONZE: return "[Bronze Achievement]";
        case AchievementRarity::SILVER: return "[Silver Achievement]";
        case AchievementRarity::GOLD:   return "[Gold Achievement]";
        case AchievementRarity::DIAMOND: return "[Diamond Achievement]";
        default: return "[Unknown]";
    }
}

QString AchievementNotificationWidget::getRarityIcon(AchievementRarity rarity)
{
    switch (rarity) {
        case AchievementRarity::BRONZE: return "B";
        case AchievementRarity::SILVER: return "S";
        case AchievementRarity::GOLD:   return "G";
        case AchievementRarity::DIAMOND: return "D";
        default: return "?";
    }
}

void AchievementNotificationWidget::updateLabelStyles()
{
    QColor color;
    switch (currentDisplaying_.rarity) {
        case AchievementRarity::BRONZE:
            color = QColor("#FFD9A8");
            titleLabel_->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFD9A8;");
            rarityLabel_->setStyleSheet("font-size: 12px; color: #DBAF5A;");
            break;
        case AchievementRarity::SILVER:
            color = QColor("#E8E8E8");
            titleLabel_->setStyleSheet("font-size: 16px; font-weight: bold; color: #E8E8E8;");
            rarityLabel_->setStyleSheet("font-size: 12px; color: #B8B8B8;");
            break;
        case AchievementRarity::GOLD:
            color = QColor("#FFE64F");
            titleLabel_->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFE64F;");
            rarityLabel_->setStyleSheet("font-size: 12px; color: #FFB700;");
            break;
        case AchievementRarity::DIAMOND:
            color = QColor("#87CEEB");
            titleLabel_->setStyleSheet("font-size: 16px; font-weight: bold; color: #87CEEB;");
            rarityLabel_->setStyleSheet("font-size: 12px; color: #4A90E2;");
            break;
        default:
            break;
    }
}
