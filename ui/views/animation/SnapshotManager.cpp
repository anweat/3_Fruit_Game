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
    int mapSize = snapshot_.size();
    
    if (row1 >= 0 && row1 < mapSize && col1 >= 0 && col1 < mapSize &&
        row2 >= 0 && row2 < mapSize && col2 >= 0 && col2 < mapSize) {
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
        if (r >= 0 && r < static_cast<int>(snapshot_.size()) && c >= 0 && c < static_cast<int>(snapshot_.size())) {
            snapshot_[r][c].type = FruitType::EMPTY;
            snapshot_[r][c].special = SpecialType::NONE;
        }
    }
}

void SnapshotManager::applyFall(const GameAnimationSequence& animSeq, int roundIndex)
{
    if (snapshot_.empty()) return;
    
    if (roundIndex < 0 || roundIndex >= static_cast<int>(animSeq.rounds.size())) {
        return;
    }
    
    const auto& round = animSeq.rounds[roundIndex];
    
    // 关键修复：使用FallMove中记录的类型信息，而不是从snapshot读取
    // 1. 先清空所有源位置
    for (const auto& move : round.fall.moves) {
        int fromR = move.fromRow;
        int fromC = move.fromCol;
        if (fromR >= 0 && fromR < static_cast<int>(snapshot_.size()) && 
            fromC >= 0 && fromC < static_cast<int>(snapshot_.size())) {
            snapshot_[fromR][fromC].type = FruitType::EMPTY;
            snapshot_[fromR][fromC].special = SpecialType::NONE;
        }
    }
    
    // 2. 应用移动到目标位置（使用FallMove中的类型）
    for (const auto& move : round.fall.moves) {
        int toR = move.toRow;
        int toC = move.toCol;
        if (toR >= 0 && toR < static_cast<int>(snapshot_.size()) && 
            toC >= 0 && toC < static_cast<int>(snapshot_.size())) {
            snapshot_[toR][toC].type = move.type;
            snapshot_[toR][toC].special = move.special;
            snapshot_[toR][toC].isMatched = false;
        }
    }
    
    // 4. 新生成的水果直接使用动画数据中的类型信息（而不是从engineMap读取）
    for (const auto& nf : round.fall.newFruits) {
        int r = nf.row;
        int c = nf.col;
        if (r >= 0 && r < static_cast<int>(snapshot_.size()) && 
            c >= 0 && c < static_cast<int>(snapshot_.size())) {
            // 使用动画数据中记录的类型，而不是engineMap（engineMap是最终状态）
            snapshot_[r][c].type = nf.type;
            snapshot_[r][c].special = nf.special;
            snapshot_[r][c].isMatched = false;
        }
    }
}

void SnapshotManager::updateHiddenCells(const GameAnimationSequence& animSeq, 
                                         int roundIndex, 
                                         AnimPhase phase)
{
    hiddenCells_.clear();
    
    // �����׶Σ����ؽ�������������
    if (phase == AnimPhase::SWAPPING) {
        hiddenCells_.insert({animSeq.swap.row1, animSeq.swap.col1});
        hiddenCells_.insert({animSeq.swap.row2, animSeq.swap.col2});
        return;
    }
    
    if (roundIndex < 0 || roundIndex >= static_cast<int>(animSeq.rounds.size())) {
        return;
    }
    
    const auto& round = animSeq.rounds[roundIndex];
    
    // �����׶Σ����ر������ĸ���
    if (phase == AnimPhase::ELIMINATING) {
        for (const auto& pos : round.elimination.positions) {
            hiddenCells_.insert(pos);
        }
    }
    
    // 下落阶段：隐藏snapshot中正在移动的源位置
    if (phase == AnimPhase::FALLING) {
        // 🔧 关键修复：隐藏源位置（snapshot中的原始位置），而不是目标位置
        // 因为动画渲染器会在插值位置绘制水果，如果不隐藏源位置会造成重影
        for (const auto& move : round.fall.moves) {
            // 隐藏snapshot中的源位置
            hiddenCells_.insert({move.fromRow, move.fromCol});
        }
        
        // 新生成的水果在engineMap中，snapshot中对应位置应该是EMPTY
        // 但为了安全起见也隐藏，避免渲染旧状态
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
    for (int row = 0; row < static_cast<int>(snapshot_.size()); ++row) {
        for (int col = 0; col < static_cast<int>(snapshot_.size()); ++col) {
            hiddenCells_.insert({row, col});
        }
    }
}
