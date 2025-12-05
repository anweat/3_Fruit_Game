#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    QApplication::setApplicationName("Fruit Crush");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("GameDev");
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    return app.exec();
}
