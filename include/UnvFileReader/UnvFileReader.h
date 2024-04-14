#pragma once

#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

namespace UnvFileReader {

    struct UnvUnits {
        std::string lengthUnit;
        std::string temperatureMode;
        double lengthConversionFactor;
        double forceConversionFactor;
        double temperatureConversionFactor;
        double temperatureOffset;
    };

    struct UnvNode {
        int index;
        double x, y, z;
    };

    struct UnvCell {
        int label;
        int typeId;
        std::vector<int> nodes;
        std::string groupName;
    };

    struct UnvFileStructure {
        UnvUnits unvUnits;

        std::unordered_map<int, UnvNode> nodes;
        std::unordered_map<int, UnvCell> cells;

        std::unordered_map<std::string, std::vector<int>> groups;
    };

    class UnvFileReader final {
      public:
        static UnvFileStructure readUnvFile(const std::filesystem::path& path);

      private:
        UnvFileReader() = default;

        UnvFileStructure readUnvFileImpl(const std::filesystem::path& path);

        static bool isSeparator(const std::string_view& stringView);
        int readTag(std::ifstream& fileStream);
        void skipSection(std::ifstream& fileStream);

        void readUnvUnits(std::ifstream& fileStream, UnvUnits& unvUnits);
        void readUnvNodes(std::ifstream& fileStream, std::unordered_map<int, UnvNode>& unvNodes);
        void readUnvCells(std::ifstream& fileStream, std::unordered_map<int, UnvCell>& unvCells);
        void readUnvGroups(std::ifstream& fileStream, std::unordered_map<int, UnvCell>& unvCells, std::unordered_map<std::string, std::vector<int>>& groups);

        inline static const std::string SEPARATOR{"    -1"};
    };
}  // namespace UnvFileReader
