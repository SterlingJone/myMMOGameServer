#include <cstdio>
#include <signal.h>

#include "GameManager.h"
int main()
{
	signal(SIGPIPE, SIG_IGN);

	app::StartApp();

    return 0;
}