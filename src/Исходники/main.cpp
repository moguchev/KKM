#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    auto mainWindow = new MainWindow;
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);
    mainWindow->setWindowTitle("KKM");
    mainWindow->setWindowIcon(QIcon("C:/Users/Sergei/Desktop/7 сем/АСВТ/Курсовая/icon.jpg"));
    mainWindow->show();
    return a.exec();
}
