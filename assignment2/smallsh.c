//########################################################################################
//AUTHOR: Garrett Haley
//CREDIT: Matt Baker, Brandon Lee, StackOverflow, Benjamin Brewster, xiaoli liu
//COURSE: CS344 Oregon State University
//PROGRAM: 3
//DATE: 05/22/2017
//#######################################################################################
//LIBRARYS INCLUDED/////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
///THE STRUCTS//////////////////////////////////////////////////////////////////////////
//Linked list of PID's
typedef struct linkedPID {
    int currentPID; 
    struct linkedPID *next;
} linkedPID;
// User input struct (gathers up the user input)
typedef struct userInput {
    bool switchIn;
    bool switchOut;
    char *ingoing;
    char *outgoing;
    char *userCommand;
    char *userArgs;
    bool qualificationProcess;
} userInput;
/////////////////////////////////////////////////////////////////////////////////////////
// COMMAND CHOICES, FLAGS, LINKEDLIST AND salientFlag INITIALIZATION///////////////////
char *userCommands[] = {"status","cd", "exit"};//commands
linkedPID *head, *current; //sets head of linked list to current
bool shellIsRunning = true; // sets position of shell
bool EXITSTATUS = false;  // initializes the exit value to false until point of exit
bool salientFlag = false; //initialies flag position
///////////////////////////////////////////////////////////////////////////////////////
// Setup single linked li
// Function declarations///////////////////////////////////////////////////////////
void executionSwitch(userInput *usersCommand, int fd);
void helpExecute(userInput *usersCommand, int tokenBuffer, int index);
void childProcAssistant(userInput *usersCommand, pid_t pid, pid_t wpid, int status);
void tidyShell();
void loadShell();
void executes(userInput *usersCommand);
void enviornmentCheck();
bool checkInternalCommand(char *usersCommand);
void executeInternalCommand(userInput *usersCommand);
void execute(userInput *usersCommand);
userInput *gatherUserInput(int argBuffer, int lineBuffer);
void abnormalCharInput(userInput *usersCommand, int argBuffer, char *ArgsBuffer, char *switchInputDirection, char *switchOutputRedirection);
///////////////////////////////////////////////////////////////////////////////////////
userInput *gatherUserInput(int argBuffer, int lineBuffer) {
    bool newLine = false;
    char *UIBuffer = (char*) malloc(sizeof(char) * lineBuffer);//ALLOCATION OF VARS///////////////////////
    char *ArgsBuffer = (char*) malloc(sizeof(char) * argBuffer);//ALLOCATION OF VARS///////////////////////
    char *switchInputDirection = (char*) malloc(sizeof(char) * argBuffer);//ALLOCATION OF VARS///////////////////////
    char *switchOutputRedirection = (char*) malloc(sizeof(char) * argBuffer);//ALLOCATION OF VARS///////////////////////
    userInput *usersCommand = (userInput *)malloc(sizeof(userInput));//ALLOCATION OF VARS///////////////////////
    usersCommand->userCommand = (char*) malloc(sizeof(char) * lineBuffer);//ALLOCATION OF VARS///////////////////////
    usersCommand->userArgs = (char*) malloc(sizeof(char) * argBuffer);//ALLOCATION OF VARS///////////////////////
    usersCommand->qualificationProcess = false;//DEFAULT VAR VALUES////////////////////////////
    usersCommand->switchIn = false;//DEFAULT VAR VALUES////////////////////////////
    usersCommand->switchOut = false;//DEFAULT VAR VALUES////////////////////////////
    usersCommand->ingoing = NULL;//DEFAULT VAR VALUES////////////////////////////
    usersCommand->outgoing = NULL;//DEFAULT VAR VALUES////////////////////////////
    while (true) {
        int currentChar = getchar();//get the character
        if (!isspace(currentChar)) {//cehck if its a space
            ungetc(currentChar, stdin);//dont use that one if it is!
            break;
        }
        if (currentChar == '\n' || currentChar == EOF) {//if its a new line or end of file, break!
            UIBuffer[0] = '\0';
            newLine = 1;
            break;
        }
    }
    int i = 0;
    while (!newLine) {//While we are one the same line still
        int currentChar = getchar(); //get the character
        if (isspace(currentChar) || currentChar == '\n' || currentChar == EOF || (i == lineBuffer - true)) {//check if that value is not what you want
            if (currentChar == EOF || currentChar == '\n') {//if its one of these choices, move to a different line!!
                usersCommand->userArgs[0] = '\0';
                newLine = true;
            }
            UIBuffer[i] = '\0';//set buffer to null termination
            break;
        }
        UIBuffer[i] = currentChar;//set value at location i to the current character
        i++;
        if (lineBuffer <= i) {//if the buffer is less that i
            lineBuffer += lineBuffer;//increase it
            usersCommand->userCommand = realloc(usersCommand->userCommand, (lineBuffer * sizeof(char)));//and reallocate space for usersCommands
            UIBuffer = realloc(UIBuffer, (lineBuffer * sizeof(char)));
        }
    }
    i = 0;
    bool Abnormal = false;//sets Abnormal characters to initally false
    while (!newLine) {//while we are still on the same line
        int currentChar = getchar();//same as code above!
        if (currentChar == '\n' || currentChar == EOF || (i == lineBuffer - 1)) {
            ArgsBuffer[i] = '\0';
            break;
        }
        if (currentChar == '&'|| currentChar == '<' || currentChar == '>') {// now this checks if the char grabbed is some sort of Abnormal character that we want
            Abnormal = true;//set Abnormal bool to true
        }
        ArgsBuffer[i] = currentChar;
        i++;
        if (argBuffer <= i) {//same as code above, reallocate if needed
            argBuffer += argBuffer;
            switchInputDirection = realloc(switchInputDirection, (argBuffer * sizeof(char)));
            switchOutputRedirection = realloc(switchInputDirection, (argBuffer * sizeof(char)));
            ArgsBuffer = realloc(ArgsBuffer, (argBuffer * sizeof(char)));
        }
    }
    if (Abnormal) {
        abnormalCharInput(usersCommand, argBuffer, ArgsBuffer, switchInputDirection, switchOutputRedirection);//if we have a Abnormal character, run it through our function for them
    }
    strcpy(usersCommand->userCommand, UIBuffer);//copy commands into buffer
    if (usersCommand->switchOut) {//if true
        usersCommand->outgoing = (char*)malloc(sizeof(switchOutputRedirection) / sizeof(char*));//malloc redict
        strcpy(usersCommand->outgoing, switchOutputRedirection);//copy outgoing to redirect
    }

    if (usersCommand->switchIn) {//if switchin is true
        usersCommand->ingoing = (char*)malloc(sizeof(switchInputDirection) / sizeof(char*));//malloc size of inputredirect instead of output like above
        strcpy(usersCommand->ingoing, switchInputDirection);//same as above
    }

    if (!newLine) {
        strcpy(usersCommand->userArgs, ArgsBuffer);
    }
    free(switchInputDirection);//FREE ALLOCATION
    free(switchOutputRedirection);//FREE ALLOCATION
    free(UIBuffer);//FREE ALLOCATION
    free(ArgsBuffer);//FREE ALLOCATION
    fflush(stdout);//then flush standard out!
    return usersCommand;//return the commands
}
//FUNCITON FOR SPECIAL CHARACTERS//////////////////////////////////////////////////////
void abnormalCharInput(userInput *usersCommand, int argBuffer, char *ArgsBuffer, char *switchInputDirection, char *switchOutputRedirection) {
    int startChar = -1;//INITIALIZE
    int backChar = -1;//INITIALIZE
    int inChar = -1;//INITIALIZE
    int outChar = -1;//INITIALIZE
    int i = 0;//INITIALIZE
    for (i = 0; i < strlen(ArgsBuffer); i++) {// if i is less than the length of the argument buffer and a Abnormal char and all chars are -1, its the start!
        if (ArgsBuffer[i] == '&') {//Checks if its the symbol to the left
            if (startChar == -1 && backChar == -1 && inChar == -1 && outChar == -1) {
                startChar = i;
            }
            backChar = i;
            usersCommand->qualificationProcess = true;
        }
        if (ArgsBuffer[i] == '<') {//Checks if its the symbol to the left
            if (startChar == -1 && backChar == -1 && inChar == -1 && outChar == -1) {
                startChar = i;
            }
            inChar = i;
            usersCommand->switchIn = true;
        }
        if (ArgsBuffer[i] == '>') {//Checks if its the symbol to the left
            if (startChar == -1 && backChar == -1 && inChar == -1 && outChar == -1) {
                startChar = i;
            }
            outChar = i;
            usersCommand->switchOut = true;
        }
    }
    if (-1 < outChar) {//if outChar now has some value after start
        int k = 0;
        for (i = outChar + 1; i < strlen(ArgsBuffer); i++) {
            if (!isspace(ArgsBuffer[i])) {
                switchOutputRedirection[k] = ArgsBuffer[i];//fill switchOutputRedirection with the argument buffer vals passed in
                k++;
            }
            if (isspace(ArgsBuffer[i]) && 0 < k) {
                break;
            }
        }
        switchOutputRedirection[k] = '\0';//set to null termination
    }
    if (-1 < inChar) {//Does the same as the code above but instead fills switchInputDirection instead of switchOutputRedirection
        int k = 0;
        for (i = inChar + 1; i < strlen(ArgsBuffer); i++) {
            if (!isspace(ArgsBuffer[i])) {
                switchInputDirection[k] = ArgsBuffer[i];
                k++;
            }
            if (isspace(ArgsBuffer[i]) && 0 < k){
                break;
            }
        }
        switchInputDirection[k] = '\0';
    }
    char *newBuffer = (char*) malloc(sizeof(char) * argBuffer);//ALLOCATE NEW BUFFER
    strncpy(newBuffer, ArgsBuffer, startChar);
    newBuffer[startChar] = '\0';//set to null termination
    free(ArgsBuffer);//free that old ArgsBuffer
    ArgsBuffer = (char*) malloc(sizeof(char) * argBuffer);//reallocate that ArgsBuffer
    strcpy(ArgsBuffer, newBuffer);
    free(newBuffer);//free newbuffer after old has recopied new
}
//FUNCTION EXECUTES USERS COMMANDS////////////////////////////
void executes(userInput *usersCommand) {
    enviornmentCheck();//background check!
    if (usersCommand->userCommand[0] != '\0' && usersCommand->userCommand[0] != '#') {//checks is usersCommand is not null or #
        if (checkInternalCommand(usersCommand->userCommand)) {//if its an InternalCommand
            executeInternalCommand(usersCommand);//execute it
        } else {
            execute(usersCommand);//else move to execute from executes
        }
    }
}
//FUNCTION CHECKS BACKGROUND///////////////////////////////////
void enviornmentCheck() {
    int pid;
    int status;
    do {
        pid = waitpid(-1, &status, WNOHANG);
        if (0 < pid) {//Checks to see if background PID is done
            printf("background PID %d is done: ", pid);//lets user know whats going on
            if (WIFSIGNALED(status) != 0) {//checks if proc was ended by signal
                printf("ended by signal %d\n", WTERMSIG(status));
            }
            if (WIFEXITED(status)) {//gives status update
                printf("exit value %d\n", WEXITSTATUS(status));
            }
        }
    } while (0 < pid);
}
//FUNCTION CHECKS FOR INTERNAL COMMANDS
bool checkInternalCommand(char *usersCommand) {
    int i;
    for (i = 0; i < 3; i++) {
        if (!strcmp(userCommands[i], usersCommand)) {//if they are the same return its internal
            return true;
        }
    }

    return false;//not internal
}
//FUNCTION EXECUTES INTERAL COMMANDS
void executeInternalCommand(userInput *usersCommand) {
    if (!strcmp(usersCommand->userCommand, "status")) {//if the command is status
        if (salientFlag == false) {
            printf("exit value %d\n", EXITSTATUS);//print its exit stats
        } else {
            printf("terminated: signal %d\n", salientFlag);//print the terminated signal to user
        }
    } else if (!strcmp(usersCommand->userCommand, "cd")) {//if command is status
        if (usersCommand->userArgs[0] == '\0') {
            char *home = getenv("HOME");//change dir
            if (home != NULL) {
                chdir(home);
            }
        } else if (chdir(usersCommand->userArgs) != false) {//checks if that command is even in our set
            printf("I don't recognize that command \"%s\"\n", usersCommand->userArgs);//returns if it can't find the command
            EXITSTATUS = true;//exit
        }
    } else if (!strcmp(usersCommand->userCommand, "exit")) {//if command is exit, stop the shell!
        shellIsRunning = false;
    }
}
//FUNCTION EXECUTES USER COMMANDS
void execute(userInput *usersCommand) {
    int status;//INITIALIZE
    pid_t pid;//INITIALIZE
    pid_t wpid;//INITIALIZE
    pid = fork();//INITIALIZE
    int index = 0;//INITIALIZE
    int tokenBuffer = 64;//INITIALIZE
    if (pid == 0) {
        struct sigaction act;//create sig struct
        act.sa_flags = 0;//INITIALIZE
        act.sa_handler = SIG_DFL;//INITIALIZE
        sigaction(SIGINT, &act, 0);//INITIALIZE
        int fd;//INITIALIZE
        if (usersCommand->switchIn || usersCommand->switchOut) {//checks if executeSwitch needs to be ran
            executionSwitch(usersCommand, fd);
        } else if (!usersCommand->switchIn && usersCommand->qualificationProcess) {//if it doesn't open fl
            fd = open("/dev/null", O_RDONLY, 0644);
            if (fd == -1) {// if this is true return that you can't open it
                printf("I can't open /dev/null for stdin redirection!\n");
                exit(1);
            } else {
                if (dup2(fd, 0) == -1) {//checks if you can switch stdin to pth
                    printf("Can't switch stdin to /dev/null!\n");
                    exit(1);
                }

                close(fd);//close it
            }
        }
      helpExecute(usersCommand, tokenBuffer, index);//sent to helper func.
    } else if (0 > pid) {//if pid is greater than zero, theres a forking error, let the user know
        printf("Some kind of fork error!\n");
        EXITSTATUS = true;
    } else {
        childProcAssistant(usersCommand, pid, wpid, status);//else use the childprocessassistant function
    }
}
//FUNCTION EXECUTES THE SWITCH FUNCTIONALITY/////////////////////////////////////////////////////////////
void executionSwitch(userInput *usersCommand, int fd) {
    if (usersCommand->switchOut) {
        fd = open(usersCommand->outgoing, O_WRONLY | O_CREAT | O_TRUNC, 0644);//open for process
        if (fd == -1) {
            printf("file cannot be opened for stdout switching!!: %s\n", usersCommand->outgoing);//return if file can be opened
            exit(1);
        } else {
            if (dup2(fd, 1) == -1) {//checks for dup in fd
                printf("Cannot switch stdout!\n");//if true return error message
                exit(1);//exit
            }

            close(fd);//close the file
        }
    }
    if (usersCommand->switchIn) {//same funcitonlity as above in the swithout portion
        fd = open(usersCommand->ingoing, O_RDONLY, 0644);
        if (fd == -1) {
            printf("This file can't be opened: %s\n", usersCommand->ingoing);//same funcitonality as section above
            exit(1);
        } else {
            if (dup2(fd, 0) == -1) {
                printf("Could not switch to stdin!\n");//same as above
                exit(1);
            }

            close(fd);//close that file
        }
    }
}
//FUNCTION ASSISTS ON EXECUTION OF COMMANDS/////////////////////////////////////////
void helpExecute(userInput *usersCommand, int tokenBuffer, int index) {
    char **userArgs = malloc(sizeof(char*) * tokenBuffer);//ALLOCATE userArgs space
    userArgs[index] = usersCommand->userCommand;//set indexed location to userCommand
    index++;//move the index given in function params
    char *token = strtok(usersCommand->userArgs, " ");//sets token
    while (token != NULL) {//if the tokens not null
        userArgs[index] = token;//set the token into its indexed spot and move the index
        index++;
        if (tokenBuffer <= index) {//if the buffor is less than the index, then dynamically realocate
            tokenBuffer += tokenBuffer;
            userArgs = realloc(userArgs, (tokenBuffer * sizeof(char*)));//REALLOCATE userArgs
        }
        token = strtok(NULL, " ");
    }
    userArgs[index] = NULL;
    if (execvp(userArgs[0], userArgs) == -1) {//if this is true, then the command is not recognizable, let the user know
        printf("I don't recognize that command!\n");
        exit(1);
    }
}
//FUNCTION ASSISTS WITH CHILD PIDS///////////////////////////////////////////////////////////
void childProcAssistant(userInput *usersCommand, pid_t pid, pid_t wpid, int status) {
    do {
        if (usersCommand->qualificationProcess) {
            while (head->next != NULL) {
                head = head->next;
            }
            head->next = (linkedPID *)malloc(sizeof(linkedPID));//malloc next link to a link size
            head->currentPID = pid;//chance head of linked list
            head = head->next;//move the header to next
            head->next = NULL;//set the header of the header to null because its null terminated
            printf("Background PID is %d\n", pid);//give the PID to user
            fflush(stdout);//flush the standard out
        } else {
            wpid = waitpid(pid, &status, WUNTRACED);//wait for PID
        }
    } while (!WIFSIGNALED(status) && !WIFEXITED(status));//while not exited
    if (WIFEXITED(status)) {
        EXITSTATUS = WEXITSTATUS(status);
        salientFlag = false;//reset flag if exited
    } else if (WIFSIGNALED(status)) {
        EXITSTATUS = true;//reset flag is exited
        salientFlag = WTERMSIG(status);
    }
}
//FUNCTION FREES AND CLEANS UP AFTER SHELL ALLOCATION
void tidyShell() {
    current = head;
    while (current != NULL) {
        if (current->currentPID != -1) {
            kill(current->currentPID, SIGKILL);//loop through and kill the PIDS
        }
        current = current->next;
    }
    while ((current = head) != NULL) {
        head = head->next;
        free(current);//free the used memory
    }
}
//MAIN FUNCTION//////////////////////////////////////////////////////////////
int main() {
    loadShell();//loads the shell
    while (shellIsRunning) {//keeps shell running
        printf(" : ");//prompts user
        fflush(stdout);//flush standard out 
        executes(gatherUserInput(512, 2048));//gathers input
    }
    tidyShell();//tidy's up after the shell makes an allocation mess
    return EXIT_SUCCESS;
}
void loadShell() {
    head = (linkedPID *)malloc(sizeof(linkedPID));//ALLOCATE HEAD
    current = head;//SET HEAD
    current->currentPID = -1;//SET PID
    current->next = NULL;//NULL TERMINATING
    struct sigaction act;//CREATE SIGNAL STRUCT
    act.sa_flags = 0;//INITIALIZE
    act.sa_handler = SIG_IGN;//INITIALIZE
    sigaction(SIGINT, &act, NULL);//INITIALIZE
}
