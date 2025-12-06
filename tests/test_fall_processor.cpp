#include <QtTest/QtTest>
#include "FallProcessor.h"
#include "FruitGenerator.h"
#include "FruitTypes.h"

class TestFallProcessor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qDebug() << "Test FallProcessor initialized";
    }

    void testHasEmptySlots()
    {
        FallProcessor processor;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        // 初始化满地图
        generator.initializeMap(map);
        
        // 不应有空位
        QVERIFY(!processor.hasEmptySlots(map));
        
        // 制造一个空位
        map[0][0].type = FruitType::EMPTY;
        
        // 应该检测到空位
        QVERIFY(processor.hasEmptySlots(map));
    }
    
    void testGetEmptySlots()
    {
        FallProcessor processor;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        generator.initializeMap(map);
        
        // 初始无空位
        auto emptySlots = processor.getEmptySlots(map);
        QCOMPARE(emptySlots.size(), static_cast<size_t>(0));
        
        // 制造几个空位
        map[0][0].type = FruitType::EMPTY;
        map[3][4].type = FruitType::EMPTY;
        map[7][7].type = FruitType::EMPTY;
        
        emptySlots = processor.getEmptySlots(map);
        QCOMPARE(emptySlots.size(), static_cast<size_t>(3));
    }
    
    void testFillEmptySlots()
    {
        FallProcessor processor;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        generator.initializeMap(map);
        
        // 制造空位
        map[0][0].type = FruitType::EMPTY;
        map[3][4].type = FruitType::EMPTY;
        
        // 填充
        auto filled = processor.fillEmptySlots(map, generator);
        
        // 应该填充了2个位置
        QCOMPARE(filled.size(), static_cast<size_t>(2));
        
        // 填充后应该无空位
        QVERIFY(!processor.hasEmptySlots(map));
        
        // 验证填充的位置不是EMPTY
        QVERIFY(map[0][0].type != FruitType::EMPTY);
        QVERIFY(map[3][4].type != FruitType::EMPTY);
    }
    
    void testProcessColumnFall()
    {
        FallProcessor processor;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        generator.initializeMap(map);
        
        // 在第0列制造空位
        // 原状态：[A, B, C, D, E, F, G, H]（从上到下）
        // 把位置5和6设为EMPTY
        FruitType savedType = map[3][0].type;
        map[5][0].type = FruitType::EMPTY;
        map[6][0].type = FruitType::EMPTY;
        
        // 处理第0列的下落
        auto moves = processor.processColumnFall(map, 0);
        
        // 应该有移动发生
        QVERIFY(moves.size() > 0);
        
        // 验证第5和6行现在不是空的（被上方水果填充）
        // 但顶部应该有空位
        int emptyCount = 0;
        for (int row = 0; row < MAP_SIZE; row++) {
            if (map[row][0].type == FruitType::EMPTY) {
                emptyCount++;
            }
        }
        // 应该有2个空位（移到了顶部）
        QCOMPARE(emptyCount, 2);
    }
    
    void testProcessFall()
    {
        FallProcessor processor;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        generator.initializeMap(map);
        
        // 制造多个空位
        map[5][0].type = FruitType::EMPTY;
        map[6][0].type = FruitType::EMPTY;
        map[4][3].type = FruitType::EMPTY;
        
        // 处理全图下落
        auto steps = processor.processFall(map, generator);
        
        // 应该至少有一个步骤
        QVERIFY(steps.size() > 0);
        
        // 处理后应该无空位（已填充）
        QVERIFY(!processor.hasEmptySlots(map));
    }

    void cleanupTestCase()
    {
        qDebug() << "Test FallProcessor cleanup";
    }
};

QTEST_MAIN(TestFallProcessor)
#include "test_fall_processor.moc"
