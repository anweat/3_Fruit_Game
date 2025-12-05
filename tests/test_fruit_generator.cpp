#include <QtTest/QtTest>

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
        // TODO: 实现随机生成测试
        QVERIFY(true);
    }

    void cleanupTestCase()
    {
        qDebug() << "Test FruitGenerator cleanup";
    }
};

QTEST_MAIN(TestFruitGenerator)
#include "test_fruit_generator.moc"
