#include "GameManager.h"

#include <cstdio>
#include <signal.h>
#include <unistd.h>
int main()
{
    signal(SIGPIPE, SIG_IGN);
    app::StartApp();
    return 0;
}