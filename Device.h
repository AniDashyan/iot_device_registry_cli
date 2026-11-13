#pragma once
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <compare>
#include <algorithm>
#include <vector>

struct Device {
    std::string id;
    std::string name;
    std::string type;
    std::set<std::string> tags;
    int priority;
    bool isOnline = true;

    void PrintDevice() const;
    auto operator<=>(const Device& other) const = default;
};

struct Stats {
        int totalDeviceCount;
        int onLineDeviceCount;
        std::set<std::string> uniqueDeviceTypes;
        int averagePriority;
};

class DeviceRegistry {
public:
    DeviceRegistry() = default;

    bool AddDevice(Device& device);
    bool RemoveDevice(const std::string& deviceId);
    Device * GetDevice(const std::string& deviceId);
    std::vector<Device> GetAllDevices() const;
    std::vector<Device> GetDeviceByTagName(std::string tag) const;
    void Alert();
    Stats GetStats();
    void TransformNames(std::string prefix);


    private:
        std::map<std::string, Device> mRegisteredDevices;
        std::unordered_map<std::string, std::vector<std::string>> mTagToDeviceMap;
        std::priority_queue<Device, std::vector<Device>> mAlertQueue;
        std::set<std::string> mUniqueDeviceTypes;
        Stats mStats;
};