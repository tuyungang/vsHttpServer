#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "TcpEventServer.h"

using namespace std;

int main(int argc, char **argv)
{
    TcpEventServer server(3);
    server.SetPort(12345);

    //TcpEventServer sserver(1);
    server.StartRun();
    return 0;
}
