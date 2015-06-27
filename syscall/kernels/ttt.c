#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ascio.h"

int getVal() {
  char c=asc_getc();
  if (c == '\n'){
     printf("Yo");
     return -1;
  }
  return atoi(&c);
}

char gridChar(int i) {
    switch(i) {
        case -1:
            return 'X';
        case 0:
            return ' ';
        case 1:
            return 'O';
    }
    return 'c';
}

void draw(int b[9]) {

    printf(" %c | %c | %c\n",gridChar(b[0]),gridChar(b[1]),gridChar(b[2]));
    printf("---+---+---\n");
    printf(" %c | %c | %c\n",gridChar(b[3]),gridChar(b[4]),gridChar(b[5]));
    printf("---+---+---\n");
    printf(" %c | %c | %c\n",gridChar(b[6]),gridChar(b[7]),gridChar(b[8]));
}

int win(const int board[9]) {
    //determines if a player has won, returns 0 otherwise.
    unsigned wins[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
    int i;
    for(i = 0; i < 8; ++i) {
        if(board[wins[i][0]] != 0 &&
           board[wins[i][0]] == board[wins[i][1]] &&
           board[wins[i][0]] == board[wins[i][2]])
            return board[wins[i][2]];
    }
    return 0;
}

int minimax(int board[9], int player) {
    //How is the position like for player (their turn) on board?
    int winner = win(board);
    if(winner != 0) return winner*player;
    int move = -1;
    int score = -2;//Losing moves are preferred to no move
    int i;
    for(i = 0; i < 9; ++i) {//For all moves,
        if(board[i] == 0) {//If legal,
            board[i] = player;//Try the move
            int thisScore = -minimax(board, player*-1);
            if(thisScore > score) {
                score = thisScore;
                move = i;
            }//Pick the one that's worst for the opponent
            board[i] = 0;//Reset board after try
        }
    }
    if(move == -1) return 0;
    return score;
}

void computerMove(int board[9]) {
    int move = -1;
    int score = -2;
    int i;
    for(i = 0; i < 9; ++i) {
        if(board[i] == 0) {
            board[i] = 1;
            int tempScore = -minimax(board, -1);
            board[i] = 0;
            if(tempScore > score) {
                score = tempScore;
                move = i;
            }
        }
    }
    //returns a score based on minimax tree at a given node.
    board[move] = 1;
}

void playerMove(int board[9]) {
    int move = 0;
    do {
        printf("\nInput move ([0..8]): ");
	//printf("Yo we're in playerMove function");
        move=getVal();
	//printf("Hey so we've hypothetically got the move");
	//printf("Here is board[move] %d\n", board[move]);
	printf("\n");
	board[move] = 0;
    } while ((move >= 9 || move < 0) || (board[move] == 1));
    board[move] = -1;
}

//Potential function for random player
//Chooses moves randomly
/*
void randomMove(int board[9]) {
     int i = random() % 3;
     bool flag = false;
     while (flag == false){
        if (board[i] != 'X' && board[i] != 'O'){
           board[i] = 'O';
           flag = true;
        }
    }
}
*/
/*
int[] helper(int a [9]){
   for (int i =0; i<9; i++){
       a[i] = 0;
  }
  return a;
}
*/
/*
#define ZERO_ANY(T, a, n) do{\
   T *a_ = (a);\
   size_t n_ = (n);\
   for (; n_ > 0; --n_, ++a_)\
     *a_ = (T) { 0 };\
} while (0)
*/

int main() {
    int i;
    unsigned turn;
    int playerCount = 0;
    int board[9] = {0,0,0,0,0,0,0,0,0};
    turn = 0;
    //computer squares are 1, player squares are -1.
    for (i=0; i<100; ++i){
    printf("Computer O, You: X\nPlay (1)st or (2)nd? ");
    int player=getVal();
    if (player < 0){
       printf("OH NOOOOOESSS\n");
    }
    printf("\n");
    for(turn = 0; turn < 9 && win(board) == 0; ++turn) {
        if((turn+player) % 2 == 0){
            computerMove(board);
        } else {
            draw(board);
	    playerCount++;
            playerMove(board);
        }
    }
    printf("the numnber of times we entered playerMove is %d\n", playerCount);
    board[0] = 0;
    board[1] = 0;
    board[2] = 0;
    board[3] = 0;
    board[4] = 0;
    board[5] = 0;
    board[6] = 0;
    board[7] = 0;
    board[8] = 0;

    switch(win(board)) {
	printf("Player Counter is %d\n", playerCount);
        case 0:
            break;
        case 1:
            draw(board);
            printf("You lose.\n");
            break;
        case -1:
            printf("You win. Inconceivable!\n");
	    break;
    }
}
    return 1;
}
