#include <QtTest>
#include "GameEngine.h"

class TestGameEngine : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qDebug() << "=== Test GameEngine Start ===";
    }

    // 测试1：初始化游戏
    void testInitializeGame()
    {
        GameEngine engine;
        engine.initializeGame();
        
        // 验证地图已初始化
        const auto& map = engine.getMap();
        QCOMPARE((int)map.size(), MAP_SIZE);
        QCOMPARE((int)map[0].size(), MAP_SIZE);
        
        // 验证分数为0
        QCOMPARE(engine.getCurrentScore(), 0);
        
        // 验证连击为0
        QCOMPARE(engine.getComboCount(), 0);
        
        // 验证状态为IDLE
        QCOMPARE(engine.getState(), GameState::IDLE);
        
        // 验证有可移动
        QVERIFY(engine.hasValidMoves());
    }

    // 测试2：无效交换（不相邻）
    void testInvalidSwap()
    {
        GameEngine engine;
        engine.initializeGame();
        
        // 尝试交换不相邻的水果
        bool result = engine.swapFruits(0, 0, 2, 2);
        QVERIFY(!result);  // 应该失败
    }

    // 测试3：有效交换并产生匹配
    void testValidSwapWithMatch()
    {
        GameEngine engine;
        engine.initializeGame();
        
        auto& map = const_cast<std::vector<std::vector<Fruit>>&>(engine.getMap());
        
        // 手动构造一个确定会匹配的情况
        // 设置(0,0)到(0,2)都是苹果，(0,3)是香蕉
        map[0][0].type = FruitType::APPLE;
        map[0][1].type = FruitType::APPLE;
        map[0][2].type = FruitType::BANANA;
        map[0][3].type = FruitType::APPLE;
        
        // 清理特殊属性
        for (int col = 0; col < 4; col++) {
            map[0][col].special = SpecialType::NONE;
        }
        
        int scoreBefore = engine.getCurrentScore();
        
        // 交换(0,2)和(0,3)，形成三个苹果连线
        bool result = engine.swapFruits(0, 2, 0, 3);
        
        // 交换应该成功，并且分数应该增加
        QVERIFY(result);
        QVERIFY(engine.getCurrentScore() > scoreBefore);
    }

    // 测试4：游戏循环处理
    void testGameCycle()
    {
        GameEngine engine;
        engine.initializeGame();
        
        auto& map = const_cast<std::vector<std::vector<Fruit>>&>(engine.getMap());
        
        // 手动创建匹配
        map[0][0].type = FruitType::GRAPE;
        map[0][1].type = FruitType::GRAPE;
        map[0][2].type = FruitType::GRAPE;
        map[0][0].special = SpecialType::NONE;
        map[0][1].special = SpecialType::NONE;
        map[0][2].special = SpecialType::NONE;
        
        int scoreBefore = engine.getCurrentScore();
        
        // 处理游戏循环
        bool hadElimination = engine.processGameCycle();
        
        // 应该有消除发生，分数应该增加
        QVERIFY(hadElimination);
        QVERIFY(engine.getCurrentScore() > scoreBefore);
    }

    // 测试5：重置游戏
    void testResetGame()
    {
        GameEngine engine;
        engine.initializeGame();
        
        // 手动增加一些分数
        auto& map = const_cast<std::vector<std::vector<Fruit>>&>(engine.getMap());
        map[0][0].type = FruitType::ORANGE;
        map[0][1].type = FruitType::ORANGE;
        map[0][2].type = FruitType::ORANGE;
        map[0][0].special = SpecialType::NONE;
        map[0][1].special = SpecialType::NONE;
        map[0][2].special = SpecialType::NONE;
        
        engine.processGameCycle();
        
        int scoreAfterPlay = engine.getCurrentScore();
        QVERIFY(scoreAfterPlay > 0);
        
        // 重置游戏
        engine.resetGame();
        
        // 验证分数被重置
        QCOMPARE(engine.getCurrentScore(), 0);
        QCOMPARE(engine.getComboCount(), 0);
        QCOMPARE(engine.getState(), GameState::IDLE);
    }

    // 测试6：检查有可移动
    void testHasValidMoves()
    {
        GameEngine engine;
        engine.initializeGame();
        
        // 初始化后应该有可移动
        QVERIFY(engine.hasValidMoves());
    }

    void cleanupTestCase()
    {
        qDebug() << "=== Test GameEngine cleanup ===";
    }
};

QTEST_MAIN(TestGameEngine)
#include "test_game_engine.moc"
