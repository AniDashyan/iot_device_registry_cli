#include <iostream>
#include <sstream>
#include <string>
#include <functional>
#include <variant>
#include <ranges>
#include "Device.h"

struct LineArgs {
    std::string command;
    std::vector<std::string> args;
};

void Parseline(std::string& line, LineArgs& lineArgs) {
    std::istringstream ss(line);
    ss >> lineArgs.command;
    std::string arg;
    while (ss >> arg) {
        lineArgs.args.push_back(arg);
    }
}

void PrintHelp() {
    std::cout << "Commands Usage\n";
    
    std::cout << "add <id> <name> <type> <priority> [tags...] - Adds a device to registry" << std::endl;
    std::cout << "remove <id> — Removes a device from registry" << std::endl;
    std::cout << "list - Prints all the devices in registry sorted by names\n";
    std::cout << "list-by-priority - Print all devices sorted by priority (descending)\n";
    std::cout << "find-tag <tag> - Finds and prints all devices matching a tag.\n";
    std::cout << "list-high-priority - Finds and prints all devices with priority ≥ minPriority\n";
    std::cout << "alert - Prints the top 3 highest priority devices\n";
    std::cout << "stats - Prints stats about registry\n";
    std::cout << "transform-names <prefix> - Adds a given prefixes to all device names in registry\n";
    std::cout << "quit - Exits\n";
}

using CommandFunc = std::function<void(DeviceRegistry&, LineArgs&)>;
using CommandMap = std::unordered_map<std::string, CommandFunc>;

void ExecuteCommand(DeviceRegistry & deviceReg, [[maybe_unused]] LineArgs & deviceArgs) {
    CommandMap commandMap = {
        {"add", [](DeviceRegistry& deviceReg, LineArgs& deviceArgs) {
            Device device;
            std::vector<std::string> tags;
            int priortiy;

            if (deviceArgs.args.size() != 5) {

                std::cout << "Not enough arguments for 'add' command. Usage: add <id> <name> <type> <priority> [tags...]" << std::endl;
                return;
            }

            tags = std::views::split(deviceArgs.args[4], ',') | std::ranges::to<std::vector<std::string>>();
            priortiy = std::stoi(deviceArgs.args[3]);
            if (priortiy < 0 || priortiy > 10) {
                std::cout << "Error: priority must be a number in [1, 10] interval.\n";
                return ;
            }

            device.id = deviceArgs.args[0];
            device.name = deviceArgs.args[1];
            device.type = deviceArgs.args[2];
            device.priority = priortiy;
            device.tags = std::set<std::string>(tags.begin(), tags.end());
            device.isOnline = true;
            if (deviceReg.AddDevice(device)) {
                std::cout << "Device was added to registry successfully!\n";
            }
            else {
                std::cout << "Failed to add a device, it already exists!!\n";
            }
        }},

        {"remove", [](DeviceRegistry &deviceReg, LineArgs & deviceArgs) {
            if (deviceReg.RemoveDevice(deviceArgs.args[0])) {
                std::cout << "Device was removed from registry successfully!!\n";
            }
            else {
                std::cout << "Failed to remove the device\n";
            }
        }},

        {"list", [](DeviceRegistry &deviceReg, [[maybe_unused]] LineArgs & deviceArgs) {
            auto devices = deviceReg.GetAllDevices();
            std::sort(devices.begin(), devices.end(), [](Device &dev1, Device &dev2) {
                return dev1.name < dev2.name;
            });
            std::for_each(devices.begin(), devices.end(), [](const Device &device){
                device.PrintDevice();
            });
        }
        },

        {"list-by-priority", [](DeviceRegistry &deviceReg, [[maybe_unused]] LineArgs & deviceArgs) {
            auto devices = deviceReg.GetAllDevices();
            std::sort(devices.begin(), devices.end(), [](Device &dev1, Device &dev2) {
                return dev1.priority > dev2.priority;
            });
            std::for_each(devices.begin(), devices.end(), [](const Device &device){
                device.PrintDevice();
            });
        }},

        {"find-tag", [](DeviceRegistry &deviceReg, LineArgs &deviceArgs){
            auto devices = deviceReg.GetDeviceByTagName(deviceArgs.args[0]);
            std::for_each(devices.begin(), devices.end(), [](const Device &dev) {
                dev.PrintDevice();
            });
        }},

        {"list-high-priority", [](DeviceRegistry &deviceReg, LineArgs &deviceArgs){ 
            auto devices = deviceReg.GetAllDevices();
            int priority;

            if (deviceArgs.args.empty()) {
                std::cout << "Usage: list-high-priority <priority>" << std::endl;
                return;
            }
            priority = std::stoi(deviceArgs.args[0]);
            if (priority < 0 || priority > 10) {
                std::cout << "Error: priority must be a number in [1, 10] interval.";
                return ;
            }

            std::vector<Device> priorityDevices;
            std::copy_if(devices.begin(), devices.end(), std::back_inserter(priorityDevices), [=](const Device & dev){
                return dev.priority >= priority;
            });
            std::for_each(priorityDevices.begin(), priorityDevices.end(), [](const Device &device){
                device.PrintDevice();
            });
        }},

        {"alert", [](DeviceRegistry &deviceReg, [[maybe_unused]] LineArgs & deviceArgs){
            deviceReg.Alert();
        }},

        {"stats", [](DeviceRegistry &deviceReg, [[maybe_unused]] LineArgs & deviceArgs) {
            Stats stats = deviceReg.GetStats();
            std::cout << "Total Device Count: " << stats.totalDeviceCount << "\n";
            std::cout << "Online Devices: " << stats.onLineDeviceCount << "\n";
            std::cout << "Average Priority: " << stats.averagePriority << "\n";
        }},

        {"transform-names", [](DeviceRegistry &deviceReg, LineArgs &deviceArgs) {
            if (deviceArgs.args.empty()) {
                std::cout << "Usage: transform-names <prefix>" << std::endl;
                return;
            }
            deviceReg.TransformNames(deviceArgs.args[0]);
        }}, 
    };

    auto it = commandMap.find(deviceArgs.command);
    if (it == commandMap.end()) {
        std::cout << "Invalid command: " << deviceArgs.command << std::endl; 
        return ;
    }
    it->second(deviceReg, deviceArgs);
}


int main() {
    DeviceRegistry deviceReg;
    std::string line;

    std::cout << " !!Welcome to the IoT Device Registry CLI!! " << std::endl;
    std::cout << "Since the DeviceRegistry is empty now it's recommended to start by adding some devices. You can use the 'add' command to do so." << std::endl;
    std::cout << "Type 'help' to see the list of available commands." << std::endl;
    while (std::cout << "> " && std::getline(std::cin, line)) {
        LineArgs lineArgs;
        Parseline(line, lineArgs);
        if (lineArgs.command == "quit") {
            break;
        }
        else if (lineArgs.command == "help") {
            PrintHelp();
            continue;
        }

        ExecuteCommand(deviceReg, lineArgs);
    }
    return 0;
} 