#include <iostream>
#include <string>
#include "msg.h"

int main(int argc, char **argv) 
{
	CLIENT *clnt;
	int *result;
	char *server_name;
	char *message;

	if (argc < 3) {
		std::cerr << "usage: " << argv[0] << " server_name message" << std::endl;
		return 1;
	}

	server_name = argv[1];
	message = argv[2];

	clnt = clnt_create(server_name, MESSAGE_PROG, MESSAGE_VERS, "netpath");
	if (clnt == NULL) {
		clnt_pcreateerror(server_name);
		return 1;
	}

	result = printmessage_1(message, clnt);
	if (result == NULL) {
		clnt_perror(clnt, "FAIL");
	} else {
		std::cout << "SUCCESS.  Result: " << *result << std::endl;
	}
	clnt_destroy(clnt);
	return 0;
}
