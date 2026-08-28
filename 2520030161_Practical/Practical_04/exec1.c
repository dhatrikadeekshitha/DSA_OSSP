#include <unistd.h>

int main()
{
    execl("/bin/pwd", "pwd", NULL);
    return 0;
}
