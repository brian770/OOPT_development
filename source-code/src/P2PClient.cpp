#include <P2PClient.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")  // WinSock 라이브러리 링크

P2PClient::P2PClient(){}
P2PClient::~P2PClient(){}

bool P2PClient::sendMessageToPeer(const std::string &ip, int port, const std::string &msg, std::string &response)
{
    response.clear();

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "[P2PClient] socket creation failed\n";
        return false;
    }
    
    // 소켓 타임아웃 설정(3초)
    DWORD timeout = 3000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());


    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[P2PClient] connection failed to " << ip << ":" << port << "\n";
        closesocket(sock);
        return false;
    }

    int sent = send(sock, msg.c_str(), static_cast<int>(msg.length()), 0);
    if (sent == SOCKET_ERROR) {
        std::cerr << "[P2PClient] send failed\n";
        closesocket(sock);
        return false;
    }

    // 응답 수신
    char buffer[4096];
    int bytes = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes > 0) {
        response.assign(buffer, bytes);
    } else {
        std::cerr << "[P2PClient] recv failed or empty\n";
        closesocket(sock);
        return false;
    }

    closesocket(sock);
    return true;
}