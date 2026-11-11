#pragma once
#include <QString>

class SystemMonitor {
public:
    SystemMonitor() = default;
    virtual ~SystemMonitor() = default;

    static SystemMonitor* create();

    virtual QString getCpuUsage() = 0;
    virtual QString getRamUsage() = 0;
    virtual QString getVramUsage() = 0;
    virtual QString getAppMemory() = 0;
};
