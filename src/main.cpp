#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <mutex>
#include "ui/main_window.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

static std::ofstream g_logFile;
// qWarning 등은 StreamReceiver 워커 스레드에서도 호출된다. ofstream 은 스레드 안전하지 않다.
static std::mutex g_logMutex;

static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    Q_UNUSED(context);

    const char* prefix = "[INFO]";
    switch (type) {
    case QtDebugMsg:    prefix = "[DEBUG]"; break;
    case QtWarningMsg:  prefix = "[WARN]";  break;
    case QtCriticalMsg: prefix = "[ERROR]"; break;
    case QtFatalMsg:    prefix = "[FATAL]"; break;
    case QtInfoMsg:     prefix = "[INFO]";  break;
    }

    std::string line = std::string(prefix) + " " + msg.toStdString();

    std::lock_guard<std::mutex> lock(g_logMutex);
    std::cerr << line << std::endl;
    if (g_logFile.is_open()) {
        g_logFile << line << std::endl;
    }
}

// 로그는 실행 디렉터리가 아니라 앱 데이터 폴더(%APPDATA%/VMS/VMS)에 쓴다.
// Program Files 에 배포하면 실행 디렉터리에는 쓰기 권한이 없다.
static void openLogFile() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!dir.isEmpty() && QDir().mkpath(dir)) {
        QString path = QDir(dir).filePath("vms_debug.log");
        g_logFile.open(std::filesystem::path(path.toStdWString()), std::ios::out | std::ios::trunc);
    }
    if (!g_logFile.is_open()) {
        g_logFile.open("vms_debug.log", std::ios::out | std::ios::trunc);
    }
}

int main(int argc, char *argv[]) {
    // QStandardPaths 가 앱 이름을 알아야 하므로 QApplication 생성 전에 정적으로 설정한다.
    QCoreApplication::setOrganizationName("VMS");
    QCoreApplication::setOrganizationDomain("vms.local");
    QCoreApplication::setApplicationName("VMS");
    QCoreApplication::setApplicationVersion("1.0.0");

    openLogFile();
    qInstallMessageHandler(customMessageHandler);

    qInfo() << "Starting VMS...";

    avformat_network_init();

    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    int result = app.exec();

    qInfo() << "Cleaning up...";
    avformat_network_deinit();

    return result;
}
