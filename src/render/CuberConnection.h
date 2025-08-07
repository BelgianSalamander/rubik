//
// Created by Anatol on 12/02/2023.
//

#ifndef RUBIK_CUBERCONNECTION_H
#define RUBIK_CUBERCONNECTION_H

#include <string>
#include <queue>
#include "common.h"

#include <cstdio>
#include <winsock2.h>
#include <optional>

class CuberConnection {
public:
    CuberConnection(std::string ip, int port);
    ~CuberConnection();

    void reconfigure(std::string ip, int port);

    void tick();

    bool isActive();

    void resetRobot();
    void doMoves(std::string moves);

    const std::string& getIp() const {
        return ip;
    }

    int getPort() const {
        return port;
    }

    bool isListening() const {
        return listening;
    }

    std::queue<Move> movesExecuted;
private:
    struct Packet {

        int size;
        unsigned char* data;

        ~Packet() {
            delete[] data;
        }

        Packet() {};
        Packet(const Packet& other) {
            size = other.size;
            data = new unsigned char[size];
            memcpy(data, other.data, size);
        }
    };

    std::string ip;
    int port;

    SOCKET listener;

    bool listening = false;
    bool active = false;
    SOCKET client;

    int sizeBuffer;
    int sizeBufferPos = 0;

    void* dataBuffer = nullptr;
    int dataBufferPos = 0;

    void acceptClient();

    bool hasIncomingData();
    std::optional<CuberConnection::Packet> readPacket();

    void sendall(const char* data, int size);
};


#endif //RUBIK_CUBERCONNECTION_H
