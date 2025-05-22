#include "wordle.h"
#include "player.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main() {
    char solution[WORD_LENGTH + 1];
    scanf("%s", solution);
    Player player = player_AI;
    wordle(solution, &player);
    return 0;
}