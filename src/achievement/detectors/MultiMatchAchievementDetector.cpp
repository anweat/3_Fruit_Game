#include "MultiMatchAchievementDetector.h"

void MultiMatchAchievementDetector::detect(
    const GameDataSnapshot& snapshot,
    std::function<void(const QString&, int, int)> callback
)
{
    int matchSize = snapshot.lastMatchSize;
    
    // 📌 修复问题 #1: 只在元素相同时计数N消
    if (!snapshot.lastMatchSameElement) {
        return;  // 如果不是全相同元素，不计为多消成就
    }
    
    // 首次N消
    if (matchSize >= 4) {
        callback("ach_match4_first", 1, 1);
        totalMatch4_++;
    }
    if (matchSize >= 5) {
        callback("ach_match5_first", 1, 1);
        totalMatch5_++;
    }
    if (matchSize >= 6) {
        callback("ach_match6", matchSize, 6);
        totalMatch6_++;
    }
    if (matchSize >= 8) {
        callback("ach_match8", matchSize, 8);
        totalMatch8_++;
    }
    
    // 累计N消次数
    callback("ach_match4_100", totalMatch4_, 100);
    callback("ach_match5_50", totalMatch5_, 50);
    callback("ach_match6_20", totalMatch6_, 20);
    callback("ach_match8_10", totalMatch8_, 10);
    
    // 单局N+消次数
    if (matchSize >= 5) {
        callback("ach_match5plus_3", snapshot.match5PlusCount, 3);
    }
    if (matchSize >= 6) {
        callback("ach_match6plus_5", snapshot.match6PlusCount, 5);
    }
}
