#include <windows.h>
#include <iostream>
#include <deque>
#include <tchar.h>


using namespace std;


struct DiskInfo {
    TCHAR name[8];
    DWORD system_flags;
    DWORD serial_number;
    TCHAR file_system[10];
    __int64 total_space;
    __int64 free_space;
};

deque<DiskInfo> getDisksInfo();

void printInfo(deque<DiskInfo> deque);

int main(int argc, char** argv) {
    deque<DiskInfo> info = getDisksInfo();
    printInfo(info);
}

void printInfo(deque<DiskInfo> deque) {
    cout << "---------------------------------------------------------\n";
    for (auto& i : deque) {
        cout << "Disk name : " << i.name << endl ;
        cout << "System flags : " << i.system_flags << endl ;
        cout << "Serial number : " << i.serial_number << endl ;
        cout << "File system : " << i.file_system << endl ;
        cout << "Total space : " << i.total_space / (1024 * 1024) << " MB" << endl ;
        cout << "Free space : " << i.free_space / (1024 * 1024) << " MB" << endl ;
        cout << "---------------------------------------------------------\n";
    }
}

deque<DiskInfo> getDisksInfo() {
    deque<DiskInfo> disks_info_queue;

    DWORD dwSize = GetLogicalDriveStrings(0, nullptr);
    auto disk_names_arr = new TCHAR [ dwSize+1 ];
    GetLogicalDriveStrings(dwSize, disk_names_arr);

    for (LPTSTR p = disk_names_arr; *p; p += lstrlen(p) + 1) {
        DiskInfo disk_info{};

        _tcscpy_s(disk_info.name, 4, p);
        GetVolumeInformation(disk_info.name, nullptr, 0, &disk_info.serial_number, nullptr, &disk_info.system_flags,
                             disk_info.file_system,
                             sizeof(disk_info.file_system));
        GetDiskFreeSpaceEx(disk_info.name, (PULARGE_INTEGER) &disk_info.free_space,
                           (PULARGE_INTEGER) &disk_info.total_space,
                           nullptr);

        disks_info_queue.push_back(disk_info);
    }

    return disks_info_queue;  //возвращаем очередь
}
