#include <QtTest/QtTest>
#include "FruitGenerator.h"
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

    void cleanupTestCase()
    {
        qDebug() << "Test FruitGenerator cleanup";
    }
};

QTEST_MAIN(TestFruitGenerator)
#include "test_fruit_generator.moc"
