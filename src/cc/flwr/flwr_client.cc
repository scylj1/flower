/*
 *
 * This project is build on Visual Studio 2019 on Windows with C++ 17
 * There are some differences between Windows and Linux systems, so some code should be changed to work in Linux
 * Additional include directories, additional library directories and additional dependencies are set manually
 * Preprocessor defination are set to avoid some warnings
 * 
 * 
 * Author: Lekang Jiang 14/08/2021
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <queue>
#include <optional>
//#include <windows.h>
#include <map>
#include "transport.grpc.pb.h"
#include "typing.h"
#include "serde.h"
#include "message_handler.h"
#include "start.h"
#include "pytorch_client.h"








int main(int argc, char** argv) {
    
    std::string target_str = "localhost:50051";
    Example_client client;
    start_client(target_str, &client);
    std::cin.get(); //keep the window
    return 0;
}
