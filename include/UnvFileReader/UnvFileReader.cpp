#include "UnvFileReader.h"

#include <algorithm>

namespace UnvFileReader {
    namespace internal {
        void leftTrim(std::string& s) {
            auto firstNonSpace = std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); });
            s.erase(s.begin(), firstNonSpace);
        }

        void rightTrim(std::string& s) {
            auto lastNonSpace = std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base();
            s.erase(lastNonSpace, s.end());
        }

        std::string trim(const std::string& s) {
            std::string copy = s;
            leftTrim(copy);
            rightTrim(copy);
            return copy;
        }
    }  // namespace internal

    UnvFileStructure UnvFileReader::readUnvFile(const std::filesystem::path& path) { return UnvFileReader{}.readUnvFileImpl(path); }

    UnvFileStructure UnvFileReader::readUnvFileImpl(const std::filesystem::path& path) {
        std::ifstream fileStream;
        fileStream.open(path, std::ios_base::in);

        if (!fileStream.good()) {
            throw std::runtime_error("Could not open file " + path.string());
        }

        UnvFileStructure unvFileStructure{};

        while (fileStream.good()) {
            int tag = readTag(fileStream);

            if (tag == -1) {
                break;
            }

            switch (tag) {
                case 164:
                    readUnvUnits(fileStream, unvFileStructure.unvUnits);
                    break;
                case 2411:
                    readUnvNodes(fileStream, unvFileStructure.nodes);
                    break;
                case 2412:
                    readUnvCells(fileStream, unvFileStructure.cells);
                    break;
                case 2467:
                    readUnvGroups(fileStream, unvFileStructure.cells, unvFileStructure.groups);
                    break;
                case 2420:
                    skipSection(fileStream);
                    break;
            }
        }

        return unvFileStructure;
    }

    bool UnvFileReader::isSeparator(const std::string_view& stringView) { return stringView == SEPARATOR; }

    int UnvFileReader::readTag(std::ifstream& fileStream) {
        std::string tag;
        do {
            if (!fileStream.good()) {
                return -1;
            }

            std::getline(fileStream, tag);
            if (tag.size() < 6) {
                return -1;
            }

            tag = tag.substr(0, 6);

        } while (isSeparator(tag));

        return std::stoi(tag);
    }

    void UnvFileReader::skipSection(std::ifstream& fileStream) {
        std::string line;

        while (fileStream.good()) {
            std::getline(fileStream, line);
            if (isSeparator(line)) break;
        }
    }

    void UnvFileReader::readUnvUnits(std::ifstream& fileStream, UnvUnits& unvUnits) {
        std::string line;
        std::getline(fileStream, line);
        unvUnits.lengthUnit = internal::trim(line.substr(10, 20));

        std::getline(fileStream, line);
        std::string lengthConversionFactorString = line.substr(0, 25);
        std::string forceConversionFactorString = line.substr(25, 25);
        std::string temperatureConversionFactorString = line.substr(50, 25);

        std::getline(fileStream, line);
        std::string temperatureOffsetString = line.substr(0, 25);

        unvUnits.lengthConversionFactor = std::stod(lengthConversionFactorString);
        unvUnits.forceConversionFactor = std::stod(forceConversionFactorString);
        unvUnits.temperatureConversionFactor = std::stod(temperatureConversionFactorString);
        unvUnits.temperatureOffset = std::stod(temperatureOffsetString);
    }

    void UnvFileReader::readUnvNodes(std::ifstream& fileStream, std::unordered_map<int, UnvNode>& unvNodes) {
        std::string line;
        while (true) {
            std::getline(fileStream, line);
            if (isSeparator(line)) return;

            int pointIndex = std::stoi(line.substr(0, 10));

            UnvNode node{};
            std::getline(fileStream, line);
            node.index = pointIndex;
            node.x = std::stod(line.substr(0, 25));
            node.y = std::stod(line.substr(25, 25));
            node.z = std::stod(line.substr(50, 25));
            unvNodes[pointIndex] = std::move(node);
        }
    }

    void UnvFileReader::readUnvCells(std::ifstream& fileStream, std::unordered_map<int, UnvCell>& unvCells) {
        std::string line;

        auto nodesDefiningElement = [](std::string& s, int numNodes) {
            std::vector<int> nodeIndices;
            nodeIndices.reserve(numNodes);
            for (int i = 0; i < numNodes; i++) {
                nodeIndices.push_back(std::stoi(s.substr(10 * i, 10)));
            }
            return nodeIndices;
        };

        while (true) {
            std::getline(fileStream, line);
            if (isSeparator(line)) return;

            int cellIndex, feDescriptorId, numNodes;
            cellIndex = std::stoi(line.substr(0, 10));
            feDescriptorId = std::stoi(line.substr(10, 10));
            numNodes = std::stoi(line.substr(50, 10));

            UnvCell cell{};

            switch (feDescriptorId) {
                case 11: {  // Ignore first part of beam element
                    std::getline(fileStream, line);
                }
            }

            std::getline(fileStream, line);
            cell.label = cellIndex;
            cell.typeId = feDescriptorId;
            cell.nodes = nodesDefiningElement(line, numNodes);

            unvCells[cellIndex] = std::move(cell);
        }
    }

    void UnvFileReader::readUnvGroups(std::ifstream& fileStream, std::unordered_map<int, UnvCell>& unvCells, std::unordered_map<std::string, std::vector<int>>& groups) {
        std::string line;
        while (true) {
            std::getline(fileStream, line);
            if (isSeparator(line)) return;

            int groupNumber, numberOfCells;
            groupNumber = std::stoi(line.substr(0, 10));
            numberOfCells = std::stoi(line.substr(70, 10));

            std::getline(fileStream, line);
            std::string groupName = internal::trim(line);

            std::vector<int> cellIndices;
            cellIndices.reserve(numberOfCells);
            int numOfReadInCells = 0;
            while (numOfReadInCells < numberOfCells) {
                std::getline(fileStream, line);
                int numToReadFromLine = (numOfReadInCells == numberOfCells - 1) ? 1 : 2;

                for (int i = 0; i < numToReadFromLine; i++) {
                    int cellIndex = std::stoi(line.substr(10 + 40 * i, 10));
                    cellIndices.push_back(cellIndex);
                    unvCells.at(cellIndex).groupName = groupName;
                }

                numOfReadInCells += numToReadFromLine;
            }

            groups[groupName] = cellIndices;
        }
    }
}  // namespace UnvFileReader
