#pragma once

#include <string>
#include <functional>
#include <fstream>
#include <iostream>
#include <chrono>
#include <filesystem>
#include "cube/FastRubiksCube.h"

template<typename T>
struct BasicSerializer {
    static void serialize(T* ptr, uint64_t size, std::ostream& out) {
        out.write((char*) ptr, size);
    }

    static void deserialize(T* ptr, uint64_t size, std::istream& in) {
        in.read((char*) ptr, size);
    }
};

template<typename T, typename Serializer = BasicSerializer<T>>
class Database {
public:
    uint64_t size;
    std::string path;
    std::function<void (T*)> loader;
    T* ptr = nullptr;

    Database(uint64_t size, std::string path, std::function<void (T*)> loader) : size(size), path(std::move(path)), loader(std::move(loader)) {}

    ~Database() {
        if (ptr) {
            delete ptr;
        }
    }

    void ensureLoaded() {
        if (!ptr) {
            this->ptr = (T*) malloc(size);

            std::ifstream in(path, std::ios::binary);
            if (in) {
                Serializer::deserialize(this->ptr, size, in);
                in.close();
                std::cout << "Loaded " << path << std::endl;
                return;
            }

            std::cout << "Need to generate " << path << std::endl;
            auto start = std::chrono::high_resolution_clock::now();
            loader(this->ptr);
            auto end = std::chrono::high_resolution_clock::now();

            std::cout << "Finished generating!" << std::endl;
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Took " << elapsed << " milliseconds!" << std::endl;

            std::filesystem::path filePath = path;
            std::filesystem::create_directories(filePath.parent_path());
            std::ofstream out(path, std::ios::binary);
            if (!out) {
                std::cerr << "Couldn't open " << path << "\n";
                exit(1);
            }
            Serializer::serialize(this->ptr, size, out);
            out.close();

            std::cout << "Saved " << path << std::endl;
        }
    }
};
