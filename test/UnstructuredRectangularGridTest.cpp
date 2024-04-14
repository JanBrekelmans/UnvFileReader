#include <doctest/doctest.h>

#include <UnvFileReader/UnvFileReader.h>

#include <algorithm>

TEST_CASE("Unstructured Rectangular grid test") {
    const std::filesystem::path path("./test/files/Unstructured/RectangularGrid.unv");
    auto fileStructure = UnvFileReader::UnvFileReader::readUnvFile(path);

    // Verify the units
    auto& unvUnits = fileStructure.unvUnits;
    CHECK("SI: Meter (newton)" == unvUnits.lengthUnit);
    CHECK(1.0 == doctest::Approx(unvUnits.lengthConversionFactor));
    CHECK(1.0 == doctest::Approx(unvUnits.forceConversionFactor));
    CHECK(1.0 == doctest::Approx(unvUnits.temperatureConversionFactor));
    CHECK(273.15 == doctest::Approx(unvUnits.temperatureOffset));

    // Verify the nodes
    auto& nodes = fileStructure.nodes;
    CHECK(109 == nodes.size());

    // Verify the cells
    auto& cells = fileStructure.cells;
    CHECK(176 + 4 * 10 == cells.size());  // 176 interior cells, and 4 times 10 boundary cells

    // Number of interior and boundary cells
    auto numInteriorCells = std::count_if(cells.begin(), cells.end(), [](auto p) { return p.second.nodes.size() == 3; });
    auto numBoundaryCells = std::count_if(cells.begin(), cells.end(), [](auto p) { return p.second.nodes.size() == 2; });
    CHECK(176 == numInteriorCells);
    CHECK(4 * 10 == numBoundaryCells);

    // Verify the groups
    auto groups = fileStructure.groups;

    // Verify the interior cells
    CHECK(176 == groups.at("Interior").size());
    CHECK(10 == groups.at("TopFace").size());
    CHECK(10 == groups.at("BottomFace").size());
    CHECK(10 == groups.at("RightFace").size());
    CHECK(10 == groups.at("LeftFace").size());

    std::vector<std::string> groupNames{"Interior", "TopFace", "BottomFace", "RightFace", "LeftFace"};

    for (const auto& groupName : groupNames) {
        const auto& group = groups.at(groupName);
        std::for_each(group.begin(), group.end(), [&](auto cellIndex) {
            auto& cell = cells[cellIndex];
            CHECK(groupName == cell.groupName);
        });
    }
}
