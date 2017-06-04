
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <memory.h>
#include <fcntl.h>

#define ARR_SIZE 16
#define MAX_NUM 5
#define BOARD_SIZE 4

void SIGUSR1Handler(int sigNum);

void SIGINTHandler(int signum);

int readNumber();

void printNumber(int num);

void printBorder();

int OpenFileToRead();

void WriteMessage(char *message);

void SendFinished();

int stop;

int main() {

    struct sigaction usr_action;
    sigset_t         block_mask;

    //Open file to write data into
    //TODO close file.
    int file = OpenFileToRead();
    int closeResult;

    //TODO make validation checks
    sigfillset(&block_mask);
    usr_action.sa_handler = SIGUSR1Handler;
    usr_action.sa_mask    = block_mask;
    usr_action.sa_flags   = 0;
    //TODO make validation checks
    sigaction(SIGUSR1, &usr_action, NULL);

    usr_action.sa_handler = SIGINTHandler;
    sigaction(SIGINT, &usr_action, NULL);

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

    SendFinished();

}

int OpenFileToRead() {

    //Open file to read data from.
    int file = open("data.txt", O_RDONLY);

    //Check if file was opened.
    if (file < 0) {
        perror("Error\n");
        exit(1);
    }

    //Redirect to STDIN.
    dup2(file, 0);

    return file;
}

void SIGUSR1Handler(int sigNum) {

    int board[BOARD_SIZE][BOARD_SIZE];
    int closeResult;
    int foundZero = 0;
    int found2048 = 0;

    //Fill the board.
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {

            board[i][j] = readNumber();

            if (board[i][j] == 2048) {

                found2048 = 1;
            } else if (board[i][j] == 0) {

                foundZero = 1;
            }
        }
    }

    //Print the board.
    for (int i = 0; i < BOARD_SIZE; i++) {

        for (int j = 0; j < BOARD_SIZE; j++) {

            printNumber(board[i][j]);
        }

        printBorder();
    }

    WriteMessage("\n");

    if (found2048) {

        WriteMessage("Congratulations!\n");
        //TODO handle exit
        stop = 1;

    } else if (!foundZero) {

        WriteMessage("Game Over!\n");
        stop = 1;
    }
}

void SIGINTHandler(int signum){

    WriteMessage("BYE BYE\n");

    stop = 1;
}

void SendFinished(){

    int killResult;
    pid_t  parent = getppid();

    killResult = kill(parent, SIGUSR2);

    if(killResult < 0){

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


int readNumber() {

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

void printNumber(int num) {

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

void printBorder() {

    int writeResult;

    writeResult = write(1, " |\n", 3);

    if (writeResult < 0) {

        perror("Error: write failed.\n");
        exit(1);
    }
}


