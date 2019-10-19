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

#define LOOP_COUNT ((20 * 1024 * 1024) / sizeof(uint32_t))

void passed(const char* msg)
{
	std::cout << "\033[21;32mPASSED\033[0m\t" << msg << std::endl;
}

void failed(const char* msg)
{
	std::cerr << "\033[21;31mFAILED\033[0m\t" << msg << std::endl;
}

void receiver(Communicator* comm)
{
    uint32_t buffer;
    for(uint32_t i = 0; i < LOOP_COUNT; ++i)
    {
        comm->receive(&buffer, sizeof(uint32_t));
        if(buffer != i)
        {
            std::cout << "[RECEIVER] Error, read " << buffer << std::endl;
            failed("");
            exit(-1);
        }
    }
}

void sender(Communicator* comm)
{
    uint32_t buffer;
    for(uint32_t i = 0; i < LOOP_COUNT; ++i)
    {
        buffer = i;
        comm->send(&buffer, sizeof(uint32_t));
    }

    comm->flush();
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    int32_t waitRet;

    std::string branch = "UNKNOWN";
    Communicator* comm;

    try 
    {
        branch = "[Server] ";
        comm = new PosixCommunicatorServer(COMM_SENDER,
                                           COMM_DATA_BLOCK_COUNT,
                                           COMM_DATA_BUFFER_BLOCK_SIZE);

        comm->init();
    }
    catch(nsCommunicator::CommunicatorException& exc)
    {
        std::cout << branch << "Error, caught exception:\n\t" << exc.what() 
                  << std::endl;
        failed("");
        exit(-1);
    }

    if(!fork())
    {
        try 
        {
            branch = "[Client] ";
            comm = new PosixCommunicatorClient(COMM_RECEIVER);

            comm->init();
            comm->connect(1, 5);;

            receiver(comm);

            delete comm;
        }
        catch(nsCommunicator::CommunicatorException& exc)
        {
            std::cout << branch << "Error, caught exception:\n\t" << exc.what() 
                    << std::endl;
            failed("");
            exit(-1);
        }

        exit(0);
    }
    else 
    {
        try 
        {
            comm->connect(1, 5);;

            sender(comm);

            wait(&waitRet);
            if(waitRet != 0)
            {
                exit(waitRet);
            }

            delete comm;
        }
        catch(nsCommunicator::CommunicatorException& exc)
        {
            std::cout << branch << "Error, caught exception:\n\t" << exc.what() 
                    << std::endl;
            failed("");
            exit(-1);
        }
    }

    try 
    {
        branch = "[Server] ";
        comm = new PosixCommunicatorServer(COMM_RECEIVER,
                                           COMM_DATA_BLOCK_COUNT,
                                           COMM_DATA_BUFFER_BLOCK_SIZE);
        comm->init();
    }
    catch(nsCommunicator::CommunicatorException& exc)
    {
        std::cout << branch << "Error, caught exception:\n\t" << exc.what() 
                  << std::endl;
        failed("");
        exit(-1);
    }

    if(!fork())
    {
        try 
        {
            branch = "[Client] ";
            comm = new PosixCommunicatorClient(COMM_SENDER);

            comm->init();
            comm->connect(1, 5);;

            sender(comm);

            delete comm;
        }
        catch(nsCommunicator::CommunicatorException& exc)
        {
            std::cout << branch << "Error, caught exception:\n\t" << exc.what() 
                    << std::endl;
            failed("");
            exit(-1);
        }
        exit(0);
    }
    else 
    {
        try 
        {
            comm->connect(1, 5);;

            receiver(comm);

            wait(&waitRet);
            if(waitRet != 0)
            {
                exit(waitRet);
            }

            delete comm;
        }
        catch(nsCommunicator::CommunicatorException& exc)
        {
            std::cout << branch << "Error, caught exception:\n\t" << exc.what() 
                    << std::endl;
            failed("");
            exit(-1);
        }
    }    

    passed("");

    return 0;
}