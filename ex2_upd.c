

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#define BOARD_SIZE 4

void Initialize(pid_t pid);

void SetRandomCell();

void PrintBoardLine();

void SendUSR1();

void TimerHandler(int sigNum);

void ShiftLeft(int *gameBoard);

void ShiftRight(int *gameBoard);

void ShiftUp(int *gameBoard);

void ShiftDown(int *gameBoard);

void RandomX();

int OpenFileToWrite();

void HandleUserCommand(char command);

void DecideGameStatus();

void PrintGameMessage(int choice);

int CheckBoard();

int   board[BOARD_SIZE][BOARD_SIZE];
pid_t pid;
int   x;


int main(int argc, char *argv[]) {

    //Check number of parameters.
    if (argc != 2) {

        perror("Error: wrong number of parameters.\n");
        exit(1);
    }

    //Variable declarations.
    pid = atoi(argv[1]);

    //Open file to read data from.
    OpenFileToWrite();

    //Initialize the game.
    Initialize(pid);

    struct sigaction usr_action;
    sigset_t         block_mask;
    sigfillset(&block_mask);
    usr_action.sa_handler = TimerHandler;
    usr_action.sa_mask    = block_mask;
    usr_action.sa_flags   = 0;
    //TODO make validation checks
    sigaction(SIGALRM, &usr_action, NULL);

    //x=2;
    alarm(x);

    char command;

    while (1) {

        system("stty cbreak -echo");
        command = getchar();
        system("stty cooked echo");

        if (command != -1) {
            //Disable alarm
            alarm(0);

            //Handle user command.
            HandleUserCommand(command);
        }

    }
}

void Initialize(pid_t pid) {

    int randCellX1 = 0;
    int randCellY1 = 0;
    int randCellX2 = 0;
    int randCellY2 = 0;

    //Initialize board with zeros.
    for (int i = 0; i < BOARD_SIZE; ++i) {

        for (int j = 0; j < BOARD_SIZE; ++j) {

            board[i][j] = 0;
        }
    }

    srand(time(NULL));

    //Generate random temp value.
    x = (rand() % 5) + 1;

    //Generate random cell positions.
    randCellX1 = rand() % BOARD_SIZE;
    randCellY1 = rand() % BOARD_SIZE;
    randCellX2 = rand() % BOARD_SIZE;

    do {

        randCellY2 = rand() % BOARD_SIZE;
    } while (randCellY2 == randCellY1);


    //Set random cells.
    board[randCellX1][randCellY1] = 2;
    board[randCellX2][randCellY2] = 2;

    //Print board in line format.
    PrintBoardLine();

    //Send signal to process.
    SendUSR1();

}

int OpenFileToWrite() {
    //Open file to read data from.
    int file = open("data.txt", O_WRONLY);

    //Check if file was opened.
    if (file < 0) {
        perror("Error: failed to open file.\n");
        exit(1);
    }

    //Redirect to STDOUT.
    dup2(file, 1);

    return file;
}

void TimerHandler(int sigNum) {

    alarm(0);

    //Set 2 at a random empty cell.
    SetRandomCell();

    //Set new random x value.
    RandomX();

    //Print to file.
    PrintBoardLine();

    //Send signal to process inp.
    SendUSR1();

//  sigaction(SIGALRM, &usr_action, NULL);

    //Set new timer.
    alarm(x);
}

void HandleUserCommand(char command) {

    switch (command) {

        case 'A':
            //Move left.
            ShiftLeft(board);
            DecideGameStatus();
            break;

        case 'D':
            //Move right.
            ShiftRight(board);
            DecideGameStatus();
            break;

        case 'W':
            //Move up.
            ShiftUp(board);
            DecideGameStatus();
            break;

        case 'X':
            //Move down.
            ShiftDown(board);
            DecideGameStatus();
            break;

        case 'S':
            Initialize(pid);
            alarm(x);
            break;

        default:
            break;

    }
}

void SetRandomCell() {

    int isDone = 0;
    int randCellX;
    int randCellY;

    srand(time(NULL));

    while (!isDone) {

        randCellX = rand() % 4;
        randCellY = rand() % 4;

        if (board[randCellX][randCellY] == 0) {

            board[randCellX][randCellY] = 2;
            isDone = 1;
        }
    }

}

void SendUSR1() {

    int killResult;

    killResult = kill(pid, SIGUSR1);

    //Check if signal was sent.
    if (killResult < 0) {

        perror("Error: signal failed.\n");
        exit(1);
    }
}

void PrintBoardLine() {

    int writeResult;

    for (int i = 0; i < BOARD_SIZE; ++i) {

        for (int j = 0; j < BOARD_SIZE; ++j) {

            char *number[6];
            memset(number, 0, 6);

            //Convert number to string.
            if (i == BOARD_SIZE - 1 && j == BOARD_SIZE - 1) {

                sprintf(number, "%d\n", board[i][j]);
            } else {

                sprintf(number, "%d,", board[i][j]);
            }

            writeResult = write(1, number, strlen(number));

            //Check if wrote.
            if (writeResult < 0) {

                perror("Error: write failed.\n");
                exit(1);
            }
        }
    }
}

void DecideGameStatus() {

    int boardStatus = CheckBoard();

    if (boardStatus == 0) {

        //Prepare for next move.
        RandomX();
        PrintBoardLine();
        SendUSR1();
        alarm(x);
    } else if (boardStatus == 1) {

        //Print game won.
        PrintGameMessage(1);
        //TODO handle exit as requested.
        exit(0);
    } else {

        //Print game lost.
        PrintBoardLine(2);
        //TODO handle exit as requested.
        exit(0);
    }
}

void PrintGameMessage(int choice) {

    int  writeResult;
    char message[1];

    if (choice == 2) {

        //Set message to game lost.
        message[0] = 'l';
    } else {

        //Set message to game won.
        message[0] = 'w';
    }

    //Write message.
    writeResult = write(1, message, 1);

    perror("Wrote message.\n");

    //Check if wrote.
    if (writeResult < 0) {

        perror("Error: write failed.\n");
        exit(1);
    }

}

int CheckBoard() {

    int foundZero = 0;

    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {

            if (board[i][j] == 2048) {

                //Game won.
                return 1;
            }

            if(board[i][j] == 0){

                foundZero = 1;
            }
        }
    }

    if(foundZero){

        //Game is still on.
        return 0;
    }

    //Game over.
    return 2;

}

void RandomX() {

    srand(time(NULL));

    x = (rand() % 5) + 1;
}

void ShiftLeft(int *gameBoard) {

    int      ptr, checker;
    for (int i = 0; i < BOARD_SIZE; i++) {
        ptr     = 0;
        checker = 0;
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gameBoard[i * BOARD_SIZE + j] != 0) {
                if (ptr != j) {
                    gameBoard[i * BOARD_SIZE + ptr] = gameBoard[i * BOARD_SIZE +
                                                                j];
                    gameBoard[i * BOARD_SIZE + j]   = 0;
                }
                if ((ptr > checker) &&
                    (gameBoard[i * BOARD_SIZE + ptr] ==
                     gameBoard[i * BOARD_SIZE + ptr - 1])) {
                    gameBoard[i * BOARD_SIZE + ptr - 1] *= 2;
                    gameBoard[i * BOARD_SIZE + ptr] = 0;
                    checker = ptr;
                    --ptr;
                }
                ++ptr;
            }
        }
    }
}

void ShiftRight(int *gameBoard) {

    int      ptr, checker;
    for (int i = 0; i < BOARD_SIZE; i++) {
        ptr     = BOARD_SIZE - 1;
        checker = BOARD_SIZE - 1;
        for (int j = BOARD_SIZE - 1; j >= 0; j--) {
            if (gameBoard[i * BOARD_SIZE + j] != 0) {
                if (ptr != j) {
                    gameBoard[i * BOARD_SIZE + ptr] = gameBoard[i * BOARD_SIZE +
                                                                j];
                    gameBoard[i * BOARD_SIZE + j]   = 0;
                }
                if ((ptr < checker) &&
                    (gameBoard[i * BOARD_SIZE + ptr] ==
                     gameBoard[i * BOARD_SIZE + ptr + 1])) {
                    gameBoard[i * BOARD_SIZE + ptr + 1] *= 2;
                    gameBoard[i * BOARD_SIZE + ptr] = 0;
                    checker = ptr;
                    ++ptr;
                }
                --ptr;
            }
        }
    }
}

void ShiftDown(int *gameBoard) {
    //go over the integers array and move down and merge cells.
    int i, j, ptr, checker;
    for (i = 0; i < BOARD_SIZE; i++) {
        ptr     = BOARD_SIZE - 1;
        checker = BOARD_SIZE - 1;
        for (j  = BOARD_SIZE - 1; j >= 0; j--) {
            if (gameBoard[i + j * BOARD_SIZE] != 0) {
                if (ptr != j) {
                    gameBoard[i + BOARD_SIZE * ptr] = gameBoard[i +
                                                                BOARD_SIZE * j];
                    gameBoard[i + BOARD_SIZE * j]   = 0;
                }
                if ((ptr < checker) &&
                    (gameBoard[i + ptr * BOARD_SIZE] ==
                     gameBoard[i + (ptr + 1) * BOARD_SIZE])) {
                    gameBoard[i + (ptr + 1) * BOARD_SIZE] *= 2;
                    gameBoard[i + ptr * BOARD_SIZE] = 0;
                    checker = ptr;
                    ++ptr;
                }
                --ptr;
            }
        }
    }
}

void ShiftUp(int *gameBoard) {

    int      ptr, checker;
    for (int i = 0; i < BOARD_SIZE; i++) {
        ptr     = 0;
        checker = 0;
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gameBoard[j * BOARD_SIZE + i] != 0) {
                if (ptr != j) {
                    gameBoard[i + ptr * BOARD_SIZE] = gameBoard[i +
                                                                BOARD_SIZE * j];
                    gameBoard[i + BOARD_SIZE * j]   = 0;
                }
                if ((ptr > checker) &&
                    (gameBoard[i + ptr * BOARD_SIZE] ==
                     gameBoard[i + (ptr - 1) * BOARD_SIZE])) {
                    gameBoard[i + (ptr - 1) * BOARD_SIZE] *= 2;
                    gameBoard[i + ptr * BOARD_SIZE] = 0;
                    checker = ptr;
                    --ptr;
                }
                ++ptr;
            }
        }
    }
}
