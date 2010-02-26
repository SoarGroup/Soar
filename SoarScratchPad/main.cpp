#include <portability.h>

#include "sml_Client.h"

int main(int argc, char** argv)
{
	sml::Kernel* k = sml::Kernel::CreateKernelInNewThread();
	sml::Agent* a = k1->CreateAgent("soar");

	k->Shutdown();

	return 0;
}