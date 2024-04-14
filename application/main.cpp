#include <string>
#include <iostream>

#include <UnvFileReader/UnvFileReader.h>

int main() {
    const std::filesystem::path path("./test/files/Unstructured/RectangularGrid.unv");

    try {
        auto unvFileStructure = UnvFileReader::UnvFileReader::readUnvFile(path);

        auto& nodes = unvFileStructure.nodes;
        auto& groups = unvFileStructure.groups;

        std::cout << "Number of nodes: " << nodes.size() << "\n";
        for (auto it : groups) {
            std::cout << "Group " << it.first << " has " << it.second.size() << " cells\n";
        }
    } catch (std::runtime_error e) {
        std::cout << e.what();
    }
}
