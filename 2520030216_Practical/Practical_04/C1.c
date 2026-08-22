#include<unistd.h>
#include<stdio.h>
int main(){
int pid=fork();
if(pid==0){
printf("hello");
}
if(pid>0){
printf("hai");
}
fork();

	printf("hello\n");
}

