//
// Created by Anatol on 14/02/2023.
//

#include "solver_util.h"

#include <filesystem>

TempFileProvider::TempFileProvider()
    :paths()
{
    std::filesystem::create_directory("tmp");
}

TempFileProvider::~TempFileProvider() {
    for (const std::string& path : paths) {
        std::filesystem::remove(path);
    }
}

std::string TempFileProvider::getNewTempFile() {
    std::string path = "tmp/temp_" + std::to_string(paths.size()) + ".bin";
    paths.push_back(path);
    return path;
}
