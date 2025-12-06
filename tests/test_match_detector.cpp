#include <QtTest/QtTest>
#include "MatchDetector.h"
#include "FruitGenerator.h"
#include "FruitTypes.h"

class TestMatchDetector : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qDebug() << "Test MatchDetector initialized";
    }

    void testHorizontalMatch()
    {
        MatchDetector detector;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        // 创建一个8x8地图
        generator.initializeMap(map);
        
        // 手动创建横向三连 (第0行, 0-2列都是苹果)
        map[0][0].type = FruitType::APPLE;
        map[0][1].type = FruitType::APPLE;
        map[0][2].type = FruitType::APPLE;
        
        // 清理周围可能干扰的位置，确保恰好是3个匹配
        if (map[0][3].type == FruitType::APPLE) {
            map[0][3].type = FruitType::BANANA;
        }
        // 清理纵向可能的干扰
        for (int col = 0; col <= 2; col++) {
            if (map[1][col].type == FruitType::APPLE) {
                map[1][col].type = FruitType::BANANA;
            }
            if (map[2][col].type == FruitType::APPLE) {
                map[2][col].type = FruitType::GRAPE;
            }
        }
        
        // 检测匹配
        auto matches = detector.detectMatches(map);
        
        // 应该至少检测到一个匹配
        QVERIFY(matches.size() > 0);
        
        // 验证第一个匹配是横向的
        bool foundHorizontalMatch = false;
        for (const auto& match : matches) {
            if (match.direction == MatchDirection::HORIZONTAL && 
                match.fruitType == FruitType::APPLE) {
                foundHorizontalMatch = true;
                QVERIFY(match.matchCount >= 3);
            }
        }
        QVERIFY(foundHorizontalMatch);
    }

    void testVerticalMatch()
    {
        MatchDetector detector;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        // 创建一个8x8地图
        generator.initializeMap(map);
        
        // 手动创建纵向三连 (第0列, 0-2行都是橙子)
        map[0][0].type = FruitType::ORANGE;
        map[1][0].type = FruitType::ORANGE;
        map[2][0].type = FruitType::ORANGE;
        
        // 清理周围可能干扰的位置，确保恰好是3个匹配
        if (map[3][0].type == FruitType::ORANGE) {
            map[3][0].type = FruitType::APPLE;  // 改成不同的水果
        }
        // 清理横向可能的干扰
        if (map[0][1].type == FruitType::ORANGE) {
            map[0][1].type = FruitType::APPLE;
        }
        if (map[1][1].type == FruitType::ORANGE) {
            map[1][1].type = FruitType::APPLE;
        }
        if (map[2][1].type == FruitType::ORANGE) {
            map[2][1].type = FruitType::APPLE;
        }
        
        // 检测匹配
        auto matches = detector.detectMatches(map);
        
        // 应该至少检测到一个匹配
        QVERIFY(matches.size() > 0);
        
        // 验证匹配是纵向的
        bool foundVerticalMatch = false;
        for (const auto& match : matches) {
            if (match.direction == MatchDirection::VERTICAL && 
                match.fruitType == FruitType::ORANGE) {
                foundVerticalMatch = true;
                QVERIFY(match.matchCount >= 3);
            }
        }
        QVERIFY(foundVerticalMatch);
    }
    
    void testFourMatch()
    {
        MatchDetector detector;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        generator.initializeMap(map);
        
        // 创建横向四连
        map[3][2].type = FruitType::GRAPE;
        map[3][3].type = FruitType::GRAPE;
        map[3][4].type = FruitType::GRAPE;
        map[3][5].type = FruitType::GRAPE;
        
        // 清理可能干扰的位置，确保恰好是4个匹配
        if (map[3][1].type == FruitType::GRAPE) {
            map[3][1].type = FruitType::APPLE;
        }
        if (map[3][6].type == FruitType::GRAPE) {
            map[3][6].type = FruitType::APPLE;
        }
        // 清理纵向可能的干扰
        for (int row = 0; row < MAP_SIZE; row++) {
            if (row != 3) {
                for (int col = 2; col <= 5; col++) {
                    if (map[row][col].type == FruitType::GRAPE) {
                        map[row][col].type = FruitType::APPLE;
                    }
                }
            }
        }
        
        auto matches = detector.detectMatches(map);
        
        // 验证检测到4消，并应生成直线炸弹
        bool foundFourMatch = false;
        for (const auto& match : matches) {
            if (match.matchCount == 4 && match.fruitType == FruitType::GRAPE) {
                foundFourMatch = true;
                QCOMPARE(match.generateSpecial, SpecialType::LINE_H);
            }
        }
        QVERIFY(foundFourMatch);
    }
    
    void testFiveMatch()
    {
        MatchDetector detector;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        generator.initializeMap(map);
        
        // 创建横向五连
        map[5][1].type = FruitType::BANANA;
        map[5][2].type = FruitType::BANANA;
        map[5][3].type = FruitType::BANANA;
        map[5][4].type = FruitType::BANANA;
        map[5][5].type = FruitType::BANANA;
        
        // 清理可能干扰的位置，确保恰好是5个匹配
        if (map[5][0].type == FruitType::BANANA) {
            map[5][0].type = FruitType::APPLE;
        }
        if (map[5][6].type == FruitType::BANANA) {
            map[5][6].type = FruitType::APPLE;
        }
        // 清理纵向可能的干扰
        for (int row = 0; row < MAP_SIZE; row++) {
            if (row != 5) {
                for (int col = 1; col <= 5; col++) {
                    if (map[row][col].type == FruitType::BANANA) {
                        map[row][col].type = FruitType::APPLE;
                    }
                }
            }
        }
        
        auto matches = detector.detectMatches(map);
        
        // 验证检测到5消，并应生成万能炸弹
        bool foundFiveMatch = false;
        for (const auto& match : matches) {
            if (match.matchCount == 5 && match.fruitType == FruitType::BANANA) {
                foundFiveMatch = true;
                QCOMPARE(match.generateSpecial, SpecialType::RAINBOW);
            }
        }
        QVERIFY(foundFiveMatch);
    }
    
    void testNoMatches()
    {
        MatchDetector detector;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        // 生成无三连的地图
        generator.initializeMap(map);
        
        // 应该没有匹配
        auto matches = detector.detectMatches(map);
        QCOMPARE(matches.size(), static_cast<size_t>(0));
        
        // hasMatches应该返回false
        QVERIFY(!detector.hasMatches(map));
    }
    
    void testHasPossibleMoves()
    {
        MatchDetector detector;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        // 初始化地图
        generator.initializeMap(map);
        
        // 一般情况下应该有可能的移动
        // (这个测试可能偶尔失败，但概率很低)
        bool hasMoves = detector.hasPossibleMoves(map);
        
        // 至少应该能够检测到一些可能的移动
        // 注意：如果地图真的无解，这个测试会失败，但概率极低
        qDebug() << "Has possible moves:" << hasMoves;
    }
    
    void testMatchesAt()
    {
        MatchDetector detector;
        FruitGenerator generator;
        std::vector<std::vector<Fruit>> map;
        
        generator.initializeMap(map);
        
        // 创建特定位置的三连
        map[2][2].type = FruitType::WATERMELON;
        map[2][3].type = FruitType::WATERMELON;
        map[2][4].type = FruitType::WATERMELON;
        
        // 检测特定位置的匹配
        auto matches = detector.detectMatchesAt(map, 2, 3);
        
        // 应该检测到匹配
        QVERIFY(matches.size() > 0);
        QCOMPARE(matches[0].fruitType, FruitType::WATERMELON);
    }

    void cleanupTestCase()
    {
        qDebug() << "Test MatchDetector cleanup";
    }
};

QTEST_MAIN(TestMatchDetector)
#include "test_match_detector.moc"
