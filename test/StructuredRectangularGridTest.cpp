#include <doctest/doctest.h>

#include <UnvFileReader/UnvFileReader.h>

#include <algorithm>

TEST_CASE("Structured Rectangular grid test") {
    const std::filesystem::path path("./test/files/Structured/RectangularGrid.unv");
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
    CHECK(11 * 11 == nodes.size());

    // Verify the cells
    auto& cells = fileStructure.cells;
    CHECK(10 * 10 + 4 * 10 == cells.size());  // 100 interior cells, and 4 times 10 boundary cells

    // Number of interior and boundary cells
    auto numInteriorCells = std::ranges::count_if(cells.begin(), cells.end(), [](auto p) { return p.second.nodes.size() == 4; });
    auto numBoundaryCells = std::ranges::count_if(cells.begin(), cells.end(), [](auto p) { return p.second.nodes.size() == 2; });
    CHECK(10 * 10 == numInteriorCells);
    CHECK(4 * 10 == numBoundaryCells);

    // Verify the groups
    auto groups = fileStructure.groups;

    // Verify the interior cells
    CHECK(100 == groups.at("Interior").size());
    CHECK(10 == groups.at("TopFace").size());
    CHECK(10 == groups.at("BottomFace").size());
    CHECK(10 == groups.at("RightFace").size());
    CHECK(10 == groups.at("LeftFace").size());

    std::vector<std::string> groupNames{"Interior", "TopFace", "BottomFace", "RightFace", "LeftFace"};

    for (const auto& groupName : groupNames) {
        const auto& group = groups.at(groupName);
        std::ranges::for_each(group.begin(), group.end(), [&](auto cellIndex) {
            auto& cell = cells[cellIndex];
            CHECK(groupName == cell.groupName);
        });
    }
}