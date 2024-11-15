#include "MyMacros.h"
#include "MyData.h"
#include "MySignals.h"
#include "MyThreadFunctions.h"

int main(int argc, char *argv[])
{
	ARG x = {.argc= &argc, .argv = &argv};
	TaskManagerStart(&x);
	return 0;
}
