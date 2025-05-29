#include <P2PServer.hpp>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

P2PServer::P2PServer(int port, RequestHandler handler)
    : port(port), handleRequest(handler), listenSocket(INVALID_SOCKET), running(false) {}

P2PServer::~P2PServer() {
    stop();
}

void P2PServer::start() {
    if (running) return;

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "[P2PServer] socket creation failed\n";
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[P2PServer] bind failed\n";
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        return;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[P2PServer] listen failed\n";
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        return;
    }

    running = true;
    serverThread = std::thread(&P2PServer::listenLoop, this);
    std::cout << "[P2PServer] Listening on port " << port << "...\n";
}

void P2PServer::stop() {
    if (!running) return;

    running = false;

    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }

    if (serverThread.joinable()) {
        serverThread.join();
    }

    std::cout << "[P2PServer] Stopped.\n";
}

void P2PServer::listenLoop() {
    while (running) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            if (running) std::cerr << "[P2PServer] accept failed\n";
            continue;
        }

        std::thread(&P2PServer::handleClient, this, clientSocket).detach();
    }
}

void P2PServer::handleClient(SOCKET clientSocket) {
    char buffer[4096] = {0};
    int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytes > 0) {
        std::string request(buffer, bytes);
        std::string response = handleRequest(request);

        send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
    } else {
        std::cerr << "[P2PServer] recv failed or client disconnected\n";
    }

    closesocket(clientSocket);
}