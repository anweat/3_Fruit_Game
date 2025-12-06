#include <QtTest>
#include "SpecialFruitGenerator.h"
#include "FruitGenerator.h"
#include "MatchDetector.h"

class TestSpecialFruitGenerator : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qDebug() << "=== Test SpecialFruitGenerator Start ===";
    }

    // 测试1：判断4消横向应生成横向直线炸弹
    void testDetermineLineBombH()
    {
        SpecialFruitGenerator generator;
        
        MatchResult match;
        match.matchCount = 4;
        match.direction = MatchDirection::HORIZONTAL;
        match.fruitType = FruitType::APPLE;
        match.positions = {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
        
        SpecialType result = generator.determineSpecialType(match);
        QCOMPARE(result, SpecialType::LINE_H);
    }

    // 测试2：判断4消纵向应生成纵向直线炸弹
    void testDetermineLineBombV()
    {
        SpecialFruitGenerator generator;
        
        MatchResult match;
        match.matchCount = 4;
        match.direction = MatchDirection::VERTICAL;
        match.fruitType = FruitType::BANANA;
        match.positions = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
        
        SpecialType result = generator.determineSpecialType(match);
        QCOMPARE(result, SpecialType::LINE_V);
    }

    // 测试3：判断5消直线应生成万能炸弹
    void testDetermineRainbow()
    {
        SpecialFruitGenerator generator;
        
        MatchResult match;
        match.matchCount = 5;
        match.direction = MatchDirection::HORIZONTAL;
        match.fruitType = FruitType::GRAPE;
        match.positions = {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}};
        
        SpecialType result = generator.determineSpecialType(match);
        QCOMPARE(result, SpecialType::RAINBOW);
    }

    // 测试4：判断L形应生成菱形炸弹
    void testDetermineDiamond()
    {
        SpecialFruitGenerator generator;
        
        MatchResult match;
        match.matchCount = 5;
        match.direction = MatchDirection::L_SHAPE;
        match.fruitType = FruitType::ORANGE;
        
        SpecialType result = generator.determineSpecialType(match);
        QCOMPARE(result, SpecialType::DIAMOND);
    }

    // 测试5：判断3消不应生成特殊元素
    void testNoSpecialFor3Match()
    {
        SpecialFruitGenerator generator;
        
        MatchResult match;
        match.matchCount = 3;
        match.direction = MatchDirection::HORIZONTAL;
        match.fruitType = FruitType::WATERMELON;
        
        SpecialType result = generator.determineSpecialType(match);
        QCOMPARE(result, SpecialType::NONE);
    }

    // 测试6：在地图上生成特殊水果
    void testGenerateSpecialFruit()
    {
        SpecialFruitGenerator generator;
        FruitGenerator fruitGen;
        std::vector<std::vector<Fruit>> map;
        
        // 初始化地图
        fruitGen.initializeMap(map);
        
        // 构造匹配结果
        MatchResult match;
        match.matchCount = 4;
        match.direction = MatchDirection::HORIZONTAL;
        match.fruitType = FruitType::APPLE;
        match.positions = {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
        
        // 生成横向直线炸弹
        auto pos = generator.generateSpecialFruit(map, match, SpecialType::LINE_H);
        
        // 验证生成位置有效
        QVERIFY(pos.first >= 0 && pos.first < MAP_SIZE);
        QVERIFY(pos.second >= 0 && pos.second < MAP_SIZE);
        
        // 验证该位置确实有特殊属性
        QCOMPARE(map[pos.first][pos.second].special, SpecialType::LINE_H);
    }

    // 测试7：不生成NONE类型的特殊元素
    void testGenerateNoneSpecial()
    {
        SpecialFruitGenerator generator;
        FruitGenerator fruitGen;
        std::vector<std::vector<Fruit>> map;
        
        fruitGen.initializeMap(map);
        
        MatchResult match;
        match.matchCount = 3;
        match.positions = {{0, 0}, {0, 1}, {0, 2}};
        
        auto pos = generator.generateSpecialFruit(map, match, SpecialType::NONE);
        
        // 应该返回无效位置
        QCOMPARE(pos.first, -1);
        QCOMPARE(pos.second, -1);
    }

    void cleanupTestCase()
    {
        qDebug() << "=== Test SpecialFruitGenerator cleanup ===";
    }
};

QTEST_MAIN(TestSpecialFruitGenerator)
#include "test_special_fruit_generator.moc"
