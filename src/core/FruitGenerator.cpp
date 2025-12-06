#include "FruitGenerator.h"
#include "MatchDetector.h"
#include <chrono>
#include <algorithm>

FruitGenerator::FruitGenerator() {
    // 使用当前时间作为随机种子
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    rng_.seed(static_cast<unsigned int>(seed));
}

FruitGenerator::~FruitGenerator() {
    // 默认析构函数
}

FruitType FruitGenerator::generateRandomFruit() {
    // 生成0-5之间的随机数，对应6种水果类型
    std::uniform_int_distribution<int> dist(0, FRUIT_TYPE_COUNT - 1);
    return static_cast<FruitType>(dist(rng_));
}

FruitType FruitGenerator::generateRandomFruit(const std::vector<FruitType>& excludeTypes) {
    // 如果排除所有类型，返回随机水果
    if (excludeTypes.size() >= FRUIT_TYPE_COUNT) {
        return generateRandomFruit();
    }
    
    FruitType fruit;
    bool valid = false;
    int attempts = 0;
    const int maxAttempts = 100; // 防止无限循环
    
    while (!valid && attempts < maxAttempts) {
        fruit = generateRandomFruit();
        valid = true;
        
        // 检查是否在排除列表中
        for (const auto& excludeType : excludeTypes) {
            if (fruit == excludeType) {
                valid = false;
                break;
            }
        }
        attempts++;
    }
    
    return fruit;
}

void FruitGenerator::initializeMap(std::vector<std::vector<Fruit>>& map) {
    // 初始化地图大小为8×8
    map.resize(MAP_SIZE);
    for (int i = 0; i < MAP_SIZE; i++) {
        map[i].resize(MAP_SIZE);
    }
    
    // 从上到下，从左到右填充水果
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            // 生成安全的水果类型(不会立即形成三连)
            FruitType fruitType = generateSafeFruit(map, row, col);
            
            // 创建水果对象
            map[row][col] = Fruit(fruitType, row, col);
        }
    }
}

FruitType FruitGenerator::generateSafeFruit(const std::vector<std::vector<Fruit>>& map, 
                                            int row, int col) {
    // 生成候选水果类型列表
    std::vector<FruitType> candidates;
    for (int i = 0; i < FRUIT_TYPE_COUNT; i++) {
        candidates.push_back(static_cast<FruitType>(i));
    }
    
    // 随机打乱候选列表
    std::shuffle(candidates.begin(), candidates.end(), rng_);
    
    // 尝试找到不会立即形成三连的水果类型
    for (const auto& fruitType : candidates) {
        if (!wouldCreateMatch(map, row, col, fruitType)) {
            return fruitType;
        }
    }
    
    // 如果所有类型都会形成三连(理论上不应该发生),返回随机类型
    return generateRandomFruit();
}

void FruitGenerator::fillEmptySlots(std::vector<std::vector<Fruit>>& map) {
    // 从上到下，从左到右填充所有空位
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map[row][col].type == FruitType::EMPTY) {
                // 生成安全的水果类型
                FruitType fruitType = generateSafeFruit(map, row, col);
                
                // 更新水果对象
                map[row][col].type = fruitType;
                map[row][col].special = SpecialType::NONE;
                map[row][col].row = row;
                map[row][col].col = col;
                map[row][col].isMatched = false;
                map[row][col].isMoving = false;
                map[row][col].animationProgress = 0.0f;
            }
        }
    }
}

void FruitGenerator::setSeed(unsigned int seed) {
    rng_.seed(seed);
}

void FruitGenerator::shuffleMap(std::vector<std::vector<Fruit>>& map, MatchDetector& detector) {
    // 收集所有非空水果类型
    std::vector<FruitType> fruits;
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            if (map[row][col].type != FruitType::EMPTY) {
                fruits.push_back(map[row][col].type);
            }
        }
    }
    
    // 打乱水果列表
    std::shuffle(fruits.begin(), fruits.end(), rng_);
    
    // 尝试多次重排，直到找到无三连且有可移动的排列
    const int maxAttempts = 100;
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        // 将打乱后的水果重新放回地图
        int index = 0;
        for (int row = 0; row < MAP_SIZE; row++) {
            for (int col = 0; col < MAP_SIZE; col++) {
                if (index < fruits.size()) {
                    map[row][col].type = fruits[index];
                    map[row][col].special = SpecialType::NONE;
                    map[row][col].row = row;
                    map[row][col].col = col;
                    map[row][col].isMatched = false;
                    map[row][col].isMoving = false;
                    map[row][col].animationProgress = 0.0f;
                    index++;
                }
            }
        }
        
        // 检查是否无三连且有可移动
        if (!detector.hasMatches(map) && detector.hasPossibleMoves(map)) {
            return; // 成功找到合法排列
        }
        
        // 如果不合法，重新打乱
        std::shuffle(fruits.begin(), fruits.end(), rng_);
    }
    
    // 如果所有尝试都失败，使用初始化地图的方法（保证能成功）
    initializeMap(map);
}

bool FruitGenerator::ensurePlayable(std::vector<std::vector<Fruit>>& map, MatchDetector& detector) {
    // 检查当前地图是否有可移动
    if (detector.hasPossibleMoves(map)) {
        return true; // 已经有可移动，无需处理
    }
    
    // 无可移动，进行重排
    shuffleMap(map, detector);
    
    // 再次检查是否有可移动
    return detector.hasPossibleMoves(map);
}

bool FruitGenerator::wouldCreateMatch(const std::vector<std::vector<Fruit>>& map, 
                                      int row, int col, FruitType type) {
    // 检查横向是否会形成三连
    // 检查左边两个位置
    if (col >= 2) {
        if (map[row][col - 1].type == type && 
            map[row][col - 2].type == type &&
            map[row][col - 1].type != FruitType::EMPTY) {
            return true;
        }
    }
    
    // 检查左边一个和右边一个
    if (col >= 1 && col < MAP_SIZE - 1) {
        if (map[row][col - 1].type == type && 
            map[row][col + 1].type == type &&
            map[row][col - 1].type != FruitType::EMPTY) {
            return true;
        }
    }
    
    // 检查右边两个位置(虽然初始化时不太可能,但为了完整性)
    if (col < MAP_SIZE - 2) {
        if (map[row][col + 1].type == type && 
            map[row][col + 2].type == type &&
            map[row][col + 1].type != FruitType::EMPTY) {
            return true;
        }
    }
    
    // 检查纵向是否会形成三连
    // 检查上边两个位置
    if (row >= 2) {
        if (map[row - 1][col].type == type && 
            map[row - 2][col].type == type &&
            map[row - 1][col].type != FruitType::EMPTY) {
            return true;
        }
    }
    
    // 检查上边一个和下边一个
    if (row >= 1 && row < MAP_SIZE - 1) {
        if (map[row - 1][col].type == type && 
            map[row + 1][col].type == type &&
            map[row - 1][col].type != FruitType::EMPTY) {
            return true;
        }
    }
    
    // 检查下边两个位置
    if (row < MAP_SIZE - 2) {
        if (map[row + 1][col].type == type && 
            map[row + 2][col].type == type &&
            map[row + 1][col].type != FruitType::EMPTY) {
            return true;
        }
    }
    
    // 没有形成三连，安全
    return false;
}
