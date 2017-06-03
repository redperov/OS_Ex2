
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <memory.h>
#include <fcntl.h>

#define ARR_SIZE 16
#define MAX_NUM 5
#define BOARD_SIZE 4

void signalHandler(int sigNum);

int readNumber();

void printNumber(int num);

void printBorder();

int OpenFileToRead();

int main() {

    struct sigaction usr_action;
    sigset_t         block_mask;

    //Open file to write data into
    //TODO close file.
    int file = OpenFileToRead();

    //TODO make validation checks
    sigfillset(&block_mask);
    usr_action.sa_handler = signalHandler;
    usr_action.sa_mask    = block_mask;
    usr_action.sa_flags   = 0;
    //TODO make validation checks
    sigaction(SIGUSR1, &usr_action, NULL);


    while (1) {

    }

    //TODO close file when process is over
    /* //Close file.
   closeResult = close(file);

    //Check if file was closed.
    if(closeResult < 0){

        perror("Error: failed to close file.\n");
        exit(1);
    }*/

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

void signalHandler(int sigNum) {

    int board[BOARD_SIZE][BOARD_SIZE];
    int closeResult;

    //Fill the board.
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {

            board[i][j] = readNumber();
        }
    }

    //Print the board.
    for (int i = 0; i < BOARD_SIZE; i++) {

        for (int j = 0; j < BOARD_SIZE; j++) {

            printNumber(board[i][j]);
        }

        printBorder();
    }

    printf("\n");
}

int readNumber() {

    int  readResult         = 0;
    int  counter            = 0;
    int  endOfNumber        = 0;
    int  retNumber;
    char temp;
    char rawNumber[MAX_NUM] = {0};

    while (counter < MAX_NUM && !endOfNumber) {

        readResult = read(0, &temp, 1);

        //Check if read.
        if (readResult < 0) {

            perror("Error: failed to read.\n");
            exit(1);
        }

        if(temp == 'l'){

            printf("Game Over!");
            //TODO handle exit like requested.
            exit(0);
        }
        else if(temp == 'w'){

            printf("Congratulations!");
            //TODO handle exit like requested.
            exit(0);
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


