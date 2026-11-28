/**
 * \file test_word_decoder.cpp
 * \brief ALEWordDecoder: what a received word means to this station.
 *
 * TWAS carries the sender's address ("this was"): it is a termination or a
 * rejection whatever its address, even when the sender's address shares our
 * first three characters (B0AA / B0AH).  TO carries the called address, of
 * which a scanning call sends the first three characters: a prefix of ours
 * is a call to us.
 */

#include "Word/ale_word.h"
#include "Word/ale_word_decoder.h"
#include <cstdio>
#include <cstring>

using namespace ale;

static int failures = 0;

static ALEWord word(PreambleType type, const char *addr3)
{
    ALEWord w;
    w.type = type;
    std::memcpy(w.address, addr3, 3);
    w.address[3] = '\0';
    w.valid = true;
    return w;
}

static void expect(const char *what, WordEvent::Type got, WordEvent::Type want)
{
    if (got != want) {
        std::printf("FAIL: %s: got %d, want %d\n", what, (int)got, (int)want);
        failures++;
    } else {
        std::printf("PASS: %s\n", what);
    }
}

int main()
{
    ALEWordDecoder d;
    WordDecodeContext ctx;
    ctx.self_address = "B0AH";

    expect("TWAS from a station sharing our prefix is a termination",
           d.decode(word(PreambleType::TWAS, "B0A"), ctx).type, WordEvent::Type::TWAS_REJECTION);
    expect("TWAS from another station is a termination",
           d.decode(word(PreambleType::TWAS, "ICO"), ctx).type, WordEvent::Type::TWAS_REJECTION);
    expect("TO with our first three characters is a call to us",
           d.decode(word(PreambleType::TO, "B0A"), ctx).type, WordEvent::Type::TO_SELF);
    expect("TO another station is not",
           d.decode(word(PreambleType::TO, "ICO"), ctx).type, WordEvent::Type::NONE);
    return failures ? 1 : 0;
}
