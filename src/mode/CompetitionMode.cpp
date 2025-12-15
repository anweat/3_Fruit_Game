#include "CompetitionMode.h"
#include <QDebug>

CompetitionMode::CompetitionMode(QObject* parent)
    : QObject(parent)
    , timer_(new QTimer(this))
    , remainingTime_(0)
    , isRunning_(false)
    , isPaused_(false)
{
    // 连接定时器信号
    connect(timer_, &QTimer::timeout, this, &CompetitionMode::onTimerTick);
}

CompetitionMode::~CompetitionMode()
{
    if (timer_) {
        timer_->stop();
    }
}

void CompetitionMode::setConfig(const CompetitionConfig& config)
{
    config_ = config;
    remainingTime_ = static_cast<int>(config_.duration);
}

void CompetitionMode::startCompetition()
{
    if (isRunning_) {
        qWarning() << "Competition already running!";
        return;
    }
    
    remainingTime_ = static_cast<int>(config_.duration);
    isRunning_ = true;
    isPaused_ = false;
    
    // 每秒触发一次
    timer_->start(1000);
    
    qDebug() << "Competition started! Duration:" << remainingTime_ << "seconds";
    emit competitionStarted();
    emit timeUpdated(remainingTime_);
}

void CompetitionMode::pauseCompetition()
{
    if (!isRunning_ || isPaused_) {
        return;
    }
    
    isPaused_ = true;
    timer_->stop();
    
    qDebug() << "Competition paused. Remaining:" << remainingTime_ << "seconds";
}

void CompetitionMode::resumeCompetition()
{
    if (!isRunning_ || !isPaused_) {
        return;
    }
    
    isPaused_ = false;
    timer_->start(1000);
    
    qDebug() << "Competition resumed. Remaining:" << remainingTime_ << "seconds";
}

void CompetitionMode::endCompetition()
{
    if (!isRunning_) {
        return;
    }
    
    isRunning_ = false;
    isPaused_ = false;
    timer_->stop();
    
    qDebug() << "Competition ended normally!";
    emit competitionEnded();
}

void CompetitionMode::abandonCompetition()
{
    if (!isRunning_) {
        return;
    }
    
    isRunning_ = false;
    isPaused_ = false;
    timer_->stop();
    
    qDebug() << "Competition abandoned!";
    emit competitionAbandoned();
}

void CompetitionMode::onTimerTick()
{
    if (remainingTime_ > 0) {
        remainingTime_--;
        emit timeUpdated(remainingTime_);
        
        // 最后10秒警告
        if (remainingTime_ <= 10 && remainingTime_ > 0) {
            qDebug() << "⏰ Warning:" << remainingTime_ << "seconds left!";
        }
    }
    
    if (remainingTime_ <= 0) {
        endCompetition();
    }
}

QString CompetitionMode::getDurationString(CompetitionDuration duration)
{
    switch (duration) {
        case CompetitionDuration::SECONDS_60:
            return "60秒赛";
        case CompetitionDuration::SECONDS_120:
            return "120秒赛";
        case CompetitionDuration::SECONDS_180:
            return "180秒赛";
        default:
            return "未知";
    }
}
