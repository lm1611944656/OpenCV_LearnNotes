#include "jsonparse.h"
#include <iostream>

int main() {
    CJsonParse parser("config/ElevatorCameraList.json");
    TCamerasInfo_t camerasInfo = parser.getCamerasInfo();

    for (int i = 0; i < camerasInfo.camerasNums; ++i) {
        printf("Camera %d:\n", i + 1);
        printf("  IP Address: %s\n", camerasInfo.camerasBaseInfo[i].cameraIPAddr);
        printf("  Port: %s\n", camerasInfo.camerasBaseInfo[i].cameraPort);
        printf("  Username: %s\n", camerasInfo.camerasBaseInfo[i].cameraUserName);
        printf("  Password: %s\n", camerasInfo.camerasBaseInfo[i].cameraPasswd);
    }

    return 0;
}