#include "../utils.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace spatparse;
using Catch::Approx;

TEST_CASE("Azimuth conversions work correctly", "[azimuth]")
{
  SECTION("IEM AIIRAD conversions")
  {
    // Speaker #3 at azimuth 45° in SpatGRIS should be -45° in IEM AIIRAD
    REQUIRE(azimuth_to_iem_aiirad(45.0) == Approx(-45.0));
    REQUIRE(azimuth_from_iem_aiirad(-45.0) == Approx(45.0));

    // Speaker #5 at azimuth 225° in SpatGRIS should be 135° in IEM AIIRAD
    REQUIRE(azimuth_to_iem_aiirad(225.0) == Approx(135.0));
    REQUIRE(azimuth_from_iem_aiirad(135.0) == Approx(225.0));

    // Test edge cases
    REQUIRE(azimuth_to_iem_aiirad(0.0) == Approx(0.0));
    REQUIRE(azimuth_to_iem_aiirad(180.0) == Approx(-180.0));
    REQUIRE(azimuth_to_iem_aiirad(360.0) == Approx(0.0)); // Should wrap to 0

    // Test round-trip conversions
    for(double angle = 0.0; angle < 360.0; angle += 15.0)
    {
      double iem = azimuth_to_iem_aiirad(angle);
      double back = azimuth_from_iem_aiirad(iem);
      REQUIRE(back == Approx(angle).margin(0.001));
    }
  }

  SECTION("Spat5 conversions")
  {
    // Speaker #3 at azimuth 45° in SpatGRIS should be 45° in Spat5
    REQUIRE(azimuth_to_spat5(45.0) == Approx(45.0));
    REQUIRE(azimuth_from_spat5(45.0) == Approx(45.0));

    // Speaker #5 at azimuth 225° in SpatGRIS should be -135° in Spat5
    REQUIRE(azimuth_to_spat5(225.0) == Approx(-135.0));
    REQUIRE(azimuth_from_spat5(-135.0) == Approx(225.0));

    // Test edge cases
    REQUIRE(azimuth_to_spat5(0.0) == Approx(0.0));
    REQUIRE(azimuth_to_spat5(180.0) == Approx(180.0));
    REQUIRE(azimuth_to_spat5(360.0) == Approx(0.0)); // Should wrap to 0

    // Test round-trip conversions
    for(double angle = 0.0; angle < 360.0; angle += 15.0)
    {
      double spat = azimuth_to_spat5(angle);
      double back = azimuth_from_spat5(spat);
      REQUIRE(back == Approx(angle).margin(0.001));
    }
  }
}