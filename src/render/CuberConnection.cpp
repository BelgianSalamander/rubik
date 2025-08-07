//
// Created by Anatol on 12/02/2023.
//

#include "CuberConnection.h"
#include <iostream>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

bool isWinsockInitialized = false;

void initWinsock() {
    if (isWinsockInitialized) return;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
        std::cout << "WSAStartup failed.\n";
        return;
    }
    isWinsockInitialized = true;

    std::cout << "Winsock initialized.\n";
}

CuberConnection::CuberConnection(std::string ip, int port) {
    initWinsock();
    this->reconfigure(ip, port);

}

void CuberConnection::reconfigure(std::string newIp, int newPort) {
    this->ip = newIp;
    this->port = newPort;

    listening = false;

    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cout << "Could not create socket: " << WSAGetLastError() << std::endl;
        return;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(this->port);
    inet_pton(AF_INET, this->ip.c_str(), &server.sin_addr.s_addr);

    if (bind(listener, (sockaddr *) &server, sizeof(server)) == SOCKET_ERROR) {
        std::cout << "Bind failed with error code: " << WSAGetLastError() << std::endl;
        return;
    }

    if (listen(listener, 1) == SOCKET_ERROR) {
        std::cout << "Listen failed with error code: " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "Listening for connections on " << this->ip << ":" << this->port << std::endl;

    listening = true;
}

void CuberConnection::tick() {
    if (!active) {
        acceptClient();
    }

    if (!active) return;

    while (true) {
        auto packet = readPacket();

        if (!packet.has_value()) break;

        int packetType = packet.value().data[0];

        if (packetType == 1) {
            std::string message = std::string((char*)packet.value().data + 1, packet.value().size - 1);
            std::cout << "Received message: " << message << std::endl;
        } else if (packetType == 5) {
            std::string inst = std::string((char*)packet.value().data + 1, packet.value().size - 1);
            std::cout << "Executing instruction " << inst << std::endl;
            movesExecuted.push(Move::fromString(inst));
        } else {
            std::cout << "Received unknown packet type: " << packetType << std::endl;
        }
    }
}

void CuberConnection::acceptClient() {
    if (!listening) return;
    //Use select to check if there is a client waiting to connect
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listener, &readfds);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int result = select(0, &readfds, NULL, NULL, &timeout);

    if (result == SOCKET_ERROR) {
        std::cout << "Select failed with error code: " << WSAGetLastError() << std::endl;
        return;
    }

    if (result == 0) return;

    if (FD_ISSET(listener, &readfds)) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);

        if ((client = accept(listener, (struct sockaddr *) &clientAddr, &clientAddrSize)) == INVALID_SOCKET) {
            std::cout << "Accept failed with error code: " << WSAGetLastError() << std::endl;
            return;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
        std::cout << "Client connected: " << ip << std::endl;

        active = true;
    }
}

bool CuberConnection::hasIncomingData() {
    if (!active) return false;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(client, &readfds);

    fd_set closedfds;
    FD_ZERO(&closedfds);
    FD_SET(client, &closedfds);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int result = select(0, &readfds, NULL, &closedfds, &timeout);

    if (result == SOCKET_ERROR) {
        std::cout << "Select failed with error code: " << WSAGetLastError() << std::endl;
        return false;
    }

    if (result == 0) {
        return false;
    }

    if (FD_ISSET(client, &closedfds)) {
        std::cout << "Client disconnected" << std::endl;
        active = false;
        return false;
    }

    if (FD_ISSET(client, &readfds)) {
        return true;
    }

    return false;
}

std::optional<CuberConnection::Packet> CuberConnection::readPacket() {
    if (sizeBufferPos != sizeof(sizeBuffer)) {
        int remaining = sizeof(sizeBuffer) - sizeBufferPos;

        if (!hasIncomingData()) return std::nullopt;

        int result = recv(client, ((char*) &sizeBuffer) + sizeBufferPos, remaining, 0);
        if (result == SOCKET_ERROR) {
            std::cout << "Recv failed with error code: " << WSAGetLastError() << std::endl;
            active = false;
            return std::nullopt;
        }

        sizeBufferPos += result;

        if (sizeBufferPos != sizeof(sizeBuffer)) {
            return std::nullopt;
        }

        if (sizeBuffer == 0) {
            std::cout << "Client disconnected" << std::endl;
            active = false;
            return std::nullopt;
        }

        if (sizeBuffer > 1000000) {
            std::cout << "Client sent invalid packet size" << std::endl;
            active = false;
            return std::nullopt;
        }

        dataBuffer = new char[sizeBuffer];
        dataBufferPos = 0;
    }

    int size = sizeBuffer;

    int remaining = size - dataBufferPos;
    if (!hasIncomingData()) return std::nullopt;

    int result = recv(client, (char*) dataBuffer + dataBufferPos, remaining, 0);
    if (result == SOCKET_ERROR) {
        std::cout << "Recv failed with error code: " << WSAGetLastError() << std::endl;
        active = false;
        return std::nullopt;
    }

    dataBufferPos += result;

    if (dataBufferPos != size) {
        return std::nullopt;
    }

    Packet packet;
    packet.data = (unsigned char*) dataBuffer;
    packet.size = size;

    sizeBufferPos = 0;
    dataBuffer = nullptr;

    return std::move(packet);
}

CuberConnection::~CuberConnection() {
    if (active) {
        closesocket(client);
    }
    closesocket(listener);

    if (dataBuffer != nullptr) {
        delete[] dataBuffer;
    }
}

bool CuberConnection::isActive() {
    return active;
}

void CuberConnection::resetRobot() {
    //Send packet containing only '\x01'

    int length = 1;
    unsigned char data = 1;

    sendall((char*) &length, sizeof(length));
    sendall((char*) &data, length);
}

void CuberConnection::sendall(const char* data, int size) {
    if (!active) return;
    int sent = 0;
    while (sent < size) {
        int result = send(client, data + sent, size - sent, 0);
        if (result == SOCKET_ERROR) {
            std::cout << "Send failed with error code: " << WSAGetLastError() << std::endl;
            active = false;
            return;
        }
        sent += result;
    }
}

void CuberConnection::doMoves(std::string moves) {
    int length = moves.length() + 1;
    unsigned char type = 0;

    std::cout << "Sending the following algorithm: \'" << moves << "\'" << std::endl;

    sendall((char*) &length, sizeof(length));
    sendall((char*) &type, 1);
    sendall(moves.c_str(), moves.length());
}
