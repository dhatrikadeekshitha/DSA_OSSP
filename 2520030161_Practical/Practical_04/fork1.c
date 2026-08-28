#include<unistd.h>
#include<stdio.h>
int main() {
int pid;
pid=fork();
if(pid==0){
printf("Child process");
printf("If child process id is %d\n",getpid());
printf("Parent process id is %d\n",getppid());
}
else if(pid >0){
printf("Parent process \n");
printf("Child process id is %d\n",getpid());
printf("Parent process id is %d\n",getpid());
}
else
printf("fork isfailure");

}
