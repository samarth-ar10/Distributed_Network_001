
struct ModuleInformation{
	int type;
	uint8_t code;
	int pid;
	int key;
	char* cmd;
	char* name;
	char* additional_info;
}*mod_info;

struct ModuleAccessInformation{
	int code;
	char * file;
	struct ModuleInformation module_info;
}*mod_access_info;


struct universal_shm{
	key_t key;
	int shmid;
	int count;
	int activate_network_module;
	struct ModuleAccessInformation  module_access_info[100];
}*uni_shm;

struct network_variables{
	int _len_;
	int _result_;
	int _sockfd_;
	int _port_address_;
	union client_server{
		struct sockaddr_in _client_address_;
		struct sockaddr_in _server_address_;	
	}client_server_selection;
	char ch[256], buf[256];
}network_variable[2];