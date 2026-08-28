#include <stdio.h>
#include <unistd.h>
int main() {
int fd[2]; // file descripter
if(pipe(fd) == -1)
printf("pipe is not successful");
else{
printf("Read end of the pipe is %d \n",fd[0]);
printf("Write end of the pipe is %d \n",fd[1]);
}
}
