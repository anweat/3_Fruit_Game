#include <QtTest/QtTest>

class TestScoreCalculator : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qDebug() << "Test ScoreCalculator initialized";
    }

    void testBasicScore()
    {
        // TODO: 实现基础得分测试
        QVERIFY(true);
    }

    void testComboScore()
    {
        // TODO: 实现连击得分测试
        QVERIFY(true);
    }

    void cleanupTestCase()
    {
        qDebug() << "Test ScoreCalculator cleanup";
    }
};

QTEST_MAIN(TestScoreCalculator)
#include "test_score_calculator.moc"
