#define MAX_LISTING_SIZE 10

struct service_listing{
	char ip_address[20];
	char port_number[7];
	char feature[50];
	int request_count;
	int request_probability;
	int local_remote;
};

struct kill_list{
	char ip_address[20];
	char port_number[7];
	int code;
}double_kill[11];

struct req_service_listing{
	int code;
	int number_of_characters;
	long data_position;
	long result;
	char feature[50];
	char data_location[100];
	int booked;
};
struct service_request_list{
	int code;
	int number_of_characters;
	char feature[50];
	char data_location[100];
	char result_location[110];
	
}request_listing[MAX_LISTING_SIZE];
struct hop_ip_list{
	char ip_address[20];
	int port_number;
};
struct ModuleInformation{
	int type;
	uint8_t code;
	int pid;
	int key;
	struct shared_memory_flags{
		int responding_to_request;
		int requesting_send;
		int requesting_recv;
		int request_received;//X
		int service_register;
		int service_kill;
		int service_request;
		int service_response;
		int service_booked;
	}shm_flags;
	int booked;
	int r_port;
	char r_add[0];
	char feature[20];
	char cmd[120];
	char name[80];
	char additional_info[200];
};

struct ModuleAccessInformation{
	int code;
	char  file[150];
	struct ModuleInformation module_info;
}*mod_access_info;

struct universal_shm{
	key_t key;
	int shmid;
	int count;
	int port_number;
	struct service_listing service_list[11];
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

struct service_file_name{
	char received_file_name[100];
	char result_file_name[100];
	long name_position;
	long data_position;
	long ip_position;
};
// struct nodeAccounting{
// 	int code;
// 	//bool type;// type 0 for requesting and type 1 for availability
// 	char* name;
// 	struct sockaddr_in Saddr;
// 	float bandwidth;
// 	float time_estimates[3];

// 	struct feature_available{
// 		char* code_name;
// 		char* description;
// 		int size;
// 		int port_number;
// 	}*availabe_features;
// 	struct feature_requested{
// 		char* code_name;
// 		char* description;
// 		int size;
// 		struct Arrival_Time
// 		{
// 			int hours;
// 			int min;
// 			int sec;
// 		}*arrival_time;
// 	}*requested_features;
// 	//bool flag[8;]
// }*n_accounting;
