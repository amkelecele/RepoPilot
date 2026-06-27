#include "mainwindow.h"

#include <QApplication>
#include <QPalette>
#include <QColor>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPalette dark;

    dark.setColor(
        QPalette::Window,
        QColor(25,25,25)
        );

    dark.setColor(
        QPalette::WindowText,
        Qt::white
        );

    dark.setColor(
        QPalette::Base,
        QColor(35,35,35)
        );

    dark.setColor(
        QPalette::AlternateBase,
        QColor(50,50,50)
        );

    dark.setColor(
        QPalette::Text,
        Qt::white
        );

    dark.setColor(
        QPalette::Button,
        QColor(40,40,40)
        );

    dark.setColor(
        QPalette::ButtonText,
        Qt::white
        );

    dark.setColor(
        QPalette::Highlight,
        QColor(255,160,40)
        );

    a.setPalette(
        dark
        );

    MainWindow w;
    w.show();
    return QApplication::exec();
}
