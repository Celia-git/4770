#include <iostream>
#include <string>
#include "msg.h"

int *
printmessage_1_svc(char *arg1, struct svc_req *rqstp) {
	static int result;
	std::string message = arg1;
	std::cout << "Received Message: " << message << std::endl;
	result = 1;
	return &result;
}
