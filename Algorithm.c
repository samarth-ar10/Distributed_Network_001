#include "header.h"	
#include "NetworkingModule.h"
/*

*/

enum Interrupt_availability{	Absent	, Present	};

#define __temp_code_	12202
#define __file_code_	30423
/*

*/
enum Interrupt_availability	Check_for_Custom_Input(	char* );

enum Return_code Grab_info_off_the_command( char**);

enum Return_code Grab_info_off_the_user( void );

enum Return_code Child_producer( int, char *, int);

enum Return_code Network_module_request( void );

enum Return_code Activate_network_module_request_checker( void );

/*
	function - main
	purpose  - starting point for the whole application
*/

int main(int argc, char* argv[]){
	
	enum Return_code _temp_return_code_;
	char net_ip_address[20];
	int net_port, avail = 0;

	//-------------------------------------
	enum Interrupt_availability _input_type_check_	=  Check_for_Custom_Input( argv[1] );

	if ( _input_type_check_ == Present){
		_temp_return_code_ = Grab_info_off_the_command(argv);
		sprintf(net_ip_address, "%s", argv[5]);
		net_port = atoi(argv[6]);
		avail = 1;
	}

	else{
		printf("do you have a network?(y/n) ->  ");
		if(getchar() == 'y'){
			printf("enter the ip address -> ");
			scanf("%s", net_ip_address);
			printf("enter the port number -> ");
			scanf("%d", &net_port);
			avail = 1;
		}
		_temp_return_code_ = Grab_info_off_the_user();
	}

	if (_temp_return_code_ != OK ){
		perror("[ERROR][Algorithm]-Grabbing failed--");
		exit(1);
	}

	if ( Child_producer(avail, net_ip_address, net_port) != OK){
		perror("[ERROR][Algorithm]-Child producer error--");	
	}
}

/*
	function - Check_for_Custom_Input
	purpose  - check if the user input is required or it is provided by the execution command
*/


enum Interrupt_availability	Check_for_Custom_Input(	char* _interrupt_string_ ){
	if( atoi(_interrupt_string_) >	1 || atoi(_interrupt_string_) <	0 )
		perror("[ERROR][Algorithm]-Input check failed--");
	else 
		return	atoi(_interrupt_string_);
	exit(1);
}

/*
	function - Grab_info_off_the_command
	purpose  - take parameters from the execution command rather than the user
*/


enum Return_code Grab_info_off_the_command( char** argv){
	
	char _file_name_[100];

	time_t	_rawtime_ ;
	struct tm * __time_info_ ;

	FILE *_log_file_;

	time ( &_rawtime_ );
	__time_info_ = localtime( &_rawtime_ );

	sprintf(_file_name_, "./log/%d_%d_%d_%d_%s_Algo.Log", 1900+__time_info_->tm_year, __time_info_->tm_mon, __time_info_->tm_mday,  __file_code_, argv[2]);

	_log_file_ = fopen( _file_name_, "w+" );

	if( _log_file_ == NULL)
		return FAILED;
	chmod( _file_name_ , 0777);
	
	fprintf( _log_file_ , "loading... at -> %s\n", asctime(__time_info_) );

	fclose( _log_file_ );

	//return ModuleRegister(uni_shm, argv[3] , sizeof(argv[3]) , argv[4] , sizeof(argv[4]) , argv[5] , sizeof( argv[5] ) , _file_name_);	
	return SharedMemoryCreation(argv[2], argv[3], argv[4], _file_name_, __temp_code_);
}

/*
	function - Grab_info_off_the_command
	purpose  - take parameters from the the user instead of execution command 
*/

enum Return_code Grab_info_off_the_user( void){
	char _name_[30], _cmd_[100], _description_[1000], *_file_name_;
	
	time_t	_rawtime_ ;
	struct tm * __time_info_ ;

	FILE *_log_file_;

	printf("\a\tWelcome to Algorithm module - %d\t", __temp_code_);
	
	printf("\nName for the module \t=>\t");
	scanf("%s", _name_);
	
	printf("\nenter the program name to execute the program \t=>\t");
	scanf("%s", _cmd_);

	printf("\nenter a description for the program\t=>\t");
	scanf("%s", _description_);	

	time ( &_rawtime_ );
	__time_info_ = localtime( &_rawtime_ );

	_file_name_ = (char *)malloc(sizeof(_name_)+sizeof(__file_code_));
	sprintf(_file_name_, "./log/%d_%d_%d_%d_%s_Algo.Log", 1900+__time_info_->tm_year, __time_info_->tm_mon, __time_info_->tm_mday,  __file_code_, _name_);

	_log_file_ = fopen(_file_name_, "w+");

	if( _log_file_ == NULL)
		return FAILED;
	//chmod( _file_name_ , 0777);

	fprintf( _log_file_, "loading... at ->%s\n", asctime(__time_info_) );

	fclose( _log_file_ );

	//return ModuleRegister(uni_shm, _name_ , sizeof(_name_) , _description_ , sizeof(_description_) , _cmd_ , sizeof(_cmd_ ), _file_name_);
	return SharedMemoryCreation(_name_, _description_, _cmd_, _file_name_, __temp_code_);
};

/*
	function - Child_producer
	purpose  - it will split the algorithm module form the network module 
*/
enum Return_code Child_producer( int avail, char *net_ip_address, int net_port){
	
	struct ModuleAccessInformation __module_access_information;
	__module_access_information = uni_shm->module_access_info[0];

	printf("command being passed -> \t%s\n", __module_access_information.module_info.cmd );
	printf("file name module[0] -> \t%s \n\n",uni_shm->module_access_info[0].file);

	int _child_ = fork();
	if( _child_ == 0){

		char * cmd = (char *) malloc( sizeof(uni_shm->module_access_info[0].module_info.cmd)+(sizeof(char)*50));
		sprintf(cmd, "xterm -e \"%s %d\"", uni_shm->module_access_info[0].module_info.cmd,(int) uni_shm->module_access_info[0].module_info.key);
		system(cmd);
	}
	else{
		
		while(Activate_network_module_request_checker() != OK);
		Network_module_initialize_outer( (int) check_key(), avail, net_ip_address, net_port );
		while(!Network_module_request());
		
	}
	return OK;
}

/*
	function - Activate_network_module_request_checker
	purpose  - check if the network module is requested by the user
*/

enum Return_code Activate_network_module_request_checker( void ){
	if(uni_shm->activate_network_module == 1)
		return OK;
	else
		return FAILED;
}

/*
	function -	Network_module_request
	purpose  - 	it will confirm the flags requested by the user and peocess the network module as requested 
*/
enum Return_code Network_module_request( void ){
	//printf("%d%d\n", uni_shm->module_access_info[0].module_info.shm_flags.service_request, uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request);
	if(uni_shm->module_access_info[0].module_info.shm_flags.requesting_send == 0 && uni_shm->module_access_info[0].module_info.shm_flags.requesting_recv == 0 && uni_shm->module_access_info[0].module_info.shm_flags.service_register == 0 && uni_shm->module_access_info[0].module_info.shm_flags.service_kill == 0 && uni_shm->module_access_info[0].module_info.shm_flags.service_request == 0 && uni_shm->module_access_info[0].module_info.shm_flags.service_booked == 0)
			uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request = 0;
	else if(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request == 0){  

		if(uni_shm->module_access_info[0].module_info.shm_flags.service_register == 1){
			FILE *msg_ptr = fopen(uni_shm->module_access_info[0].file, "w+");
			if(msg_ptr == NULL)
				printf("dammm...\n");

			fprintf(msg_ptr, "\"rej\"");
			fclose(msg_ptr);
			uni_shm->module_access_info[0].module_info.shm_flags.service_booked = 0;
			Network_module_inner_send("rej");
		}
		else if(uni_shm->module_access_info[0].module_info.shm_flags.service_request == 1){
			Network_module_inner_send("req");
		}
		else if(uni_shm->module_access_info[0].module_info.shm_flags.service_response == 1){
			printf("setting up response\n");
			Network_module_inner_send("res");
			printf("completed\n");
			uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request = 1;
			while(uni_shm->module_access_info[0].module_info.shm_flags.service_response == 1);
			printf("completed\n");
		}
		else if(uni_shm->module_access_info[0].module_info.shm_flags.service_booked == 1){
			printf("service_booked flag is high\n");
		}			
		uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request = 1;
	}
	return OK;
}



