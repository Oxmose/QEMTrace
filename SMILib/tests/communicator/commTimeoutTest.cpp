#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "communicator/PosixCommunicatorServer.h"
#include "communicator/PosixCommunicatorClient.h"
#include "communicator/CommunicatorException.h"

using namespace nsCommunicator;

#define COMM_DATA_BUFFER_BLOCK_SIZE   ((uint32_t)(1024*1024*2)) /* 2MB */
#define COMM_DATA_BLOCK_COUNT         4


void passed(const char* msg)
{
	std::cout << "\033[21;32mPASSED\033[0m\t" << msg << std::endl;
}

void failed(const char* msg)
{
	std::cerr << "\033[21;31mFAILED\033[0m\t" << msg << std::endl;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    std::string branch = "UNKNOWN";
    Communicator* comm = nullptr;
    uint32_t excCount = 0;

    try
    {
        branch = "[Server] "; 
        delete comm;
        comm = new PosixCommunicatorServer(COMM_SENDER, 5, 5);

        comm->init();
        comm->connect(1, 2);

        delete comm;
    }
    catch(nsCommunicator::CommunicatorException& exc)
    {
        std::cout << branch << "Caught exception:\n\t" << exc.what() 
                << std::endl;
        ++excCount;
    }

    try
    {
        branch = "[Client] "; 
        comm = new PosixCommunicatorClient(COMM_RECEIVER);

        comm->init();
        comm->connect(1, 2);

        delete comm;
    }
    catch(nsCommunicator::CommunicatorException& exc)
    {
        std::cout << branch << "Caught exception:\n\t" << exc.what() 
                << std::endl;
        ++excCount;
    }


    if(excCount != 2)
    {
        failed("Should have timedout");
        return 1;
    }
    else 
    {
        passed("");
        return 0;
    }
}