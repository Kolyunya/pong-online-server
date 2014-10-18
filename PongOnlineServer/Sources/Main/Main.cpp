#include <Server/Server.hpp>

int main ( int argc , char** argv )
{
    Server server(argc,argv);
    return server.exec();
    return 0;
}
