#include "SystemMonitor.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class SystemMonitorWindows : public SystemMonitor {
public:
    SystemMonitorWindows() {
        // Inicializar primera lectura de CPU para evitar pico falso en el arranque
        readCpuStats(m_lastIdle, m_lastKernel, m_lastUser);
    }

    QString getCpuUsage() override {
        unsigned long long idle = 0, kernel = 0, user = 0;
        if (!readCpuStats(idle, kernel, user)) {
            return QStringLiteral("0%");
        }

        const unsigned long long idleDiff   = idle   - m_lastIdle;
        const unsigned long long kernelDiff = kernel - m_lastKernel;
        const unsigned long long userDiff   = user   - m_lastUser;

        m_lastIdle   = idle;
        m_lastKernel = kernel;
        m_lastUser   = user;

        // En Windows, kernelTime ya incluye el tiempo de idleTime
        const unsigned long long totalSys = kernelDiff + userDiff;
        if (totalSys == 0) {
            return QStringLiteral("0%");
        }

        const unsigned long long workTime = (totalSys >= idleDiff) ? (totalSys - idleDiff) : 0;
        const double usage = (static_cast<double>(workTime) * 100.0) / static_cast<double>(totalSys);
        return QString::number(qBound(0, static_cast<int>(usage + 0.5), 100)) + QStringLiteral("%");
    }

    QString getRamUsage() override {
        MEMORYSTATUSEX memInfo{};
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            const double totalGB = static_cast<double>(memInfo.ullTotalPhys) / (1024.0 * 1024.0 * 1024.0);
            const double usedGB  = static_cast<double>(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0);
            return QStringLiteral("%1 / %2 GB")
                .arg(QString::number(usedGB, 'f', 1),
                     QString::number(totalGB, 'f', 1));
        }
        return QStringLiteral("0.0 GB");
    }

    QString getVramUsage() override {
        ComPtr<IDXGIFactory4> factory;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
            return QStringLiteral("N/A");
        }

        ComPtr<IDXGIAdapter3> bestAdapter;
        SIZE_T bestDedicatedVram = 0;

        ComPtr<IDXGIAdapter1> adapter1;
        for (UINT i = 0; factory->EnumAdapters1(i, &adapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 desc{};
            if (FAILED(adapter1->GetDesc1(&desc))) {
                adapter1.Reset();
                continue;
            }

            // Ignorar el adaptador por software (Microsoft Basic Render Driver)
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                adapter1.Reset();
                continue;
            }

            // Priorizar la GPU con mayor VRAM dedicada (dGPU > iGPU)
            if (desc.DedicatedVideoMemory >= bestDedicatedVram) {
                ComPtr<IDXGIAdapter3> adapter3;
                if (SUCCEEDED(adapter1.As(&adapter3))) {
                    bestDedicatedVram = desc.DedicatedVideoMemory;
                    bestAdapter = adapter3;
                }
            }
            adapter1.Reset();
        }

        if (bestAdapter) {
            DXGI_QUERY_VIDEO_MEMORY_INFO memInfo{};
            if (SUCCEEDED(bestAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo))) {
                const long long usedMB  = static_cast<long long>(memInfo.CurrentUsage / (1024 * 1024));
                const long long totalMB = static_cast<long long>(bestDedicatedVram / (1024 * 1024));
                if (totalMB > 0) {
                    return QStringLiteral("%1 / %2 MB").arg(usedMB).arg(totalMB);
                }
                return QString::number(usedMB) + QStringLiteral(" MB");
            }
        }

        return QStringLiteral("N/A");
    }

    QString getAppMemory() override {
        PROCESS_MEMORY_COUNTERS_EX pmc{};
        if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
            const long long usedMB = static_cast<long long>(pmc.WorkingSetSize / (1024 * 1024));
            return QString::number(usedMB) + QStringLiteral(" MB");
        }
        return QStringLiteral("0 MB");
    }

private:
    unsigned long long m_lastIdle   = 0;
    unsigned long long m_lastKernel = 0;
    unsigned long long m_lastUser   = 0;

    static unsigned long long fileTimeToUInt64(const FILETIME &ft) {
        ULARGE_INTEGER uli;
        uli.LowPart  = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        return uli.QuadPart;
    }

    static bool readCpuStats(unsigned long long &idleOut, unsigned long long &kernelOut, unsigned long long &userOut) {
        FILETIME idle{}, kernel{}, user{};
        if (!GetSystemTimes(&idle, &kernel, &user)) {
            return false;
        }
        idleOut   = fileTimeToUInt64(idle);
        kernelOut = fileTimeToUInt64(kernel);
        userOut   = fileTimeToUInt64(user);
        return true;
    }
};

SystemMonitor* SystemMonitor::create() {
    return new SystemMonitorWindows();
}
#endif
