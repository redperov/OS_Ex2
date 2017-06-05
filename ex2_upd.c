
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#define BOARD_SIZE 16
#define MOVE_SIZE 4

/**
 * function name: Initialize.
 * The input: process id.
 * The output: void.
 * The function operation: Initializes the game.
*/
void Initialize(pid_t pid);

/**
 * function name: SetRandomCell.
 * The input: void.
 * The output: void.
 * The function operation: Sets a random cell value.
*/
void SetRandomCell();

/**
 * function name: PrintBoardLine.
 * The input: void.
 * The output: void.
 * The function operation: Writes the board into a file.
*/
void PrintBoardLine();

/**
 * function name: SendUSR1.
 * The input: process id.
 * The output: void.
 * The function operation: Sends a SIGUSR1.
*/
void SendUSR1();

/**
 * function name: TimerHandler.
 * The input: signal number.
 * The output: void.
 * The function operation: Handles alarm signal.
*/
void TimerHandler(int sigNum);

/**
 * function name: SIGINTHandler.
 * The input: signal number.
 * The output: void.
 * The function operation: handles SIGINT signal.
*/
void SIGINTHandler(int signum);

/**
 * function name: LeftMove.
 * The input: game board.
 * The output: void.
 * The function operation: performs left movement.
*/
void LeftMove(int *grid);

/**
 * function name: RightMove.
 * The input: game board.
 * The output: void.
 * The function operation: performs right movement.
*/
void RightMove(int *grid);

/**
 * function name: UpMove.
 * The input: game board.
 * The output: void.
 * The function operation: performs up movement.
*/
void UpMove(int *grid);

/**
 * function name: DownMove.
 * The input: game board.
 * The output: void.
 * The function operation: performs down movement.
*/
void DownMove(int *grid);

/**
 * function name: RandomX.
 * The input: void.
 * The output: void.
 * The function operation: Sets random x value.
*/
void RandomX();

/**
 * function name: OpenFileToWrite.
 * The input: void.
 * The output: void.
 * The function operation: Opens file for writing.
*/
int OpenFileToWrite();

/**
 * function name: HandleUserCommand.
 * The input: command.
 * The output: void.
 * The function operation: Handles user input.
*/
void HandleUserCommand(char command);

/**
 * function name: DecideGameStatus.
 * The input: void.
 * The output: void.
 * The function operation: Decides game status.
*/
void DecideGameStatus();

/**
 * function name: CheckBoard.
 * The input: void.
 * The output: is finished.
 * The function operation: Checks if the board is finished.
*/
int CheckBoard();

/**
 * function name: SendFinished.
 * The input: void.
 * The output: void.
 * The function operation: Sends finished notification.
*/
void SendFinished();

//Variable declarations.
int   board[BOARD_SIZE];
pid_t pid;
int   x;
int   stop;


int main(int argc, char *argv[]) {

    //Check number of parameters.
    if (argc != 2) {

        perror("Error: wrong number of parameters.\n");
        exit(1);
    }

    //Variable declarations.
    int              file;
    int              closeResult;
    int              resultValue;
    char             command;
    struct sigaction usr_action;
    sigset_t         block_mask;

    //Set process id.
    pid = atoi(argv[1]);

    //Open file to read data from.
    file = OpenFileToWrite();

    //Initialize the game.
    Initialize(pid);

    //Set block mask.
    resultValue = sigfillset(&block_mask);

    //Check if sigfillset worked.
    if (resultValue < 0) {

        perror("Error: sigfillset failed.\n");
        exit(1);
    }
    usr_action.sa_handler = TimerHandler;
    usr_action.sa_mask    = block_mask;
    usr_action.sa_flags   = 0;

    //Set sigaction for SIGALRM.
    resultValue = sigaction(SIGALRM, &usr_action, NULL);

    //Check if sigaction worked.
    if (resultValue < 0) {

        perror("Error: sigaction failed.\n");
    }

    usr_action.sa_handler = SIGINTHandler;

    //Set sigaction for SIGINT.
    resultValue = sigaction(SIGINT, &usr_action, NULL);

    //Check if sigaction worked.
    if (resultValue < 0) {

        perror("Error: sigaction failed.\n");
    }

    //Set alarm.
    alarm(x);

    stop = 0;

    while (!stop) {

        //Read user input.
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

    //Close file.
    closeResult = close(file);

    //Check if file was closed.
    if (closeResult < 0) {

        perror("Error: failed to close file.\n");
        exit(1);
    }

    //Send finished notification.
    SendFinished();
}

void Initialize(pid_t pid) {

    //Variable declarations.
    int randCell1 = 0;
    int randCell2 = 0;
    int i;

    //Initialize board with zeros.
    for (i = 0; i < BOARD_SIZE; ++i) {

        board[i] = 0;
    }

    srand(time(NULL));

    //Generate random temp value.
    x = (rand() % 5) + 1;

    //Generate random cell positions.
    randCell1 = rand() % BOARD_SIZE;

    do {

        randCell2 = rand() % BOARD_SIZE;
    } while (randCell2 == randCell1);


    //Set random cells.
    board[randCell1] = 2;
    board[randCell2] = 2;

    //Print board in line format.
    PrintBoardLine();

    //Send signal to process.
    SendUSR1();

}

int OpenFileToWrite() {

    int dupResult;

    //Open file to read data from.
    int file = open("data.txt", O_WRONLY);

    //Check if file was opened.
    if (file < 0) {
        perror("Error: failed to open file.\n");
        exit(1);
    }

    //Redirect to STDOUT.
    dupResult = dup2(file, 1);

    //Check if dup2 worked.
    if(dupResult < 0){

        perror("Error: dup2 failed.\n");
        exit(1);
    }

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

    int boardStatus = CheckBoard();

    //Check if game can continue.
    if (boardStatus == 0) {

        alarm(x);
    } else {

        stop = 1;
    }

}

void SIGINTHandler(int signum) {

    alarm(0);
    stop = 1;
}

void HandleUserCommand(char command) {

    switch (command) {

        case 'A':
            //Move left.
            LeftMove(board);
            DecideGameStatus();
            break;

        case 'D':
            //Move right.
            RightMove(board);
            DecideGameStatus();
            break;

        case 'W':
            //Move up.
            UpMove(board);
            DecideGameStatus();
            break;

        case 'X':
            //Move down.
            DownMove(board);
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

void SendFinished() {

    //Variable declarations.
    int   killResult;
    pid_t parent = getppid();

    //Send signal to parent.
    killResult = kill(parent, SIGUSR2);

    //Check if signal was sent.
    if (killResult < 0) {

        perror("Error: kill failed.\n");
        exit(1);
    }
}

void SetRandomCell() {

    //Variable declarations.
    int isDone = 0;
    int randCell;

    srand(time(NULL));

    while (!isDone) {

        //Generate random value.
        randCell = rand() % BOARD_SIZE;

        if (board[randCell] == 0) {

            board[randCell] = 2;
            isDone = 1;
        }
    }
}

void SendUSR1() {

    int killResult;

    if (!stop) {

        killResult = kill(pid, SIGUSR1);

        //Check if signal was sent.
        if (killResult < 0) {

            perror("Error: signal failed.\n");
            exit(1);
        }
    }

}

void PrintBoardLine() {

    //Variable declarations.
    int writeResult;
    int i;

    for (i = 0; i < BOARD_SIZE; ++i) {

        char number[6];
        memset(number, 0, 6);

        //Convert number to string.
        if (i == BOARD_SIZE - 1) {

            sprintf(number, "%d\n", board[i]);
        } else {

            sprintf(number, "%d,", board[i]);
        }

        //Write value to file.
        writeResult = write(1, number, strlen(number));

        //Check if wrote.
        if (writeResult < 0) {

            perror("Error: write failed.\n");
            exit(1);
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

    } else {

        stop = 1;
    }
}

int CheckBoard() {

    int foundZero = 0;
    int i;

    for (i = 0; i < BOARD_SIZE; ++i) {

        if (board[i] == 2048) {

            //Game won.
            return 1;
        }

        if (board[i] == 0) {

            foundZero = 1;
        }

    }

    if (foundZero) {

        //Game is still on.
        return 0;
    }

    //Game over.
    return 2;

}

void RandomX() {

    srand(time(NULL));

    //Generate random value.
    x = (rand() % 5) + 1;
}

void UpMove(int *grid) {

    //Variable declarations.
    int position;
    int validate;
    int rows;
    int columns;

    for (rows = 0; rows < MOVE_SIZE; rows++) {

        position = 0;
        validate = 0;

        for (columns = 0; columns < MOVE_SIZE; columns++) {

            if (grid[columns * MOVE_SIZE + rows] != 0) {

                if (position != columns) {

                    grid[rows + position * MOVE_SIZE] = grid[rows +
                                                             MOVE_SIZE *
                                                             columns];
                    grid[rows + MOVE_SIZE * columns]  = 0;
                }

                if ((position > validate) &&
                    (grid[rows + position * MOVE_SIZE] ==
                     grid[rows + (position - 1) * MOVE_SIZE])) {

                    grid[rows + (position - 1) * MOVE_SIZE] *= 2;
                    grid[rows + position * MOVE_SIZE] = 0;

                    validate = position;
                    --position;
                }

                ++position;
            }
        }
    }
}

void DownMove(int *grid) {

    //Variable declarations.
    int rows;
    int columns;
    int position;
    int validate;

    for (rows = 0; rows < MOVE_SIZE; rows++) {

        position = MOVE_SIZE - 1;
        validate = MOVE_SIZE - 1;

        for (columns = MOVE_SIZE - 1; columns >= 0; columns--) {

            if (grid[rows + columns * MOVE_SIZE] != 0) {

                if (position != columns) {

                    grid[rows + MOVE_SIZE * position] = grid[rows +
                                                             MOVE_SIZE *
                                                             columns];
                    grid[rows + MOVE_SIZE * columns]  = 0;
                }

                if ((position < validate) &&
                    (grid[rows + position * MOVE_SIZE] ==
                     grid[rows + (position + 1) * MOVE_SIZE])) {

                    grid[rows + (position + 1) * MOVE_SIZE] *= 2;
                    grid[rows + position * MOVE_SIZE] = 0;

                    validate = position;
                    ++position;
                }

                --position;
            }
        }
    }
}

void LeftMove(int *grid) {

    //Variable declarations.
    int position;
    int validate;
    int rows;
    int columns;

    for (rows = 0; rows < MOVE_SIZE; rows++) {

        position = 0;
        validate = 0;

        for (columns = 0; columns < MOVE_SIZE; columns++) {

            if (grid[rows * MOVE_SIZE + columns] != 0) {

                if (position != columns) {

                    grid[rows * MOVE_SIZE + position] = grid[rows * MOVE_SIZE +
                                                             columns];
                    grid[rows * MOVE_SIZE + columns]  = 0;
                }

                if ((position > validate) &&
                    (grid[rows * MOVE_SIZE + position] ==
                     grid[rows * MOVE_SIZE + position - 1])) {

                    grid[rows * MOVE_SIZE + position - 1] *= 2;
                    grid[rows * MOVE_SIZE + position] = 0;

                    validate = position;
                    --position;
                }

                ++position;
            }
        }
    }
}

void RightMove(int *grid) {

    //Variable declarations.
    int position;
    int validate;
    int rows;
    int columns;

    for (rows = 0; rows < MOVE_SIZE; rows++) {

        position = MOVE_SIZE - 1;
        validate = MOVE_SIZE - 1;

        for (columns = MOVE_SIZE - 1; columns >= 0; columns--) {

            if (grid[rows * MOVE_SIZE + columns] != 0) {

                if (position != columns) {

                    grid[rows * MOVE_SIZE + position] = grid[rows * MOVE_SIZE +
                                                             columns];
                    grid[rows * MOVE_SIZE + columns]  = 0;
                }

                if ((position < validate) &&
                    (grid[rows * MOVE_SIZE + position] ==
                     grid[rows * MOVE_SIZE + position + 1])) {

                    grid[rows * MOVE_SIZE + position + 1] *= 2;
                    grid[rows * MOVE_SIZE + position] = 0;

                    validate = position;
                    ++position;
                }

                --position;
            }
        }
    }
}



