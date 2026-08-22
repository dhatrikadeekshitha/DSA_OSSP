#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
int main() {
int fd1,fd2;
char buf[100];
size_t n;
fd1=open("open3.c",O_RDWR);
fd2=open("z7",O_RDWR);
n=read( fd1,buf,5);
write(fd2,buf,n);
printf("%s",buf);
printf("Files successfully opened and created\n");
printf("%d",fd1);
printf("%d",fd2);

}
