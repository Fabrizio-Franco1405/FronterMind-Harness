#include "SystemMonitor.h"
#include <windows.h>
#include <psapi.h>

class SystemMonitorWindows : public SystemMonitor {
public:
    SystemMonitorWindows() = default;

    QString getCpuUsage() override {
        static FILETIME prevIdle, prevKernel, prevUser;
        FILETIME idle, kernel, user;
        if (GetSystemTimes(&idle, &kernel, &user)) {
            ULARGE_INTEGER i, k, u, pi, pk, pu;
            i.LowPart = idle.dwLowDateTime; i.HighPart = idle.dwHighDateTime;
            k.LowPart = kernel.dwLowDateTime; k.HighPart = kernel.dwHighDateTime;
            u.LowPart = user.dwLowDateTime; u.HighPart = user.dwHighDateTime;
            pi.LowPart = prevIdle.dwLowDateTime; pi.HighPart = prevIdle.dwHighDateTime;
            pk.LowPart = prevKernel.dwLowDateTime; pk.HighPart = prevKernel.dwHighDateTime;
            pu.LowPart = prevUser.dwLowDateTime; pu.HighPart = prevUser.dwHighDateTime;
            ULONGLONG idleDiff = i.QuadPart - pi.QuadPart, kernelDiff = k.QuadPart - pk.QuadPart, userDiff = u.QuadPart - pu.QuadPart;
            prevIdle = idle; prevKernel = kernel; prevUser = user;
            ULONGLONG total = kernelDiff + userDiff;
            if (total > 0) return QString::number((total - idleDiff) * 100 / total) + "%";
        }
        return "0%";
    }

    QString getRamUsage() override {
        MEMORYSTATUSEX memInfo; memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            DWORDLONG usedPhys = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024);
            return QString::number(usedPhys / 1024.0, 'f', 1) + " GB";
        }
        return "0 GB";
    }

    QString getVramUsage() override { return "N/A"; }

    QString getAppMemory() override {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            return QString::number(pmc.WorkingSetSize / (1024 * 1024)) + " MB";
        }
        return "0 MB";
    }
};

SystemMonitor* SystemMonitor::create() { return new SystemMonitorWindows(); }
