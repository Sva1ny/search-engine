#include "SearchEngineServer.h"
#include "Configuration.h"
#include "string"
using std::stoi;
int main()
{
    Configuration::getInstance("../conf/online.conf");
    SearchEngineServer server(stoi(CONFIG[THREAD_NUM]),
                              stoi(CONFIG[QUE_SIZE]),
                              CONFIG[IP],
                              stoi(CONFIG[PORT]));
    server.start();
    return 0;
}