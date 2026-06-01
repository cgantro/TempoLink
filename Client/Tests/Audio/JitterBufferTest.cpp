#include <catch2/catch_test_macros.hpp>
#include "Audio/JitterBuffer.h"

TEST_CASE("JitterBuffer pop returns packets in order", "[jitter]")
{
    JitterBuffer buf(4);
    std::vector<uint8_t> pkt = {1, 2, 3};

    buf.push(0, pkt);
    buf.push(1, pkt);

    std::vector<uint8_t> out;
    REQUIRE(buf.pop(out));
    REQUIRE(buf.pop(out));
    REQUIRE_FALSE(buf.pop(out));
}
