#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * function name: TimeoutHandler.
 * The input: signal number.
 * The output: void.
 * The function operation: Handles alarm.
*/
void TimeoutHandler(int signum);

/**
 * function name: timeoutHandler.
 * The input: signal number.
 * The output: void.
 * The function operation: Handles alarm.
*/
void CreateFile();

/**
 * function name: WaitForInp.
 * The input: void.
 * The output: void.
 * The function operation: Waits for second process.
*/
void WaitForInp();

/**
 * function name: InpReady.
 * The input: signal number.
 * The output: void.
 * The function operation: Notifies that it's ready.
*/
void InpReady(int signum);

/**
 * function name: FinishReceived.
 * The input: signal number.
 * The output: void.
 * The function operation: Handles finish.
*/
void FinishReceived(int signum);

//Variable declarations.
pid_t inp;
pid_t upd;
int   ready;
int   counter;

int main(int argc, char *argv[]) {

    //Variable declarations.
    int              time;
    int              status;
    struct sigaction usr_action;
    sigset_t         block_mask;
    int              parentId;
    int              resultValue;

    //Set block mask.
    resultValue = sigfillset(&block_mask);

    //Check if sigfillset worked.
    if (resultValue < 0) {

        perror("Error: sigfillset failed.\n");
        exit(1);
    }

    usr_action.sa_handler = TimeoutHandler;
    usr_action.sa_mask    = block_mask;
    usr_action.sa_flags   = 0;

    //Set sigaction for SIGALRM.
    resultValue = sigaction(SIGALRM, &usr_action, NULL);

    //Check if sigaction worked.
    if (resultValue < 0) {

        perror("Error: sigaction failed.\n");
    }

    // Establish the signal handler.
    usr_action.sa_handler = InpReady;

    //Set sigaction for SIGUSR1.
    resultValue = sigaction(SIGUSR1, &usr_action, NULL);

    //Check if sigaction worked.
    if (resultValue < 0) {

        perror("Error: sigaction failed.\n");
    }

    usr_action.sa_handler = FinishReceived;

    //Set sigaction for SIGUSR2.
    resultValue = sigaction(SIGUSR2, &usr_action, NULL);

    //Check if sigaction worked.
    if (resultValue < 0) {

        perror("Error: sigaction failed.\n");
    }

    parentId = getpid();

    ready = 0;

    //Set program time limit;
    time = atoi(argv[1]);

    //Create file for data transfer.
    CreateFile();

    //Execute ex2_inp.
    inp = fork();

    //Check if fork succeeded.
    if (inp < 0) {

        perror("Error: fork failed.\n");
        exit(1);

    } else if (inp == 0) {

        char *args[] = {"./ex2_inp.out", 0};
        int  inpExec;
        int  killResult;

        //Notify parent.
        killResult = kill(parentId, SIGUSR1);

        //Check kill was sent.
        if (killResult < 0) {
            perror("Error: kill failed.\n");
            exit(1);
        }

        //Execute ex2_inp.
        inpExec = execvp("./ex2_inp.out", args);

        //Check if execution worked.
        if (inpExec == -1) {

            perror("Error: inp execution failed.\n");
            exit(1);
        }
    } else {

        WaitForInp();

        //Execute ex2_upd.
        upd = fork();

        if (upd < 0) {

            perror("Error: fork failed.\n");
            exit(1);
        } else if (upd == 0) {

            char processId[256];
            memset(processId, 0, 256);
            sprintf(processId, "%d", inp);
            char *args[] = {"./ex2_upd.out", processId, 0};
            int  updExec;

            //Execute ex2_upd.
            updExec = execvp("./ex2_upd.out", args);

            //Check if execution worked.
            if (updExec == -1) {

                perror("Error: execution failed.\n");
                exit(1);
            }

        } else {


            //Activate program timer.
            alarm(time);

            while (counter < 2) {
                sleep(1);
            }
        }
    }

    //Unlink file.
    resultValue = unlink("data.txt");

    //Check if unlink worked.
    if (resultValue < 0) {

        perror("Error: unlink failed.\n");
        exit(1);
    }
}

void CreateFile() {

    int file;
    int closeResult;

    //Open file for data transfer.
    file = open("data.txt", O_CREAT, 0777);

    //Check if file was opened.
    if (file < 0) {

        perror("Error: failed to open file.\n");
        exit(1);
    }

    //Close file.
    closeResult = close(file);

    //Check if file was closed.
    if (closeResult < 0) {

        perror("Error: failed to close file.\n");
        exit(1);
    }
}

void TimeoutHandler(int signum) {

    alarm(0);

    int killResult;

    //Send signal to ex2_inp.
    killResult = kill(inp, SIGINT);

    //Check if signal was sent.
    if (killResult < 0) {

        perror("Error: signal failed.\n");
        exit(1);
    }

    //Send signal to ex2_upd.
    killResult = kill(upd, SIGINT);

    //Check if signal was sent.
    if (killResult < 0) {

        perror("Error: signal failed.\n");
        exit(1);
    }
}

void FinishReceived(int signum) {

    alarm(0);
    counter++;
}

void WaitForInp() {

    while (!ready) {
        sleep(1);
    }
}

void InpReady(int signum) {

    ready = 1;
}
