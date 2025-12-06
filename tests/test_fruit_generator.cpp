#include <QtTest/QtTest>
#include "FruitGenerator.h"
#include "MatchDetector.h"
#include "FruitTypes.h"

class TestFruitGenerator : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qDebug() << "Test FruitGenerator initialized";
    }

    void testRandomGeneration()
    {
        FruitGenerator generator;
        
        // 测试生成100个随机水果，确保都是有效类型
        for (int i = 0; i < 100; i++) {
            FruitType fruit = generator.generateRandomFruit();
            QVERIFY(fruit != FruitType::EMPTY);
            QVERIFY(static_cast<int>(fruit) >= 0 && static_cast<int>(fruit) < FRUIT_TYPE_COUNT);
        }
    }
    
    void testInitializeMap()
    {
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        // 测试地图初始化
        generator.initializeMap(map);
        
        // 验证地图大小
        QCOMPARE(map.size(), static_cast<size_t>(MAP_SIZE));
        for (int i = 0; i < MAP_SIZE; i++) {
            QCOMPARE(map[i].size(), static_cast<size_t>(MAP_SIZE));
        }
        
        // 验证所有位置都有水果
        for (int row = 0; row < MAP_SIZE; row++) {
            for (int col = 0; col < MAP_SIZE; col++) {
                QVERIFY(map[row][col].type != FruitType::EMPTY);
                QCOMPARE(map[row][col].row, row);
                QCOMPARE(map[row][col].col, col);
            }
        }
    }
    
    void testNoInitialMatches()
    {
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        // 测试多次初始化，确保没有初始三连
        for (int test = 0; test < 10; test++) {
            generator.initializeMap(map);
            
            // 检查横向没有三连
            for (int row = 0; row < MAP_SIZE; row++) {
                for (int col = 0; col < MAP_SIZE - 2; col++) {
                    bool hasMatch = (map[row][col].type == map[row][col + 1].type &&
                                    map[row][col].type == map[row][col + 2].type);
                    QVERIFY2(!hasMatch, "Found horizontal match in initial map");
                }
            }
            
            // 检查纵向没有三连
            for (int col = 0; col < MAP_SIZE; col++) {
                for (int row = 0; row < MAP_SIZE - 2; row++) {
                    bool hasMatch = (map[row][col].type == map[row + 1][col].type &&
                                    map[row][col].type == map[row + 2][col].type);
                    QVERIFY2(!hasMatch, "Found vertical match in initial map");
                }
            }
        }
    }
    
    void testFillEmptySlots()
    {
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        // 先初始化地图
        generator.initializeMap(map);
        
        // 将一些位置设为空
        map[0][0].type = FruitType::EMPTY;
        map[3][4].type = FruitType::EMPTY;
        map[7][7].type = FruitType::EMPTY;
        
        // 填充空位
        generator.fillEmptySlots(map);
        
        // 验证所有位置都不为空
        for (int row = 0; row < MAP_SIZE; row++) {
            for (int col = 0; col < MAP_SIZE; col++) {
                QVERIFY(map[row][col].type != FruitType::EMPTY);
            }
        }
    }
    
    void testSeedReproducibility()
    {
        FruitGenerator gen1;
        FruitGenerator gen2;
        
        // 使用相同种子
        gen1.setSeed(12345);
        gen2.setSeed(12345);
        
        // 生成的序列应该相同
        for (int i = 0; i < 20; i++) {
            FruitType f1 = gen1.generateRandomFruit();
            FruitType f2 = gen2.generateRandomFruit();
            QCOMPARE(f1, f2);
        }
    }
    
    void testShuffleMap()
    {
        FruitGenerator generator;
        MatchDetector detector;
        std::vector<std::vector<Fruit>> map;
        
        // 初始化地图
        generator.initializeMap(map);
        
        // 记录原始水果类型分布
        std::map<FruitType, int> originalCounts;
        for (int row = 0; row < MAP_SIZE; row++) {
            for (int col = 0; col < MAP_SIZE; col++) {
                originalCounts[map[row][col].type]++;
            }
        }
        
        // 进行重排
        generator.shuffleMap(map, detector);
        
        // 验证重排后水果类型分布不变
        std::map<FruitType, int> newCounts;
        for (int row = 0; row < MAP_SIZE; row++) {
            for (int col = 0; col < MAP_SIZE; col++) {
                newCounts[map[row][col].type]++;
            }
        }
        
        QCOMPARE(originalCounts.size(), newCounts.size());
        for (const auto& pair : originalCounts) {
            QCOMPARE(newCounts[pair.first], pair.second);
        }
        
        // 验证重排后无三连
        QVERIFY(!detector.hasMatches(map));
        
        // 验证重排后有可移动
        QVERIFY(detector.hasPossibleMoves(map));
    }
    
    void testEnsurePlayable()
    {
        FruitGenerator generator;
        MatchDetector detector;
        std::vector<std::vector<Fruit>> map;
        
        // 初始化地图
        generator.initializeMap(map);
        
        // 正常情况下应该已经有可移动
        bool result = generator.ensurePlayable(map, detector);
        QVERIFY(result);
        QVERIFY(detector.hasPossibleMoves(map));
        
        // 构造一个潜在死局：创建一个特殊排列
        // 使用两种交替排列的水果类型（棋盘格模式）
        // 这种模式在某些情况下可能没有可移动
        for (int row = 0; row < MAP_SIZE; row++) {
            for (int col = 0; col < MAP_SIZE; col++) {
                if ((row + col) % 2 == 0) {
                    map[row][col].type = FruitType::APPLE;
                } else {
                    map[row][col].type = FruitType::BANANA;
                }
            }
        }
        
        // ensurePlayable 应该能够确保地图可玩
        result = generator.ensurePlayable(map, detector);
        QVERIFY(result);
        // 修复后应该无三连且有可移动
        QVERIFY(!detector.hasMatches(map));
        QVERIFY(detector.hasPossibleMoves(map));
    }
    
    void testInitializeMapHasValidMoves()
    {
        FruitGenerator generator;
        MatchDetector detector;
        std::vector<std::vector<Fruit>> map;
        
        // 测试多次初始化，确保每次都有可移动
        for (int test = 0; test < 10; test++) {
            generator.initializeMap(map);
            
            // 验证无三连
            QVERIFY(!detector.hasMatches(map));
            
            // 验证有可移动
            QVERIFY2(detector.hasPossibleMoves(map), "Initialized map should have valid moves");
        }
    }

    void cleanupTestCase()
    {
        qDebug() << "Test FruitGenerator cleanup";
    }
};

QTEST_MAIN(TestFruitGenerator)
#include "test_fruit_generator.moc"
