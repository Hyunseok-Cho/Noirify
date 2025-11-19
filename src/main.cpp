#include <QApplication>
#include <QFile>
#include <QIcon>
#include "MainWindow.h"

static void loadQssIfExists(QWidget* root) {
    QFile f(QStringLiteral(":/styles.qss"));
    if (f.exists() && f.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QIcon appIcon(":/noirify.png");

    app.setWindowIcon(appIcon);
    MainWindow w;
    w.resize(1200, 720);
    w.show();
    loadQssIfExists(&w);
    return app.exec();
}
