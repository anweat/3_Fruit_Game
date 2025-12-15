#ifndef SNAPSHOTMANAGER_H
#define SNAPSHOTMANAGER_H

#include <vector>
#include <set>
#include "FruitTypes.h"
#include "AnimationController.h"
#include "GameEngine.h"

/**
 * @brief 快照管理器
 * 
 * 职责：
 * - 保存地图快照
 * - 根据消除消息更新快照
 * - 根据下落消息更新快照
 * - 管理隐藏格子集合
 */
class SnapshotManager
{
public:
    SnapshotManager();
    ~SnapshotManager();
    
    /**
     * @brief 保存当前地图快照
     */
    void saveSnapshot(const std::vector<std::vector<Fruit>>& map);
    
    /**
     * @brief 清除快照
     */
    void clearSnapshot();
    
    /**
     * @brief 获取快照
     */
    const std::vector<std::vector<Fruit>>& getSnapshot() const { return snapshot_; }
    
    /**
     * @brief 快照是否为空
     */
    bool isSnapshotEmpty() const { return snapshot_.empty(); }
    
    /**
     * @brief 应用交换到快照
     */
    void applySwap(int row1, int col1, int row2, int col2);
    
    /**
     * @brief 应用消除到快照
     */
    void applyElimination(const GameAnimationSequence& animSeq, int roundIndex);
    
    /**
     * @brief 应用下落到快照（从动画数据获取类型，不依赖engineMap）
     */
    void applyFall(const GameAnimationSequence& animSeq, int roundIndex);
    
    /**
     * @brief 更新隐藏格子集合
     */
    void updateHiddenCells(const GameAnimationSequence& animSeq, 
                           int roundIndex, 
                           AnimPhase phase);
    
    /**
     * @brief 清除隐藏格子集合
     */
    void clearHiddenCells();
    
    /**
     * @brief 检查格子是否隐藏
     */
    bool isCellHidden(int row, int col) const;
    
    /**
     * @brief 设置所有格子隐藏（用于重排动画）
     */
    void hideAllCells();
    
private:
    std::vector<std::vector<Fruit>> snapshot_;       ///< 地图快照
    std::set<std::pair<int, int>> hiddenCells_;      ///< 隐藏格子集合
};

#endif // SNAPSHOTMANAGER_H
