#include "mainwindow.h"
#include <QProcess>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QString>

int main(int argc, char *argv[])
{
    bool isSecondaryInstance = false;
    for (int i = 0; i < argc; ++i) {
        if (QString(argv[i]) == "--secondary") {
            isSecondaryInstance = true;
            break;
        }
    }

    // 只有第一个实例（主实例）才启动第二个实例
    if (!isSecondaryInstance) {
        // 启动第二个实例时，传递一个标记参数
        QProcess::startDetached(argv[0], QStringList() << "--secondary");
    }


    QApplication a(argc, argv);
    QString app_path=QCoreApplication::applicationDirPath();
    QString filename="config.ini";
    QString config_path=QDir::toNativeSeparators(app_path+
                                                   QDir::separator()+filename);
    QSettings settings(config_path,QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    qDebug() << "应用程序目录（app_path）：" << app_path;
    qDebug() << "实际读取的配置文件路径：" << settings.fileName();
    qDebug() << "读取的 host：" << gate_host;
    qDebug() << "读取的 port：" << gate_port;
    gate_url_prefix = "http://"+gate_host+":"+gate_port;
    QFile qss(":/style/stylesheet.qss");
    if(qss.open(QFile::ReadOnly)){
        qDebug("Open success");
        QString style=QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }
    else{
        qDebug("Open failed");
    }
    MainWindow w;
    w.show();
    // w.setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    return a.exec();
}
