#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
int main(){
int fd;
size_t n;
fd=open("A7",O_WRONLY);
printf("%d",fd);
if(fd==-1){
printf("error cannot open the file\n");
}
else{
printf("success file opened in write mode\n");
}
n=read( fd,"hello",5);
printf("%ld\n",n);

}

