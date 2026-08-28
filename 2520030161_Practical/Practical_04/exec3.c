#include<unistd.h>
#include<stdio.h>
int main() {
char *args[] = {"ls","-l","-F",NULL};
execvp("ls",args);
perror("execv not succeded");
}
