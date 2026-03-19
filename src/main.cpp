#include <QApplication>
#include <QSurfaceFormat>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <iostream>
#include <fstream>
#include "ui/main_window.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

static std::ofstream g_logFile;

void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("[DEBUG] %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("[WARN] %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("[ERROR] %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("[FATAL] %1").arg(msg);
        break;
    case QtInfoMsg:
        txt = QString("[INFO] %1").arg(msg);
        break;
    }
    
    std::string stdStr = txt.toStdString();
    std::cerr << stdStr << std::endl;
    
    if (g_logFile.is_open()) {
        g_logFile << stdStr << std::endl;
        g_logFile.flush();
    }
}

int main(int argc, char *argv[]) {
    g_logFile.open("vms_debug.log", std::ios::out | std::ios::trunc);
    qInstallMessageHandler(customMessageHandler);
    
    std::cerr << "Starting VMS..." << std::endl;
    
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    
    std::cerr << "Initializing FFmpeg..." << std::endl;
#if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
#endif
    avformat_network_init();
    
    std::cerr << "Creating QApplication..." << std::endl;
    QApplication app(argc, argv);
    
    app.setApplicationName("VMS");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("VMS");
    app.setOrganizationDomain("vms.local");
    
    std::cerr << "Creating MainWindow..." << std::endl;
    MainWindow mainWindow;
    
    std::cerr << "Showing MainWindow..." << std::endl;
    mainWindow.show();
    
    std::cerr << "Entering event loop..." << std::endl;
    int result = app.exec();
    
    std::cerr << "Cleaning up..." << std::endl;
    avformat_network_deinit();
    
    return result;
}
