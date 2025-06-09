// main은 그냥 알아서 컴파일하고 테스트파일만 cmake로 
#include "DVM.hpp"
#include <windows.h>
#include <iostream>

using namespace std;

int main() {
    // 콘솔을 UTF-8로 고정
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    WSAData w;
    if(WSAStartup(MAKEWORD(2,2), &w) != 0) {
        cerr << "WinSock init failed\n";
        return -1;
    }
    // DVM1: 아이디 1, 좌표 (10,20), 포트번호 5001
    // DVM dvm1("T1", 10, 20, 5001);

    // DVM2: 아이디 2, 좌표 (4,5), 포트번호 5002
    DVM dvm2("T2", 4, 5, 5002);

    WSACleanup();
    
    return 0;
}
