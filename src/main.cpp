#include "defines.h"

#include "game_entry.hpp"

int main(void) {
    u16 width = 1920, height = 1080;
    char name[] = "hello world!";
    GameConfig config = {width, height, name};

    if (!game_init(&config)) {
        exit(EXIT_FAILURE);
    }

    game_run();

    exit(EXIT_SUCCESS);
}