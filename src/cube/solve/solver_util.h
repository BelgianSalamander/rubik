//
// Created by Anatol on 14/02/2023.
//

#ifndef RUBIK_SOLVER_UTIL_H
#define RUBIK_SOLVER_UTIL_H

#include <chrono>
#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <fstream>

struct TempFileProvider {
    TempFileProvider();
    TempFileProvider(const TempFileProvider&) = delete;
    ~TempFileProvider();

    std::vector<std::string> paths;

    std::string getNewTempFile();
};

template<typename Clock>
static void progressBar(int progress, int total, std::chrono::time_point<Clock> start) {
    int percent = (int) ((float) progress / total * 100);
    std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
    auto seconds = (long long) std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    auto remaining = (long long) (elapsed.count() / progress * (total - progress));

    int barWidth = 50;
    std::cout << "[";
    int pos = barWidth * percent / 100;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }

    std::cout << "] " << percent << "% -- " << progress << "/" << total << " -- " << seconds << "s elapsed -- " << remaining << "s remaining" << std::endl;

    std::cout.flush();
}

template<typename State>
static void performBFSDisk(
        State baseState,
        uint64_t numElements,
        std::function<uint64_t (const State&)> stateToIndex,
        std::function<std::vector<State> (const State&)> getNextStates,
        std::function<void (uint8_t depth, uint64_t idx, const State&)> callback
    ) {

    auto start = std::chrono::high_resolution_clock::now();
    TempFileProvider tempFileProvider;

    std::vector<bool> queued(numElements, false);

    int depth = 0;

    std::string frontierPath = tempFileProvider.getNewTempFile();
    std::ofstream frontierOut(frontierPath, std::ios::binary);
    frontierOut.write((char*) &baseState, sizeof(State));
    frontierOut.close();
    queued[stateToIndex(baseState)] = true;
    int frontierSize = 1;

    const int BUFFER_SIZE = 10000000;
    char* outputBuffer = new char[BUFFER_SIZE * sizeof(State)];
    char* inputBuffer = new char[BUFFER_SIZE * sizeof(State)];

    int processed = 0;

    while (frontierSize) {
        std::string nextFrontierPath = tempFileProvider.getNewTempFile();
        std::ofstream nextFrontierOut(nextFrontierPath, std::ios::binary);
        int nextFrontierSize = 0;

        std::ifstream frontierIn(frontierPath, std::ios::binary);

        int inputBufferPos = 0;
        int inputBufferCount = 0;
        int outputBufferPos = 0;

        auto currDepthStart = std::chrono::high_resolution_clock::now();
        std::cout << "Processing depth " << depth << " with " << frontierSize << " elements" << std::endl;
        for (int i = 0; i < frontierSize; i++) {
            if (inputBufferPos >= inputBufferCount) {
                frontierIn.read(inputBuffer, BUFFER_SIZE * sizeof(State));
                inputBufferCount = frontierIn.gcount() / sizeof(State);
                inputBufferPos = 0;
            }

            State state = baseState;
            memcpy(&state, inputBuffer + inputBufferPos * sizeof(State), sizeof(State));

            inputBufferPos++;

            uint64_t index = stateToIndex(state);
            callback(depth, index, state);

            processed++;

            if (processed % 1000000 == 0) {
                std::cout << "Global progress:" << std::endl;
                progressBar(processed, numElements, start);
                std::cout << "Depth progress (depth " << depth << "):" << std::endl;
                progressBar(i, frontierSize, currDepthStart);
            }

            for (State next: getNextStates(state)) {
                uint64_t nextIdx = stateToIndex(next);
                if (!queued[nextIdx]) {
                    queued[nextIdx] = true;

                    if (outputBufferPos >= BUFFER_SIZE) {
                        nextFrontierOut.write(outputBuffer, BUFFER_SIZE * sizeof(State));
                        outputBufferPos = 0;
                    }

                    memcpy(outputBuffer + outputBufferPos * sizeof(State), &next, sizeof(State));
                    outputBufferPos++;
                    nextFrontierSize++;
                }
            }
        }

        nextFrontierOut.write(outputBuffer, outputBufferPos * sizeof(State));
        nextFrontierOut.close();
        frontierIn.close();

        frontierPath = nextFrontierPath;
        frontierSize = nextFrontierSize;
        depth++;

        std::cout << "Finished processing depth " << depth - 1 << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - currDepthStart).count() << " ms" << std::endl;
    }

    delete[] outputBuffer;
    delete[] inputBuffer;

    std::cout << "Final depth: " << depth - 1 << std::endl;
    std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << " ms" << std::endl;
    std::cout << "Total processed: " << processed << std::endl;

    if (processed != numElements) {
        std::cout << "ERROR: processed != size" << std::endl;
    }
}

#endif //RUBIK_SOLVER_UTIL_H
