#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void timeoutHandler(int signum);

void CreateFile();

void WaitForInp();

void InpReady(int signum);

pid_t inp;
pid_t upd;
int   ready;

int main(int argc, char *argv[]) {

    int              time;
    int              status;
    struct sigaction usr_action1;
    sigset_t         block_mask1;
    struct sigaction usr_action2;
    sigset_t         block_mask2;
    int              parentId;

    // Establish the signal handler.
    sigfillset(&block_mask1);
    usr_action1.sa_handler = timeoutHandler;
    usr_action1.sa_mask    = block_mask1;
    usr_action1.sa_flags   = 0;
    sigaction(SIGALRM, &usr_action1, NULL);

    // Establish the signal handler.
    sigfillset(&block_mask2);
    usr_action2.sa_handler = InpReady;
    usr_action2.sa_mask    = block_mask2;
    usr_action2.sa_flags   = 0;
    sigaction(SIGUSR1, &usr_action2, NULL);

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

            perror("upd success\n");
        } else {

            // WaitForChildExec();

            //Activate program timer.
            //alarm(time);

            //TODO add check
            pause();
        }
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

void timeoutHandler(int signum) {

    printf("Program ended.\n");

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

void WaitForInp() {

    while (!ready) {
        sleep(1);
    }
}

void InpReady(int signum) {

    ready = 1;
}

int WaitForChildExec() {

    //Variable declarations.
    int waitVal;
    int status;

    waitVal = wait(&status);

    if (waitVal == -1) {

        perror("Error: wait failed.\n");
        exit(1);
    }

    //Check status.
    if (WIFEXITED(status)) {

        //Check if execution succeeded.
        if (WEXITSTATUS(status) == 1) {

            return 0;
        }

        return 1;
    }
}
