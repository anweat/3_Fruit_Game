#include "SnapshotManager.h"

SnapshotManager::SnapshotManager()
{
}

SnapshotManager::~SnapshotManager()
{
}

void SnapshotManager::saveSnapshot(const std::vector<std::vector<Fruit>>& map)
{
    snapshot_ = map;
}

void SnapshotManager::clearSnapshot()
{
    snapshot_.clear();
}

void SnapshotManager::applySwap(int row1, int col1, int row2, int col2)
{
    if (snapshot_.empty()) return;
    
    if (row1 >= 0 && row1 < MAP_SIZE && col1 >= 0 && col1 < MAP_SIZE &&
        row2 >= 0 && row2 < MAP_SIZE && col2 >= 0 && col2 < MAP_SIZE) {
        std::swap(snapshot_[row1][col1], snapshot_[row2][col2]);
    }
}

void SnapshotManager::applyElimination(const GameAnimationSequence& animSeq, int roundIndex)
{
    if (snapshot_.empty()) return;
    
    if (roundIndex < 0 || roundIndex >= static_cast<int>(animSeq.rounds.size())) {
        return;
    }
    
    const auto& round = animSeq.rounds[roundIndex];
    
    // 清空被消除的格子
    for (const auto& pos : round.elimination.positions) {
        int r = pos.first;
        int c = pos.second;
        if (r >= 0 && r < MAP_SIZE && c >= 0 && c < MAP_SIZE) {
            snapshot_[r][c].type = FruitType::EMPTY;
            snapshot_[r][c].special = SpecialType::NONE;
        }
    }
}

void SnapshotManager::applyFall(const GameAnimationSequence& animSeq, 
                                 int roundIndex,
                                 const std::vector<std::vector<Fruit>>& engineMap)
{
    if (snapshot_.empty()) return;
    
    if (roundIndex < 0 || roundIndex >= static_cast<int>(animSeq.rounds.size())) {
        return;
    }
    
    const auto& round = animSeq.rounds[roundIndex];
    
    // 应用下落移动
    for (const auto& move : round.fall.moves) {
        int toR = move.toRow;
        int toC = move.toCol;
        
        if (toR >= 0 && toR < MAP_SIZE && toC >= 0 && toC < MAP_SIZE) {
            // 从引擎获取最终状态
            snapshot_[toR][toC] = engineMap[toR][toC];
        }
    }
    
    // 应用新生成的水果
    for (const auto& nf : round.fall.newFruits) {
        int r = nf.row;
        int c = nf.col;
        if (r >= 0 && r < MAP_SIZE && c >= 0 && c < MAP_SIZE) {
            snapshot_[r][c] = engineMap[r][c];
        }
    }
}

void SnapshotManager::updateHiddenCells(const GameAnimationSequence& animSeq, 
                                         int roundIndex, 
                                         AnimPhase phase)
{
    hiddenCells_.clear();
    
    // 交换阶段：隐藏交换的两个格子
    if (phase == AnimPhase::SWAPPING) {
        hiddenCells_.insert({animSeq.swap.row1, animSeq.swap.col1});
        hiddenCells_.insert({animSeq.swap.row2, animSeq.swap.col2});
        return;
    }
    
    if (roundIndex < 0 || roundIndex >= static_cast<int>(animSeq.rounds.size())) {
        return;
    }
    
    const auto& round = animSeq.rounds[roundIndex];
    
    // 消除阶段：隐藏被消除的格子
    if (phase == AnimPhase::ELIMINATING) {
        for (const auto& pos : round.elimination.positions) {
            hiddenCells_.insert(pos);
        }
    }
    
    // 下落阶段：隐藏参与下落的目标位置和新生成位置
    if (phase == AnimPhase::FALLING) {
        // 下落的元素：隐藏目标位置
        for (const auto& move : round.fall.moves) {
            hiddenCells_.insert({move.toRow, move.toCol});
        }
        
        // 新生成的元素
        for (const auto& nf : round.fall.newFruits) {
            hiddenCells_.insert({nf.row, nf.col});
        }
    }
}

void SnapshotManager::clearHiddenCells()
{
    hiddenCells_.clear();
}

bool SnapshotManager::isCellHidden(int row, int col) const
{
    return hiddenCells_.count({row, col}) > 0;
}

void SnapshotManager::hideAllCells()
{
    hiddenCells_.clear();
    for (int row = 0; row < MAP_SIZE; ++row) {
        for (int col = 0; col < MAP_SIZE; ++col) {
            hiddenCells_.insert({row, col});
        }
    }
}
