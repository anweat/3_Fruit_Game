#include <QtTest/QtTest>
#include "ScoreCalculator.h"
#include "FruitTypes.h"

class TestScoreCalculator : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qDebug() << "Test ScoreCalculator initialized";
    }

    void testBaseScore()
    {
        ScoreCalculator calculator;
        
        // 3消 = 30分
        QCOMPARE(calculator.getBaseScore(3), 30);
        
        // 4消 = 80分
        QCOMPARE(calculator.getBaseScore(4), 80);
        
        // 5消 = 200分
        QCOMPARE(calculator.getBaseScore(5), 200);
        
        // 6消 = 300分
        QCOMPARE(calculator.getBaseScore(6), 300);
        
        // 7消 = 300 + 50 = 350分
        QCOMPARE(calculator.getBaseScore(7), 350);
        
        // 8消 = 300 + 50*2 = 400分
        QCOMPARE(calculator.getBaseScore(8), 400);
    }
    
    void testSpecialBonus()
    {
        ScoreCalculator calculator;
        
        // 无特殊元素
        QCOMPARE(calculator.getSpecialBonus(SpecialType::NONE), 0);
        
        // 直线炸弹 = 50分
        QCOMPARE(calculator.getSpecialBonus(SpecialType::LINE_H), 50);
        QCOMPARE(calculator.getSpecialBonus(SpecialType::LINE_V), 50);
        
        // 菱形炸弹 = 100分
        QCOMPARE(calculator.getSpecialBonus(SpecialType::DIAMOND), 100);
        
        // 万能炸弹 = 150分
        QCOMPARE(calculator.getSpecialBonus(SpecialType::RAINBOW), 150);
    }
    
    void testShapeBonus()
    {
        ScoreCalculator calculator;
        
        // 创建一个普通匹配
        MatchResult normalMatch;
        normalMatch.matchCount = 3;
        normalMatch.generateSpecial = SpecialType::NONE;
        
        // 普通匹配无形状奖励
        QCOMPARE(calculator.getShapeBonus(normalMatch), 0);
        
        // 创建L形/T形匹配（生成菱形炸弹）
        MatchResult shapeMatch;
        shapeMatch.matchCount = 5;
        shapeMatch.generateSpecial = SpecialType::DIAMOND;
        
        // L形/T形有额外100分奖励
        QCOMPARE(calculator.getShapeBonus(shapeMatch), 100);
    }
    
    void testCalculateMatchScore()
    {
        ScoreCalculator calculator;
        
        // 测试1：普通3消，无连击
        MatchResult match1;
        match1.matchCount = 3;
        match1.fruitType = FruitType::APPLE;
        match1.generateSpecial = SpecialType::NONE;
        match1.direction = MatchDirection::HORIZONTAL;
        
        // 3消 = 30分，无特殊奖励，无连击 = 30 * 1 = 30
        QCOMPARE(calculator.calculateMatchScore(match1, 1), 30);
        
        // 测试2：4消生成直线炸弹，无连击
        MatchResult match2;
        match2.matchCount = 4;
        match2.fruitType = FruitType::ORANGE;
        match2.generateSpecial = SpecialType::LINE_H;
        match2.direction = MatchDirection::HORIZONTAL;
        
        // 4消 = 80分 + 直线炸弹50分 = 130分
        QCOMPARE(calculator.calculateMatchScore(match2, 1), 130);
        
        // 测试3：5消生成万能炸弹，2连击
        MatchResult match3;
        match3.matchCount = 5;
        match3.fruitType = FruitType::BANANA;
        match3.generateSpecial = SpecialType::RAINBOW;
        match3.direction = MatchDirection::HORIZONTAL;
        
        // 5消 = 200分 + 万能炸弹150分 = 350分 × 2连击 = 700分
        QCOMPARE(calculator.calculateMatchScore(match3, 2), 700);
        
        // 测试4：L形/T形匹配生成菱形炸弹
        MatchResult match4;
        match4.matchCount = 5;
        match4.fruitType = FruitType::GRAPE;
        match4.generateSpecial = SpecialType::DIAMOND;
        match4.direction = MatchDirection::L_SHAPE;
        
        // 5消 = 200分 + 菱形炸弹100分 + L形奖励100分 = 400分
        QCOMPARE(calculator.calculateMatchScore(match4, 1), 400);
    }
    
    void testCalculateTotalScore()
    {
        ScoreCalculator calculator;
        std::vector<MatchResult> matches;
        
        // 匹配1：3消
        MatchResult match1;
        match1.matchCount = 3;
        match1.generateSpecial = SpecialType::NONE;
        matches.push_back(match1);
        
        // 匹配2：4消+直线炸弹
        MatchResult match2;
        match2.matchCount = 4;
        match2.generateSpecial = SpecialType::LINE_H;
        matches.push_back(match2);
        
        // 总分 = (30 + 130) * 1 = 160
        QCOMPARE(calculator.calculateTotalScore(matches, 1), 160);
        
        // 2连击情况下
        // 总分 = (30 + 130) * 2 = 320
        QCOMPARE(calculator.calculateTotalScore(matches, 2), 320);
    }
    
    void testComboSystem()
    {
        ScoreCalculator calculator;
        
        // 初始连击为0
        QCOMPARE(calculator.getComboCount(), 0);
        
        // 增加连击
        int combo1 = calculator.incrementCombo();
        QCOMPARE(combo1, 1);
        QCOMPARE(calculator.getComboCount(), 1);
        
        int combo2 = calculator.incrementCombo();
        QCOMPARE(combo2, 2);
        QCOMPARE(calculator.getComboCount(), 2);
        
        int combo3 = calculator.incrementCombo();
        QCOMPARE(combo3, 3);
        
        // 重置连击
        calculator.resetCombo();
        QCOMPARE(calculator.getComboCount(), 0);
    }

    void cleanupTestCase()
    {
        qDebug() << "Test ScoreCalculator cleanup";
    }
};

QTEST_MAIN(TestScoreCalculator)
#include "test_score_calculator.moc"
