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

#include "common_structure.h"
#include "shm_manager.h"


enum Return_code { OK , FAILED };

struct ModuleAccessInformation network_module; 
/*
	functions
		PutPortNumber(int )
					enter the port number for custom sender
		GetP
*/

int PutPortNumber(int portNumber){
}

int GetPortNumber(){
	struct ifaddrs *id;
	int val;	
	val = getifaddrs(&id);
	printf("Network name - %s\n",id->ifa_name );

}

int GetUserPortNumber(){

	int portnumber;

	printf("enter the port number of the destination node \t=>\t");

	scanf("%d", &portnumber);

	return portnumber;
}

/*

*/
int PutIPaddress(){

}

void GetIPaddress(char *ipaddress){
	FILE *temp_pointer = popen("curl -s http://whatismyip.akamai.com/", "r");
	fscanf(temp_pointer, "%s", ipaddress);
	return;
}

char* GetUserIPaddress(){
	char *ipaddress;
	
	ipaddress = (char *)malloc(sizeof(char) * 256);

	printf("\nenter the ip address of the destination node \t=>\t");
	scanf("%s",ipaddress);

	return ipaddress;
}


enum Return_code SharedMemoryCreation(char* name, char* description, char* cmd, char* file_name, int code){
	
	uni_shm = (struct universal_shm*) malloc (sizeof(struct universal_shm)); 

	get_shm_file_path(sizeof(uni_shm),file_name);
	
	uni_shm = (struct universal_shm *) shm_attach();
	
	uni_shm->activate_network_module = 0;
	uni_shm->module_access_info[0].code =  code;
	
	sprintf(uni_shm->module_access_info[0].file, "%s", file_name);
	uni_shm->module_access_info[0].module_info.pid = getpid();
	uni_shm->module_access_info[0].module_info.key = (int)check_key();
	
	sprintf(uni_shm->module_access_info[0].module_info.cmd, "%s", cmd);
	
	sprintf(uni_shm->module_access_info[0].module_info.name, "%s", name);
	
	sprintf(uni_shm->module_access_info[0].module_info.additional_info, "%s", description);

	uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request = 0;
	uni_shm->module_access_info[0].module_info.shm_flags.requesting_send = 0;
	uni_shm->module_access_info[0].module_info.shm_flags.requesting_recv = 0;
	uni_shm->module_access_info[0].module_info.shm_flags.request_received = 0;
	uni_shm->module_access_info[0].module_info.shm_flags.service_register = 0;
	uni_shm->module_access_info[0].module_info.shm_flags.service_kill = 0;
	uni_shm->module_access_info[0].module_info.shm_flags.service_request = 0;
	uni_shm->module_access_info[0].module_info.shm_flags.service_booked =0;
	uni_shm->module_access_info[0].module_info.booked =0;
	int i=0;
	for(	;	i<11	;	i++	){
		uni_shm->service_list[i].ip_address[0] = '\0';
		uni_shm->service_list[i].port_number[0] = '\0';
		uni_shm->service_list[i].feature[0] = '\0';
		uni_shm->service_list[i].request_count = 0;
		uni_shm->service_list[i].request_probability = 0;
		uni_shm->service_list[i].local_remote = 0;
		double_kill[i].ip_address[0] = '\0';
		double_kill[i].port_number[0] = '\0';
		double_kill[i].code = 0;
	}

	uni_shm->count = 1;
	uni_shm->key = (int)check_key();
	uni_shm->shmid = check_shmid();
		
	return OK;
}

