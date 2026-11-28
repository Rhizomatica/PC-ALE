/**
 * \file ale_word_decoder.cpp
 */

#include "Word/ale_word_decoder.h"
#include "Word/address_encoder.h"

namespace ale {

WordEvent ALEWordDecoder::decode(const ALEWord& word,
                                 const WordDecodeContext& ctx) const
{
    WordEvent ev;
    const std::string addr = trim_ale_address(word.address);

    // Jedes gültige Wort während LBT = Kanal belegt.
    if (ctx.lbt_active) {
        ev.type = WordEvent::Type::CHANNEL_BUSY;
        return ev;
    }

    // DATA/REP nach TIS → mehrteilige Adresse des Gegenübers.
    if (ctx.collecting_conclusion
        && (word.type == PreambleType::DATA || word.type == PreambleType::REP)) {
        ev.type    = WordEvent::Type::DATA_EXTENSION;
        ev.address = addr;
        return ev;
    }

    // TO an uns → Ruf an eigene Adresse erkannt.
    // Per A.5.2.5.1 trägt das Scanning-TO-Word nur die ersten ≤3 Zeichen der
    // Zieladresse; self_address kann länger sein → Präfix-Vergleich nötig.
    // TWAS carries the SENDER's address (A.5.2.4.4, "this was"), never ours:
    // matching it against self_address made a peer whose address shares our
    // first three characters (B0AA / B0AH) look like a call to us, so its
    // termination or rejection was missed.
    if (word.type == PreambleType::TO
        && !addr.empty()
        && ctx.self_address.size() >= addr.size()
        && ctx.self_address.compare(0, addr.size(), addr) == 0) {
        ev.type    = WordEvent::Type::TO_SELF;
        ev.address = addr;
        return ev;
    }

    // TIS → Conclusion-Beginn; bei gesetztem expected_caller Adresse prüfen.
    if (word.type == PreambleType::TIS) {
        if (ctx.expected_caller.empty() || addr == ctx.expected_caller) {
            ev.type    = WordEvent::Type::TIS_CALLER;
            ev.address = addr;
        }
        return ev;
    }

    // TWAS nicht an uns → Rufablehnung.
    if (word.type == PreambleType::TWAS) {
        ev.type    = WordEvent::Type::TWAS_REJECTION;
        ev.address = addr;
        return ev;
    }

    return ev; // NONE
}

} // namespace ale
