#include "Attacks.hpp"
#include "SlidingAttacks.hpp"
#include "Uci.hpp"

int main() {
    // 1. Inicjalizacja tablic przed zrobieniem czegokolwiek!
    MoveGen::init_sliders(); // z Twojego SlidingAttacks.hpp
    Attacks::init();         // z nowego modułu Attacks

    // 2. Oddajemy kontrolę protokołowi UCI
    UCI::loop();

    return 0;
}