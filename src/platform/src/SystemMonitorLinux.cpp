#include "SystemMonitor.h"
#include <QFile>
#include <QByteArray>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <unistd.h>

class SystemMonitorLinux : public SystemMonitor {
public:
    SystemMonitorLinux() {
        readCpuStats(m_lastTotal, m_lastIdle);
        detectBestGpu();
    }

    QString getCpuUsage() override {
        unsigned long long currentTotal = 0, currentIdle = 0;
        if (!readCpuStats(currentTotal, currentIdle)) {
            return "0%";
        }

        unsigned long long totalDiff = currentTotal - m_lastTotal;
        unsigned long long idleDiff  = currentIdle - m_lastIdle;

        m_lastTotal = currentTotal;
        m_lastIdle  = currentIdle;

        if (totalDiff == 0) {
            return "0%";
        }

        double usage = (static_cast<double>(totalDiff - idleDiff) * 100.0) / static_cast<double>(totalDiff);
        return QString::number(qBound(0, static_cast<int>(usage + 0.5), 100)) + "%";
    }

    QString getRamUsage() override {
        QFile memFile("/proc/meminfo");
        if (!memFile.open(QIODevice::ReadOnly)) {
            return "0.0 GB";
        }

        const QByteArray content = memFile.readAll();
        const QList<QByteArray> lines = content.split('\n');

        long long totalMemKB = 0;
        long long availMemKB = 0;

        for (const QByteArray &line : lines) {
            if (line.startsWith("MemTotal:")) {
                totalMemKB = extractKBValue(line);
            } else if (line.startsWith("MemAvailable:")) {
                availMemKB = extractKBValue(line);
            }
            if (totalMemKB > 0 && availMemKB > 0) {
                break;
            }
        }

        if (totalMemKB > 0 && availMemKB > 0) {
            double usedGB = static_cast<double>(totalMemKB - availMemKB) / (1024.0 * 1024.0);
            double totalGB = static_cast<double>(totalMemKB) / (1024.0 * 1024.0);
            return QString("%1 / %2 GB")
                .arg(QString::number(usedGB, 'f', 1),
                     QString::number(totalGB, 'f', 1));
        }
        return "0.0 GB";
    }

    QString getVramUsage() override {
        if (m_vramUsedPath.isEmpty()) {
            detectBestGpu();
        }

        if (!m_vramUsedPath.isEmpty()) {
            QFile vramFile(m_vramUsedPath);
            if (vramFile.open(QIODevice::ReadOnly)) {
                long long vramBytes = vramFile.readAll().trimmed().toLongLong();
                long long vramMB = vramBytes / (1024 * 1024);

                if (m_vramTotalMB > 0) {
                    return QString("%1 / %2 MB").arg(vramMB).arg(m_vramTotalMB);
                }
                return QString::number(vramMB) + " MB";
            }
        }
        return "N/A";
    }

    QString getAppMemory() override {
        QFile statmFile("/proc/self/statm");
        if (statmFile.open(QIODevice::ReadOnly)) {
            const QList<QByteArray> parts = statmFile.readAll().trimmed().split(' ');
            if (parts.size() >= 2) {
                long long rssPages = parts[1].toLongLong();
                long pageSizeKB = sysconf(_SC_PAGESIZE) / 1024;
                long long rssMB = (rssPages * pageSizeKB) / 1024;
                return QString::number(rssMB) + " MB";
            }
        }
        return "0 MB";
    }

private:
    unsigned long long m_lastTotal = 0;
    unsigned long long m_lastIdle  = 0;
    QString m_vramUsedPath;
    long long m_vramTotalMB = 0;

    void detectBestGpu() {
        QDir drmDir("/sys/class/drm");
        // Filtrar solo card0, card1, card2... (ignorando conectores como card0-DP-1)
        const QStringList cards = drmDir.entryList(QStringList() << "card[0-9]*", QDir::Dirs | QDir::NoDotAndDotDot);

        long long bestScore = -1;
        QString bestUsedPath;
        long long bestTotalMB = 0;

        for (const QString &card : cards) {
            if (card.contains('-')) continue; // Ignorar salidas de video (ej. card0-HDMI-A-1)

            QString basePath = QString("/sys/class/drm/%1/device").arg(card);
            QString usedPath = basePath + "/mem_info_vram_used";
            QString totalPath = basePath + "/mem_info_vram_total";
            QString visTotalPath = basePath + "/mem_info_vis_vram_total";

            if (!QFileInfo::exists(usedPath)) continue;

            long long totalBytes = readSysfsNumber(totalPath);
            long long visTotalBytes = readSysfsNumber(visTotalPath);

            // Criterio para priorizar GPU dedicada sobre integrada (APU):
            // 1. En una GPU dedicada (como tu RX 5700 XT), mem_info_vram_total es mayor (8 GB)
            //    y normalmente en APUs Ryzen (como el 5700G) el carveout es pequeño (512MB - 4GB).
            // 2. Además, en GPUs dedicadas el symlink de /sys/class/drm/cardX/device atraviesa
            //    un puente PCIe adicional (mayor profundidad en /sys/devices/pci...), mientras
            //    que la APU está integrada en el bus interno del SoC.
            QString canonicalDevicePath = QFileInfo(basePath).canonicalFilePath();
            int pciDepth = canonicalDevicePath.count('/');

            // Puntuamos combinando la cantidad de VRAM total + un bonus si cuelga de un bridge PCIe externo
            long long score = totalBytes + (static_cast<long long>(pciDepth) * 512LL * 1024LL * 1024LL);

            // Si mem_info_vram_total es mayor al visible o mayor a 4GB, es 100% dedicada
            if (totalBytes > 4LL * 1024 * 1024 * 1024 || totalBytes > visTotalBytes) {
                score += 100LL * 1024 * 1024 * 1024;
            }

            if (score > bestScore) {
                bestScore = score;
                bestUsedPath = usedPath;
                bestTotalMB = totalBytes / (1024 * 1024);
            }
        }

        m_vramUsedPath = bestUsedPath;
        m_vramTotalMB  = bestTotalMB;
    }

    static long long readSysfsNumber(const QString &path) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            return file.readAll().trimmed().toLongLong();
        }
        return 0;
    }

    static long long extractKBValue(const QByteArray &line) {
        const QList<QByteArray> parts = line.simplified().split(' ');
        if (parts.size() >= 2) {
            return parts[1].toLongLong();
        }
        return 0;
    }

    static bool readCpuStats(unsigned long long &totalOut, unsigned long long &idleOut) {
        QFile statFile("/proc/stat");
        if (!statFile.open(QIODevice::ReadOnly)) {
            return false;
        }

        QByteArray firstLine = statFile.readLine().simplified();
        const QList<QByteArray> parts = firstLine.split(' ');
        if (parts.size() < 5) {
            return false;
        }

        unsigned long long user    = parts.value(1).toULongLong();
        unsigned long long nice    = parts.value(2).toULongLong();
        unsigned long long system  = parts.value(3).toULongLong();
        unsigned long long idle    = parts.value(4).toULongLong();
        unsigned long long iowait  = parts.value(5).toULongLong();
        unsigned long long irq     = parts.value(6).toULongLong();
        unsigned long long softirq = parts.value(7).toULongLong();
        unsigned long long steal   = parts.value(8).toULongLong();

        idleOut  = idle + iowait;
        totalOut = user + nice + system + idle + iowait + irq + softirq + steal;
        return true;
    }
};

SystemMonitor* SystemMonitor::create() {
    return new SystemMonitorLinux();
}
