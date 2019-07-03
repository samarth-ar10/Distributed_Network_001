#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <error.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ifaddrs.h>


#include "user_structure"
#include "shm_manager.h"

enum Return_code { OK , FAILED };

void Replicate_Algorithm_Cell(	int _network_module_creation){

}
