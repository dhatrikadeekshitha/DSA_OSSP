
#include<unistd.h>
int main() {
char buf[100];
read(0,buf,50);
write(1,buf,20);

}
