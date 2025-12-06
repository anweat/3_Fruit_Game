#include <QtTest>
#include "SpecialEffectProcessor.h"
#include "FruitGenerator.h"

class TestSpecialEffectProcessor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qDebug() << "=== Test SpecialEffectProcessor Start ===";
    }

    // 测试1：横向直线炸弹效果
    void testLineHEffect()
    {
        SpecialEffectProcessor processor;
        FruitGenerator fruitGen;
        std::vector<std::vector<Fruit>> map;
        std::set<std::pair<int, int>> affected;
        
        fruitGen.initializeMap(map);
        
        // 在(3, 3)位置放置横向直线炸弹
        map[3][3].special = SpecialType::LINE_H;
        
        // 触发效果
        bool result = processor.triggerSpecialEffect(map, 3, 3, affected);
        
        QVERIFY(result);
        // 应该消除整行（8个位置）
        QCOMPARE((int)affected.size(), MAP_SIZE);
        
        // 验证所有列都被影响
        for (int col = 0; col < MAP_SIZE; col++) {
            QVERIFY(affected.find({3, col}) != affected.end());
        }
    }

    // 测试2：纵向直线炸弹效果
    void testLineVEffect()
    {
        SpecialEffectProcessor processor;
        FruitGenerator fruitGen;
        std::vector<std::vector<Fruit>> map;
        std::set<std::pair<int, int>> affected;
        
        fruitGen.initializeMap(map);
        
        // 在(3, 3)位置放置纵向直线炸弹
        map[3][3].special = SpecialType::LINE_V;
        
        // 触发效果
        bool result = processor.triggerSpecialEffect(map, 3, 3, affected);
        
        QVERIFY(result);
        // 应该消除整列（8个位置）
        QCOMPARE((int)affected.size(), MAP_SIZE);
        
        // 验证所有行都被影响
        for (int row = 0; row < MAP_SIZE; row++) {
            QVERIFY(affected.find({row, 3}) != affected.end());
        }
    }

    // 测试3：菱形炸弹效果
    void testDiamondEffect()
    {
        SpecialEffectProcessor processor;
        FruitGenerator fruitGen;
        std::vector<std::vector<Fruit>> map;
        std::set<std::pair<int, int>> affected;
        
        fruitGen.initializeMap(map);
        
        // 在(4, 4)位置放置菱形炸弹
        map[4][4].special = SpecialType::DIAMOND;
        
        // 触发效果
        bool result = processor.triggerSpecialEffect(map, 4, 4, affected);
        
        QVERIFY(result);
        
        // 5×5菱形范围（曼哈顿距离≤2），中心点(4,4)
        // 应该包含13个位置：
        // 距离0: 1个 (4,4)
        // 距离1: 4个 (上下左右)
        // 距离2: 8个 (对角线和外围)
        QCOMPARE((int)affected.size(), 13);
        
        // 验证中心点被影响
        QVERIFY(affected.find({4, 4}) != affected.end());
        // 验证上下左右被影响
        QVERIFY(affected.find({3, 4}) != affected.end());
        QVERIFY(affected.find({5, 4}) != affected.end());
        QVERIFY(affected.find({4, 3}) != affected.end());
        QVERIFY(affected.find({4, 5}) != affected.end());
    }

    // 测试4：万能炸弹效果
    void testRainbowEffect()
    {
        SpecialEffectProcessor processor;
        FruitGenerator fruitGen;
        std::vector<std::vector<Fruit>> map;
        std::set<std::pair<int, int>> affected;
        
        fruitGen.initializeMap(map);
        
        // 在(4, 4)位置放置万能炸弹
        map[4][4].special = SpecialType::RAINBOW;
        
        // 设置相邻位置为APPLE，作为目标类型
        map[4][5].type = FruitType::APPLE;
        
        // 在地图上多放几个APPLE
        map[0][0].type = FruitType::APPLE;
        map[1][1].type = FruitType::APPLE;
        map[2][2].type = FruitType::APPLE;
        
        // 触发效果
        bool result = processor.triggerSpecialEffect(map, 4, 4, affected);
        
        QVERIFY(result);
        // 至少应该消除4个APPLE
        QVERIFY((int)affected.size() >= 4);
    }

    // 测试5：直线+直线组合 → 十字消除
    void testComboLineLine()
    {
        SpecialEffectProcessor processor;
        FruitGenerator fruitGen;
        std::vector<std::vector<Fruit>> map;
        std::set<std::pair<int, int>> affected;
        
        fruitGen.initializeMap(map);
        
        // 放置两个直线炸弹
        map[3][3].special = SpecialType::LINE_H;
        map[3][4].special = SpecialType::LINE_V;
        
        // 触发组合效果
        bool result = processor.triggerCombinationEffect(map, 3, 3, 3, 4, affected);
        
        QVERIFY(result);
        // 十字消除：整行+整列 = 8 + 8 - 1(重叠) = 15个位置
        // 但实际可能更多，因为会消除两个位置所在的行列
        QVERIFY((int)affected.size() >= 15);
    }

    // 测试6：菱形+菱形组合 → 7×7大范围
    void testComboDiamondDiamond()
    {
        SpecialEffectProcessor processor;
        FruitGenerator fruitGen;
        std::vector<std::vector<Fruit>> map;
        std::set<std::pair<int, int>> affected;
        
        fruitGen.initializeMap(map);
        
        // 放置两个菱形炸弹
        map[4][4].special = SpecialType::DIAMOND;
        map[4][5].special = SpecialType::DIAMOND;
        
        // 触发组合效果
        bool result = processor.triggerCombinationEffect(map, 4, 4, 4, 5, affected);
        
        QVERIFY(result);
        // 7×7菱形范围（曼哈顿距离≤3），应该包含25个位置
        QCOMPARE((int)affected.size(), 25);
    }

    // 测试7：万能+万能组合 → 全屏消除
    void testComboRainbowRainbow()
    {
        SpecialEffectProcessor processor;
        FruitGenerator fruitGen;
        std::vector<std::vector<Fruit>> map;
        std::set<std::pair<int, int>> affected;
        
        fruitGen.initializeMap(map);
        
        // 放置两个万能炸弹
        map[3][3].special = SpecialType::RAINBOW;
        map[3][4].special = SpecialType::RAINBOW;
        
        // 触发组合效果
        bool result = processor.triggerCombinationEffect(map, 3, 3, 3, 4, affected);
        
        QVERIFY(result);
        // 全屏消除：8×8 = 64个位置
        QCOMPARE((int)affected.size(), MAP_SIZE * MAP_SIZE);
    }

    // 测试8：普通水果不触发效果
    void testNoEffectForNormalFruit()
    {
        SpecialEffectProcessor processor;
        FruitGenerator fruitGen;
        std::vector<std::vector<Fruit>> map;
        std::set<std::pair<int, int>> affected;
        
        fruitGen.initializeMap(map);
        
        // 普通水果（无特殊属性）
        map[3][3].special = SpecialType::NONE;
        
        // 尝试触发效果
        bool result = processor.triggerSpecialEffect(map, 3, 3, affected);
        
        QVERIFY(!result);
        QCOMPARE((int)affected.size(), 0);
    }

    void cleanupTestCase()
    {
        qDebug() << "=== Test SpecialEffectProcessor cleanup ===";
    }
};

QTEST_MAIN(TestSpecialEffectProcessor)
#include "test_special_effect_processor.moc"
