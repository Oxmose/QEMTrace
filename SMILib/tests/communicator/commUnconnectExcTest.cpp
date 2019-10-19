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

    uint32_t excCnt = 0;

    Communicator* comm = nullptr;

    try 
    {
        comm = new PosixCommunicatorServer(COMM_SENDER,
                                           COMM_DATA_BLOCK_COUNT,
                                           COMM_DATA_BUFFER_BLOCK_SIZE);

        comm->init();
        //comm->connect(1, 5);;

        comm->send(NULL, 0);
    }
    catch(nsCommunicator::CommunicatorException& exc)
    {
        ++excCnt;
    }

    if(comm == nullptr)
    {
        failed("Communicator not created");
        return -1;

    }
    delete comm;

    try
    {
        comm = new PosixCommunicatorClient(COMM_RECEIVER);

        comm->init();
        //comm->connect(1, 5);;

        comm->receive(NULL, 0);
    }
    catch(nsCommunicator::CommunicatorException& exc)
    {
        ++excCnt;
    }

    if(comm == nullptr)
    {
        failed("Communicator not created");
        return -1;

    }
    delete comm;

    if(excCnt != 2)
    {
        failed("Should have raise exceptions");
        return -1;
    }

    passed("");

    return 0;
}