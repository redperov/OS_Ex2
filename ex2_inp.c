
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <memory.h>
#include <fcntl.h>

#define ARR_SIZE 16
#define MAX_NUM 5
#define BOARD_SIZE 4

/**
 * function name: SIGUSR1Handler.
 * The input: signal number.
 * The output: void.
 * The function operation: Handles SIGUSR1.
*/
void SIGUSR1Handler(int sigNum);

/**
 * function name: SIGINTHandler.
 * The input: signal number.
 * The output: void.
 * The function operation: Handles SIGINT.
*/
void SIGINTHandler(int signum);

/**
 * function name: ReadNumber.
 * The input: void.
 * The output: read number.
 * The function operation: reads numbers from file.
*/
int ReadNumber();

/**
 * function name: PrintNumber.
 * The input: number.
 * The output: void.
 * The function operation: prints number.
*/
void PrintNumber(int num);

/**
 * function name: PrintBorder.
 * The input: void.
 * The output: void.
 * The function operation: prints border.
*/
void PrintBorder();

/**
 * function name: OpenFileToRead.
 * The input: void
 * The output: file descriptor.
 * The function operation: opens file to read.
*/
int OpenFileToRead();

/**
 * function name: WriteMessage.
 * The input: message.
 * The output: void.
 * The function operation: writes message.
*/
void WriteMessage(char *message);

/**
 * function name: SendFinished.
 * The input: void.
 * The output: void.
 * The function operation: sends finish notification.
*/
void SendFinished();

//Variable declarations.
int stop;

int main() {

    //Variable declarations.
    struct sigaction usr_action;
    sigset_t         block_mask;
    int              closeResult;
    int              resultValue;

    //Open file to write data into
    int file = OpenFileToRead();

    //Set block mask.
    resultValue = sigfillset(&block_mask);

    //Check if sigfillset worked.
    if (resultValue < 0) {

        perror("Error: sigfillset failed.\n");
        exit(1);
    }

    usr_action.sa_handler = SIGUSR1Handler;
    usr_action.sa_mask    = block_mask;
    usr_action.sa_flags   = 0;

    //Set sigaction for SIGUSR1
    resultValue = sigaction(SIGUSR1, &usr_action, NULL);

    //Check if sigaction worked.
    if (resultValue < 0) {

        perror("Error: sigaction failed.\n");
    }

    usr_action.sa_handler = SIGINTHandler;

    //Set sigaction for SIGINT
    sigaction(SIGINT, &usr_action, NULL);

    //Check if sigaction worked.
    if (resultValue < 0) {

        perror("Error: sigaction failed.\n");
    }

    stop = 0;

    while (!stop) {
        continue;
    }

    //Close file.
    closeResult = close(file);

    //Check if file was closed.
    if (closeResult < 0) {

        perror("Error: failed to close file.\n");
        exit(1);
    }

    //Send finish notification.
    SendFinished();

}

int OpenFileToRead() {

    //Variable declarations.
    int dupResult;

    //Open file to read data from.
    int file = open("data.txt", O_RDONLY);

    //Check if file was opened.
    if (file < 0) {
        perror("Error\n");
        exit(1);
    }

    //Redirect to STDIN.
    dupResult = dup2(file, 0);

    //Check if dup2 worked.
    if(dupResult < 0){

        perror("Error: dup2 failed.\n");
        exit(1);
    }

    return file;
}

void SIGUSR1Handler(int sigNum) {

    if (!stop) {

        //Variable declarations.
        int board[BOARD_SIZE][BOARD_SIZE];
        int foundZero = 0;
        int found2048 = 0;
        int i;
        int j;

        //Fill the board.
        for ( i = 0; i < BOARD_SIZE; i++) {
            for ( j = 0; j < BOARD_SIZE; j++) {

                board[i][j] = ReadNumber();

                if (board[i][j] == 2048) {

                    found2048 = 1;
                } else if (board[i][j] == 0) {

                    foundZero = 1;
                }
            }
        }

        i = 0;
        j = 0;

        //Print the board.
        for (i = 0; i < BOARD_SIZE; i++) {

            for (j = 0; j < BOARD_SIZE; j++) {

                PrintNumber(board[i][j]);
            }

            PrintBorder();
        }

        WriteMessage("\n");

        //Check if game is over.
        if (found2048) {

            WriteMessage("Congratulations!\n");
            stop = 1;

        } else if (!foundZero) {

            WriteMessage("Game Over!\n");
            stop = 1;
        }
    }

}

void SIGINTHandler(int signum) {

    WriteMessage("BYE BYE\n");

    stop = 1;
}

void SendFinished() {

    int   killResult;
    pid_t parent = getppid();

    //Notify parent.
    killResult = kill(parent, SIGUSR2);

    //Check if signal was sent.
    if (killResult < 0) {

        perror("Error: kill failed.\n");
        exit(1);
    }
}

void WriteMessage(char *message) {

    int writeResult;

    writeResult = write(1, message, strlen(message));

    if (writeResult < 0) {

        perror("Error: write failed.\n");
        exit(1);
    }
}


int ReadNumber() {

    //Variable declarations.
    int  readResult         = 0;
    int  counter            = 0;
    int  endOfNumber        = 0;
    int  retNumber;
    char temp;
    int  writeResult;
    char rawNumber[MAX_NUM] = {0};

    while (counter < MAX_NUM && !endOfNumber) {

        readResult = read(0, &temp, 1);

        //Check if read.
        if (readResult < 0) {

            perror("Error: failed to read.\n");
            exit(1);
        }

        //Check if reached end of number.
        if (temp == ',') {

            endOfNumber = 1;
        } else {

            rawNumber[counter] = temp;
            counter++;
        }
    }

    rawNumber[counter] = '\0';

    //Convert result to an integer.
    retNumber = atoi(rawNumber);

    return retNumber;
}

void PrintNumber(int num) {

    char printResult[7];
    int  writeResult;

    if (num == 0) {

        sprintf(printResult, "|      ");
    } else {

        sprintf(printResult, "| %04d ", num);
    }

    writeResult = write(1, printResult, 7);

    if (writeResult < 0) {

        perror("Error: write failed.\n");
        exit(1);
    }

}

void PrintBorder() {

    int writeResult;

    writeResult = write(1, " |\n", 3);

    if (writeResult < 0) {

        perror("Error: write failed.\n");
        exit(1);
    }
}


