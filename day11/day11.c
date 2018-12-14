#include <stdio.h>

#define WIDTH 300
#define HEIGHT 300
static int celldata[HEIGHT][WIDTH];

int power_level(int x, int y, int serial){
    int rackId = x + 10;
    int power = rackId * y;
    power += serial;
    power *= rackId;
    power = (power / 100) % 10;
    power -= 5;
    return power;
}

void build_cell_grid(int serial){
    //Lets make the cells array be 1-based
    int (*cells)[WIDTH] = celldata - 1;
    for(int y = 1; y <= HEIGHT; ++y){
        for(int x = 1; x <= WIDTH; ++x){
            cells[y][x] = power_level(x, y, serial);
            if(x > 1)
                cells[y][x] += cells[y][x - 1];
            if(y > 1)
                cells[y][x] += cells[y - 1][x];
            if(x > 1 && y > 1)
                cells[y][x] -= cells[y - 1][x -1];
        }
    }

}

int highest_power(int squareSize, int *tx, int *ty){
    int (*cells)[WIDTH] = celldata - 1;
    int s = squareSize;
    int highestPower = 0;
    for(int y = 1; y <= HEIGHT; ++y){
        for(int x = 1; x <= WIDTH; ++x){
            if(x >= s && y >= s){
                int totalPower = 0;
                if(x == s && y == s){
                    totalPower = cells[y][x];
                } else if(y == s){
                    totalPower = cells[y][x] - cells[y][x - s];
                } else if(x == s){
                    totalPower = cells[y][x] - cells[y - s][x];
                } else {
                    totalPower = cells[y][x] - cells[y][x - s] - cells[y - s][x] + cells[y - s][x - s];
                }

                if(totalPower > highestPower){
                    highestPower = totalPower;
                    *tx = x - (s - 1);
                    *ty = y - (s - 1);
                }    
            }
        }
    }
    return highestPower;
}

int main(void){

    build_cell_grid(1788);
    int tx, ty;
    int highestPower = highest_power(3, &tx, &ty);
    printf("Total Power %i starting at (%i,%i)\n", highestPower, tx, ty);

    int bestTx = 0;
    int bestTy = 0;
    int bestPower = 0;
    int bestSize = 0;
    for(int i = 2; i <= 300; ++i){
        highestPower = highest_power(i, &tx, &ty);
        if(highestPower > bestPower){
            bestPower = highestPower;
            bestTx = tx;
            bestTy = ty;
            bestSize = i;
        }
    }

    printf("Best Power = %i starting at (%i,%i), square size = %i\n", bestPower, bestTx, bestTy, bestSize);

}