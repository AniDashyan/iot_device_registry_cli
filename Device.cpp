#include "Device.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <iterator>

void Device::PrintDevice() const {
    std::cout << "Id: " << id << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Type: " << type << std::endl;
    std::cout << "Priority: " << priority << std::endl;
    std::cout << "Tags: ";
    std::copy(tags.begin(), tags.end(), std::ostream_iterator<std::string>(std::cout, ","));
    std::cout << std::endl;
    std::cout << std::endl;
}

bool DeviceRegistry::AddDevice(Device& device) {
    if (mRegisteredDevices.find(device.id) != mRegisteredDevices.end()) {
        return false;
    }

    mRegisteredDevices[device.id] = device;
    mUniqueDeviceTypes.insert(device.type);

    for (const auto& tag : device.tags) {
        mTagToDeviceMap[tag].push_back(device.id);
    }

    mAlertQueue.push(device);
    return true;
}

bool DeviceRegistry::RemoveDevice(const std::string& deviceId) {
    if (mRegisteredDevices.find(deviceId) ==  mRegisteredDevices.end()) {
        return false;
    }
  
    const Device& device = mRegisteredDevices.at(deviceId);
    for (const auto& tag : device.tags) {
        auto it = mTagToDeviceMap.find(tag);
        if (it == mTagToDeviceMap.end()) {
            continue;
        }
        auto& ids = it->second;
        ids.erase(std::remove(ids.begin(), ids.end(), deviceId), ids.end());
        if (ids.empty()) {
            mTagToDeviceMap.erase(it);
        }
    }

    const std::string type = device.type;
    mRegisteredDevices.erase(deviceId);

    bool typeStillUsed = std::any_of(mRegisteredDevices.begin(), mRegisteredDevices.end(),
                                     [&type](const auto& entry) { return entry.second.type == type; });
    if (!typeStillUsed) {
        mUniqueDeviceTypes.erase(type);
    }
    return true;
}

Device * DeviceRegistry::GetDevice(const std::string& deviceId) {
   auto it = mRegisteredDevices.find(deviceId);
   if (it == mRegisteredDevices.end()) {
       return nullptr;
   }
   return &(it->second);
}

std::vector<Device> DeviceRegistry::GetAllDevices() const {
    std::vector<Device> devices;
    devices.reserve(mRegisteredDevices.size());
    std::transform(mRegisteredDevices.begin(), mRegisteredDevices.end(), std::back_inserter(devices),
                   [](const auto& device) { return device.second; });

    return devices;
}

std::vector<Device> DeviceRegistry::GetDeviceByTagName(std::string tag) const {
    std::vector<Device> devices;
    auto it = mTagToDeviceMap.find(tag);
    if (it == mTagToDeviceMap.end()) {
        return devices;
    }

    const auto& deviceIds = it->second;
    devices.reserve(deviceIds.size());
    std::transform(deviceIds.begin(), deviceIds.end(), std::back_inserter(devices), [this](const std::string& deviceId){
        return mRegisteredDevices.at(deviceId);
    });
    return devices;
}

void DeviceRegistry::Alert() {
    for (int i = 0; i < 3; i++) {
        if (mAlertQueue.empty()) {
            break;
        }
        auto dev = mAlertQueue.top();
        dev.PrintDevice();
        mAlertQueue.pop();
    }
}

Stats DeviceRegistry::GetStats() {
   const auto devices = GetAllDevices();
   mStats.totalDeviceCount = static_cast<int>(devices.size());
   mStats.onLineDeviceCount = static_cast<int>(std::count_if(devices.begin(), devices.end(), [](const Device& dev) {
       return dev.isOnline;
   }));
   mStats.uniqueDeviceTypes = mUniqueDeviceTypes;
   mStats.averagePriority = devices.empty() ? 0 :
       std::accumulate(devices.begin(), devices.end(), 0, [](int sum, const Device& device ) { return sum + device.priority; }) / mStats.totalDeviceCount;
   return mStats;
}

void DeviceRegistry::TransformNames(std::string prefix) {
    std::for_each(mRegisteredDevices.begin(), mRegisteredDevices.end(), [&prefix](auto& device) {
        device.second.name = prefix + device.second.name;
    });
}