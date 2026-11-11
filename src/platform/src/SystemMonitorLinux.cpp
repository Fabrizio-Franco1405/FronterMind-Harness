#include "SystemMonitor.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

class SystemMonitorLinux : public SystemMonitor {
public:
    SystemMonitorLinux() = default;

    QString getCpuUsage() override {
        static long lastUser = 0, lastNice = 0, lastSystem = 0, lastIdle = 0;
        QFile statFile("/proc/stat");
        if (statFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&statFile);
            QStringList parts = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() > 4) {
                long user = parts[1].toLong(), nice = parts[2].toLong(), system = parts[3].toLong(), idle = parts[4].toLong();
                long total = (user - lastUser) + (nice - lastNice) + (system - lastSystem) + (idle - lastIdle);
                long work = (user - lastUser) + (nice - lastNice) + (system - lastSystem);
                lastUser = user; lastNice = nice; lastSystem = system; lastIdle = idle;
                if (total > 0) return QString::number((work * 100) / total) + "%";
            }
        }
        return "0%";
    }

    QString getRamUsage() override {
        QFile memFile("/proc/meminfo");
        if (memFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&memFile);
            long totalMem = 0, availMem = 0;

            // Regex estricta: Busca el nombre, luego espacios, luego captura el número
            QRegularExpression totalRe("MemTotal:\\s+(\\d+)");
            QRegularExpression availRe("MemAvailable:\\s+(\\d+)");

            while (!in.atEnd()) {
                QString line = in.readLine();

                QRegularExpressionMatch match = totalRe.match(line);
                if (match.hasMatch()) totalMem = match.captured(1).toLong();

                match = availRe.match(line);
                if (match.hasMatch()) availMem = match.captured(1).toLong();
            }

            if (totalMem > 0 && availMem > 0) {
                double usedGB = (totalMem - availMem) / 1024.0 / 1024.0;
                return QString::number(usedGB, 'f', 1) + " GB";
            }
        }
        return "0 GB";
    }

    QString getVramUsage() override {
        QFile vramFile("/sys/class/drm/card1/device/mem_info_vram_used");
        if (!vramFile.exists()) vramFile.setFileName("/sys/class/drm/card0/device/mem_info_vram_used");

        if (vramFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&vramFile);
            long long vramBytes = in.readLine().toLongLong();
            return QString::number(vramBytes / (1024 * 1024)) + " MB";
        }
        return "N/A";
    }

    QString getAppMemory() override {
        QFile statusFile("/proc/self/status");
        if (statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&statusFile);
            QRegularExpression vmRe("VmRSS:\\s+(\\d+)");

            while (!in.atEnd()) {
                QString line = in.readLine();
                QRegularExpressionMatch match = vmRe.match(line);
                if (match.hasMatch()) {
                    return QString::number(match.captured(1).toLong() / 1024) + " MB";
                }
            }
        }
        return "0 MB";
    }
};

SystemMonitor* SystemMonitor::create() { return new SystemMonitorLinux(); }
