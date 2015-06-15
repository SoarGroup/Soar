//
//  main.cpp
//  LaunchDebugger
//
//  Created by Alex Turner on 6/15/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include <iostream>

#include "sml_Client.h"

int main(int argc, const char * argv[]) {
	sml::Agent* agent = nullptr;
	sml::Kernel* kernel = sml::Kernel::CreateKernelInNewThread();
	agent = kernel->CreateAgent("soar1");
	
	agent->SpawnDebugger();
	
	sleep(-1);
	
    return 0;
}
