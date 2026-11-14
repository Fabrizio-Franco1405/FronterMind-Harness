#include "SystemMonitor.h"
#include <windows.h>
#include <psapi.h>
#include <dxgi1_4.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class WindowsMonitor : public SystemMonitor {
public:
    WindowsMonitor() {
        FILETIME idleTime, kernelTime, userTime;
        if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
            m_prevIdleTime = (static_cast<quint64>(idleTime.dwHighDateTime) << 32) | idleTime.dwLowDateTime;
            m_prevKernelTime = (static_cast<quint64>(kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
            m_prevUserTime = (static_cast<quint64>(userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;
        }
    }

    QString getCpuUsage() override {
        FILETIME idleTime, kernelTime, userTime;
        if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
            return QStringLiteral("0%");
        }

        quint64 idle = (static_cast<quint64>(idleTime.dwHighDateTime) << 32) | idleTime.dwLowDateTime;
        quint64 kernel = (static_cast<quint64>(kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
        quint64 user = (static_cast<quint64>(userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;

        quint64 idleDiff = idle - m_prevIdleTime;
        quint64 totalDiff = (kernel - m_prevKernelTime) + (user - m_prevUserTime);

        m_prevIdleTime = idle;
        m_prevKernelTime = kernel;
        m_prevUserTime = user;

        if (totalDiff == 0) return QStringLiteral("0%");

        int cpuPercent = static_cast<int>(((totalDiff - idleDiff) * 100) / totalDiff);
        return QStringLiteral("%1%").arg(qBound(0, cpuPercent, 100));
    }

    QString getRamUsage() override {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (!GlobalMemoryStatusEx(&memInfo)) {
            return QStringLiteral("0 / 0 GB");
        }

        double totalGB = static_cast<double>(memInfo.ullTotalPhys) / (1024.0 * 1024.0 * 1024.0);
        double usedGB = static_cast<double>(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0);

        return QStringLiteral("%1 / %2 GB")
            .arg(usedGB, 0, 'f', 1)
            .arg(totalGB, 0, 'f', 1);
    }

    QString getAppMemory() override {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
            quint64 usedMB = pmc.WorkingSetSize / (1024 * 1024);
            return QStringLiteral("%1 MB").arg(usedMB);
        }
        return QStringLiteral("0 MB");
    }

    QString getVramUsage() override {
        static HQUERY hQuery = nullptr;
        static HCOUNTER hCounter = nullptr;
        static bool pdhInitialized = false;

        if (!pdhInitialized) {
            if (PdhOpenQueryW(nullptr, 0, &hQuery) == ERROR_SUCCESS) {
                if (PdhAddEnglishCounterW(hQuery, L"\\GPU Adapter Memory(*)\\Dedicated Usage", 0, &hCounter) == ERROR_SUCCESS) {
                    PdhCollectQueryData(hQuery);
                    pdhInitialized = true;
                }
            }
        }

        ComPtr<IDXGIFactory4> factory;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
            return QStringLiteral("0 / 0 MB");
        }

        UINT64 bestTotalBytes = 0;
        UINT64 dxgiUsedBytes = 0;
        ComPtr<IDXGIAdapter1> adapter;

        for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 desc;
            if (SUCCEEDED(adapter->GetDesc1(&desc))) {
                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

                if (desc.DedicatedVideoMemory > bestTotalBytes) {
                    bestTotalBytes = desc.DedicatedVideoMemory;
                    ComPtr<IDXGIAdapter3> adapter3;
                    if (SUCCEEDED(adapter.As(&adapter3))) {
                        DXGI_QUERY_VIDEO_MEMORY_INFO memInfo{};
                        if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo))) {
                            dxgiUsedBytes = memInfo.CurrentUsage;
                        }
                    }
                }
            }
            adapter.Reset();
        }

        UINT64 globalUsedBytes = 0;
        if (pdhInitialized && PdhCollectQueryData(hQuery) == ERROR_SUCCESS) {
            DWORD bufferSize = 0;
            DWORD itemCount = 0;
            if (PdhGetFormattedCounterArrayW(hCounter, PDH_FMT_LARGE, &bufferSize, &itemCount, nullptr) == PDH_MORE_DATA) {
                std::vector<BYTE> buffer(bufferSize);
                auto *items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W *>(buffer.data());
                if (PdhGetFormattedCounterArrayW(hCounter, PDH_FMT_LARGE, &bufferSize, &itemCount, items) == ERROR_SUCCESS) {
                    for (DWORD i = 0; i < itemCount; ++i) {
                        if (items[i].FmtValue.CStatus == ERROR_SUCCESS) {
                            UINT64 val = static_cast<UINT64>(items[i].FmtValue.largeValue);
                            if (val > globalUsedBytes) {
                                globalUsedBytes = val;
                            }
                        }
                    }
                }
            }
        }

        const UINT64 finalUsedMB = (globalUsedBytes > dxgiUsedBytes ? globalUsedBytes : dxgiUsedBytes) / (1024 * 1024);
        const UINT64 totalMB = bestTotalBytes / (1024 * 1024);

        return QStringLiteral("%1 / %2 MB").arg(finalUsedMB).arg(totalMB);
    }

private:
    quint64 m_prevIdleTime = 0;
    quint64 m_prevKernelTime = 0;
    quint64 m_prevUserTime = 0;
};

SystemMonitor* SystemMonitor::create() {
    return new WindowsMonitor();
}
