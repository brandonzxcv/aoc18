#include "../include/br.h"
#include <string.h>
#include <stdint.h>

struct Marble {
    int64_t value;
    struct Marble *next;
    struct Marble *previous;
};
typedef struct Marble Marble;

static Marble *currentMarble;
static Marble marbles[100000 * 100];
static int marbleCount;

static int64_t playerScores[500];

void play_game(int players, int lastMarble){
    printf("players = %i, marbles = %i\n", players, lastMarble);

    currentMarble = marbles + marbleCount++;
    currentMarble->value = 0;
    currentMarble->next = currentMarble->previous = marbles + marbleCount++;
    currentMarble->next->value = 1;
    currentMarble->next->next = currentMarble->next->previous = currentMarble;

    for(int i = 2; i <= lastMarble; ++i){
        int player = i % players;
        if(player == 0)
            player = players;
        if(i % 23 == 0){
            Marble *removed = currentMarble->previous->previous->previous->previous->previous->previous->previous;
            playerScores[player] += i + removed->value;
            removed->previous->next = removed->next;
            removed->next->previous = removed->previous;
            currentMarble = removed->next;
            continue;
        }
        Marble *newMarble = marbles + marbleCount++;
        newMarble->value = i;
        newMarble->next = currentMarble->next->next;
        currentMarble->next->next = newMarble;
        newMarble->next->previous = newMarble;
        newMarble->previous = currentMarble->next;
        currentMarble = newMarble;
    }

    int64_t highscore = 0;
    for(int i = 0; i < 500; ++i){
        if(playerScores[i] > highscore){
            highscore = playerScores[i];
        }
    }

    printf("highscore = %I64i\n", highscore);
}

int main(void){
    FILE *input = file_open("input.txt", "r");
    char line[64];
    fgets(line, 64, input);
    fclose(input);

    int playerCount = atoi(line);
    char *c = strstr(line, "worth");
    c = strchr(c, ' ') + 1;
    int totalMarbles = atoi(c);

    //play_game(419, 72164);
    play_game(playerCount, totalMarbles * 100);
}