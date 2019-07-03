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

#define __HOP_COUNT_LIMIT 50

enum Return_code { OK , FAILED };

enum entry{ HARD, SOFT };

enum categories { msg, file};

enum Return_code printing_in_file_msg(FILE *_send_file_, char* code, char* ip_address, char *port_number, char *feature, char *msg);

enum Return_code printing_in_file_file(FILE *_send_file_, char* code, char* ip_address, char *port_number, char *feature, char *msg, FILE *ptr);

enum Return_code Activate_shared_memory(int argc, char **argv);

enum Return_code Activate_network_module(void);

enum Return_code Network_module_send(char *msg, enum categories category, char *code, char *ip_address, char *port_number, char *feature, FILE *ptr);

enum Return_code Network_module_recv(void);

enum Return_code Network_module_check(void);

enum Return_code Network_module_request_msg_file();

enum Return_code Network_module_respond_msg_file();

enum Return_code Network_module_create_request_msg(char *feature, FILE *data_ptr, int code);

enum Return_code Network_module_request_service( );

enum Return_code Network_module_register_service(char *feature, struct service_file_name* );

enum Return_code Network_module_serviced_reply( struct service_file_name*);

enum Return_code Network_module_send_service_request( void );

enum Return_code Network_module_request_register( char *feature );

enum Return_code Network_module_respond_to_request( void );

enum Return_code Network_module_receive_service( struct service_file_name* );

enum Return_code Network_module_receive_service_reply();

enum Return_code file_next_word_extract(FILE *, char *);

enum Return_code get_file_name_from_pointer(FILE *search_file, char *filename);

enum Return_code get_file_size(char *filename, int *msg_size);

enum Return_code insert_service_request_list(int code, char *feature, char *file_name);

enum Return_code delete_service_request_list(int code);

enum Return_code find_service_request_list(int code);

enum Return_code get_base_name(char *file_name);

enum Return_code extract_request_listing( struct req_service_listing *,  FILE *, int *, enum entry);


enum Return_code file_next_word_extract(FILE *temp_file, char *buf){
	int temp_var = -1;
	
	while(fgetc(temp_file) != '\"');
	do{
		buf[++temp_var] = fgetc(temp_file);
	}while(buf[temp_var] != '\"');
	buf[temp_var]   = '\0';
	return OK;
}


/*
	function -	Activate_shared_memory
	purpose  - 	give user the access to the shared memory
*/

enum Return_code Activate_shared_memory(int argc, char **argv){
	
	uni_shm = (struct universal_shm*) malloc (sizeof(struct universal_shm)); 

	get_shm_key_value( sizeof(uni_shm), atoi( argv[ argc - 1 ] ));

	uni_shm = (struct universal_shm *) shm_attach();
	uni_shm->count++;

	if(uni_shm == NULL)
		return FAILED;

	return OK;
}

/*
	function -	Activate_network_memory
	purpose  - 	request the algorithm module to start the outer network module
*/

enum Return_code Activate_network_module(void){
	uni_shm->activate_network_module = 1;
	FILE *_send_file_;
	_send_file_ = fopen( uni_shm->module_access_info[0].file, "w+");
	fclose(_send_file_);
	int i =0;
	for(; i<10 ; i++)
		request_listing[i].code = 0;
	return OK;
}

/*
	function -	printing_in_file_msg
	purpose  - 	requesting a message in the file used to club up the data that is requsted by the user to be processed
*/

enum Return_code printing_in_file_msg(FILE *_send_file_, char* code, char* ip_address, char *port_number, char *feature, char *msg){
	char cmd[1000];
	int msg_size;
	sprintf(cmd,"echo  %s | wc -c", msg);
	FILE *temp_pointer = popen(cmd, "r");
	fscanf(temp_pointer, "%d", &msg_size);
	msg_size--;
	fprintf(_send_file_, "\"%s\",\"%s\",\"%d\",\"msg\",\"%d\",\"%s\",\"%s\"\n", code, ip_address, atoi(port_number), msg_size, feature, msg);
	return OK;
}

/*
	function -	printing_in_file_msg
	purpose  - 	requesting a file in the file used to club up the data that is requsted by the user to be processed 
*/
enum Return_code printing_in_file_file(FILE *_send_file_, char* code, char* ip_address, char *port_number, char *feature, char *msg, FILE *ptr){
	FILE *_file_pointer_ = (FILE *)ptr;
	char cmd[1000];
	int file_size;
	
	sprintf(cmd,"wc -c < %s", msg);
	printf("%s\n", cmd);
	FILE *temp_pointer = popen(cmd, "r");
	
	fscanf(temp_pointer, "%d", &file_size);
	
	fprintf(_send_file_, "\"%s\",\"%s\",\"%d\",\"file\",\"%d\",\"%s\",\"%s\"\"", code, ip_address, atoi(port_number), file_size, feature, msg);
	fclose(_send_file_);
	sprintf(cmd,"cat %s >> %s", msg, uni_shm->module_access_info[0].file);
	system(cmd);
	_send_file_ = fopen( uni_shm->module_access_info[0].file, "a");

	// do{
	// 	fprintf(_send_file_, "%c", temp_char);
	// 	temp_char = fgetc(_file_pointer_);
	// }while(temp_char != EOF);
	fprintf(_send_file_, "\"\n");
	return OK;
}

/*
	function -	Network_module_send
	purpose  - 	requesting a message to be sent over the network through the outer networking module given a pre defined ip address
*/
enum Return_code Network_module_send(char *msg, enum categories category, char *code, char *ip_address, char *port_number, char *feature, FILE *ptr){
	
	if(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request == 1 && uni_shm->module_access_info[0].module_info.shm_flags.requesting_send == 1){
		uni_shm->module_access_info[0].module_info.shm_flags.requesting_send = 0;
	}
	FILE *_send_file_;
	
	printf("writing in %s\n",  uni_shm->module_access_info[0].file);
	
	_send_file_ = fopen( uni_shm->module_access_info[0].file, "a");
	if(_send_file_ == NULL){
			perror("[ERROR][USER]-fopen funtion FAILED\n");
		return  FAILED;
	}
	
	FILE *fp;
	char cmd[1000];
	
	switch(category){
		case 0:
				printing_in_file_msg(_send_file_, code, ip_address, port_number, feature, msg);
				break;
		case 1:
				printing_in_file_file(_send_file_, code, ip_address, port_number, feature, msg, ptr);
		default: 
				break;
	}

	fclose(_send_file_);
	
	printf("[UPDATE]waiting for the module to be free\n");
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request !=0 );
	printf("[UPDATE]waiting for the module to complete the task\n");
	uni_shm->module_access_info[0].module_info.shm_flags.requesting_send = 1;
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request == 0 && uni_shm->module_access_info[0].module_info.shm_flags.requesting_send == 1);
	uni_shm->module_access_info[0].module_info.shm_flags.requesting_send = 0;
	printf("[UPDATE] completing the send request \n");
	
	return OK;
}

/*
	function -	Network_module_recv
	purpose  - 	recieve a response form the Inner networking module based on previos send request
*/
enum Return_code Network_module_recv(void){
	if(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request == 1 && uni_shm->module_access_info[0].module_info.shm_flags.requesting_recv == 1){
		uni_shm->module_access_info[0].module_info.shm_flags.requesting_recv = 0;
	}
	FILE *_recv_file_;
	char file_name[100];
	sprintf(file_name, "%s", uni_shm->module_access_info[0].file);
	_recv_file_ = fopen( file_name, "w");
	if(_recv_file_ == NULL){
		perror("[ERROR][USER]-fopen funtion FAILED\n");
		return  FAILED;
	}
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request == 1 && uni_shm->module_access_info[0].module_info.shm_flags.requesting_recv == 0);
	uni_shm->module_access_info[0].module_info.shm_flags.requesting_recv = 1;
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request == 0);
	char c[1000];
	fscanf(_recv_file_,"%[^\n]", c);
	printf("%s\n", c);
	fclose(_recv_file_);
	return OK;
}
/*
	function -	Network_module_check
	purpose  - 	make sure that on returning from this funtion, the Inner Networking Module is processing the request
*/
enum Return_code Network_module_check(void){
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request == 0 );
	return OK;
}
/*
	function -	Network_module_request_msg_file
	purpose  -  user creates a request to initialize the message file that would be sent over the network
*/
enum Return_code Network_module_request_msg_file(){
	FILE *msg_ptr = fopen(uni_shm->module_access_info[0].file, "w+");
	if (msg_ptr == NULL){
		perror("[ERROR][USER][Network_module_request]-fopen funtion FAILED\n");
		return FAILED;
	}
	fprintf(msg_ptr, "\"req\"");
	fclose(msg_ptr);
	return OK;
}

/*
	function -	Network_module_create_request_msg
	purpose  -  user creates a request to initialize a message in the message file that would be sent over the network
*/

enum Return_code Network_module_create_request_msg(char *feature, FILE *data_ptr, int code){
	FILE *msg_ptr = fopen(uni_shm->module_access_info[0].file, "a");
	struct stat buffer;
	char file_name[0xFFF];
	get_file_name_from_pointer(data_ptr, file_name);
	if(stat(file_name, &buffer) == -1){
		perror("[ERROR][USER][Network_module_request]-stat funtion FAILED\n");
		return FAILED;
	}
	char cmd[1000];
	int msg_size;
	sprintf(cmd,"wc -c < %s", file_name);
	FILE *temp_pointer = popen(cmd, "r");
	fscanf(temp_pointer, "%d", &msg_size);
	msg_size--;
	fprintf(msg_ptr, ",\"file\",\"%d\",\"%d\",\"%s\",\"%s\",\"", msg_size, code, feature, file_name);
	fclose(msg_ptr);
	sprintf(cmd,"cat %s >> %s", file_name, uni_shm->module_access_info[0].file);
	system(cmd);
	msg_ptr = fopen( uni_shm->module_access_info[0].file, "a");
	fprintf(msg_ptr, "\"");
	fclose(msg_ptr);
	insert_service_request_list(code, feature, file_name);
	return OK;
}

enum Return_code Network_module_request_service(){
	
	Network_module_send_service_request();
	Network_module_receive_service_reply();
	return OK;
}	

enum Return_code Network_module_register_service(char *feature, struct service_file_name *file_names){
	Network_module_request_register(feature);
	Network_module_receive_service(file_names);// after this servicing will start
	return OK;
}


enum Return_code Network_module_send_service_request( void ){
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request == 1);
	uni_shm->module_access_info[0].module_info.shm_flags.service_request = 1;
	return OK;
}

enum Return_code Network_module_receive_service_reply(){
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request != 1);
	uni_shm->module_access_info[0].module_info.shm_flags.service_request = 0;
	FILE *msg_ptr = fopen(uni_shm->module_access_info[0].file, "r");

	//this is where all the files are going to extracted and then retured to the user with their filename
	struct req_service_listing *temp_list;
	int temp_number_of_requests;
	extract_request_listing(temp_list, msg_ptr, &temp_number_of_requests, HARD);
	while(temp_number_of_requests-- != 0){
		fseek(msg_ptr, 0 , SEEK_SET);
		FILE *temp_file = fopen(temp_list[temp_number_of_requests].data_location, "w+");
		printf("%s\n", temp_list[temp_number_of_requests].data_location);
		while(ftell(msg_ptr) != temp_list[temp_number_of_requests].data_position)fgetc(msg_ptr);
		int i=temp_list[temp_number_of_requests].number_of_characters;
		for( i = 0 ; i <= temp_list[temp_number_of_requests].number_of_characters; i++ ){
			fputc(fgetc(msg_ptr), temp_file);
		}
	}
	printf("reply has been received\n");
	return OK;
}

enum Return_code Network_module_request_register( char *feature ){
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request == 1);
	strcpy(uni_shm->module_access_info[0].module_info.feature, feature);
	uni_shm->module_access_info[0].module_info.shm_flags.service_register = 1;

	return OK;
}

enum Return_code Network_module_receive_service( struct service_file_name  *file_names){
	// at this point the data would be extracted and a send file pointer will also  be created and for the user to 
	// enter the processed data
	// format "type","no_of_ip_address","ipaddress","port","no_of_file_characters","code","name","data"

	FILE *recv_ptr, *send_ptr;
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request != 1);
	uni_shm->module_access_info[0].module_info.shm_flags.service_register = 0;
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request == 1);
	uni_shm->module_access_info[0].module_info.shm_flags.service_booked = 1;
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request != 1);

	int no_of_ip = 0, no_of_file_char = 0, temp_var = 0, code = 0;
	char temp_char, recv_file_name[100];
	
	FILE *temp_pointer = fopen(uni_shm->module_access_info[0].file, "r");

	while(fgetc(temp_pointer) != ',');
	fgetc(temp_pointer);
	while(fgetc(temp_pointer) != ',');
	fgetc(temp_pointer);
	
	temp_char = fgetc(temp_pointer);
	while(temp_char != '"'){
		no_of_file_char = no_of_file_char*10 + atoi(&temp_char);
		temp_char = fgetc(temp_pointer);
	}
	fgetc(temp_pointer);// for ,
	fgetc(temp_pointer);// for "
	temp_char = fgetc(temp_pointer);
	while(temp_char != '"'){
		code = code*10 + atoi(&temp_char);
		temp_char = fgetc(temp_pointer);
	}
	fgetc(temp_pointer);// for ,
	while(fgetc(temp_pointer) != ',');// to skip feature
	fgetc(temp_pointer);// for "
	printf("entered\n");
	file_names->name_position = ftell(temp_pointer);
	do{
		recv_file_name[temp_var++] = fgetc(temp_pointer);
	}while(recv_file_name[temp_var - 1] != '"');
	recv_file_name[temp_var - 1] = '\0';
	get_base_name(recv_file_name);
	strcpy(file_names->received_file_name, recv_file_name);
	fgetc(temp_pointer);
	fgetc(temp_pointer);

	recv_ptr = fopen(recv_file_name, "w+");
	file_names->data_position = ftell(temp_pointer);
	while(no_of_file_char-- != 0){
		fputc( fgetc(temp_pointer), recv_ptr);
	}

	while(fgetc(temp_pointer) != ',');
	file_names->ip_position = ftell(temp_pointer);

	sprintf(recv_file_name, "%s.result", recv_file_name);
	send_ptr = fopen(recv_file_name, "a+");
	strcpy(file_names->result_file_name, recv_file_name);
	
	fclose(recv_ptr);
	fclose(send_ptr);
	fclose(temp_pointer);
	return OK;
}

enum Return_code Network_module_serviced_reply( struct service_file_name * file_names){
	printf("entering the run\n");
	FILE *msg_ptr = fopen(uni_shm->module_access_info[0].file, "r");
	char temp_file_name[50], temp_char, cmd[800];
	strcpy(temp_file_name, uni_shm->module_access_info[0].file);
	sprintf(temp_file_name,"%s.t", temp_file_name);
	FILE *temp_ptr = fopen( temp_file_name, "w+");
	//fprintf(temp_ptr, "\"rep\"");
	while(fgetc(msg_ptr) != ',');
	while(fgetc(msg_ptr) != ',');

	int msg_size;
	sprintf(cmd,"wc -c < %s",  file_names->result_file_name);
	FILE *temp_pointer = popen(cmd, "r");
	fscanf(temp_pointer, "%d", &msg_size);
	msg_size--;
	fclose(temp_pointer);
	fprintf(temp_ptr, "\"rep\",\"file\",\"%d\",", msg_size);


	while(ftell(msg_ptr) != file_names->name_position)
		fputc(fgetc(msg_ptr), temp_ptr);
	fprintf(temp_ptr, "\"%s\",\"", file_names->result_file_name);
	fclose(temp_ptr);
	sprintf(cmd,"cat %s >> %s", file_names->result_file_name, temp_file_name);
	system(cmd);
	//	temp_ptr = fopen( temp_file_name, "a");
	//fprintf(temp_ptr, "\"");
	// fprintf(temp_ptr, "\",\"%s\",\"%d\",\"1\",\"%d\"%c", uni_shm->module_access_info[0].module_info.r_add, uni_shm->module_access_info[0].module_info.r_port, __HOP_COUNT_LIMIT, '\0');
//fprintf(temp_ptr, "\",\"%s\",\"%d\",\"1\",\"%d\"%c", uni_shm->module_access_info[0].module_info.r_add'\0');
	// while(ftell(msg_ptr) != file_names->ip_position)
	// 	fgetc(msg_ptr);
	// temp_char = fgetc(msg_ptr);
	// while(temp_char != EOF){
	// 	fputc(temp_char, temp_ptr);
	// 	temp_char = fgetc(msg_ptr);
	// }

	fclose(temp_ptr);
	fclose(msg_ptr);
	sprintf(cmd,"cat %s > %s && echo processed data && cat %s",temp_file_name, uni_shm->module_access_info[0].file, uni_shm->module_access_info[0].file);
	int waiting_var = system(cmd);
	sprintf(cmd,"rm %s",temp_file_name);
	waiting_var = system(cmd);
	uni_shm->module_access_info[0].module_info.shm_flags.service_booked = 0;
	uni_shm->module_access_info[0].module_info.shm_flags.service_response = 1;
	printf("checking\n");
	while(uni_shm->module_access_info[0].module_info.shm_flags.responding_to_request != 1);
	uni_shm->module_access_info[0].module_info.shm_flags.service_response = 0;

	return OK;
}

enum Return_code get_file_name_from_pointer(FILE *search_file, char *filename){
	int r;
	int MAXSIZE = 0xFFF;
	char proclnk[0xFFF];
	int fno = fileno(search_file);
	sprintf(proclnk, "/proc/self/fd/%d",fno);
	r = readlink(proclnk, filename, MAXSIZE);
	if(r < 0){
		printf("[ERROR][USER][Network_module]-readlink error\n");
		return FAILED;
	}
	filename[r] = '\0';
	return OK;
}

enum Return_code get_file_size(char *filename, int *msg_size){
	char cmd[200];
	sprintf(cmd,"wc -c < %s", filename);
	FILE *temp_pointer = popen(cmd, "r");
	fscanf(temp_pointer, "%d", msg_size);
	*msg_size--;
	return OK;
}

enum Return_code get_base_name(char *file_name){
	char cmd[200];
	sprintf(cmd,"basename %s", file_name);
	FILE *temp_pointer = popen(cmd, "r");
	fscanf(temp_pointer, "%s", file_name);
	return OK;
}

enum Return_code insert_service_request_list(int code, char *feature, char *file_name){
	int i=0;
	while(request_listing[i].code !=0 && i++ < 10);
	if(i == 10){
		return FAILED;
	}
	//no i-- because of the && operator it won't be increased
	request_listing[i].code = code;
	strcpy(request_listing[i].feature,feature);
	strcpy(request_listing[i].data_location,file_name);
	return OK;
}

enum Return_code extract_request_listing( struct req_service_listing *req_listing,  FILE *temp_file, int *number_of_requests, enum entry in_out){
	char buf[256];
	
	while(1){
		file_next_word_extract(temp_file, buf);

		if(strcmp(buf, "file") != 0)
			return OK;
		
		file_next_word_extract(temp_file, buf);
		req_listing[*number_of_requests].number_of_characters = atoi(buf);
		
		file_next_word_extract(temp_file, buf);
		req_listing[*number_of_requests].code = atoi(buf);
		
		file_next_word_extract(temp_file, buf);
		strcpy(req_listing[*number_of_requests].feature, buf);
		
		file_next_word_extract(temp_file, buf);
		strcpy(req_listing[*number_of_requests].data_location, buf);

		if(in_out == HARD){
			fgetc(temp_file);
			fgetc(temp_file);
			req_listing[*number_of_requests].data_position = ftell(temp_file);
			int i = req_listing[*number_of_requests].number_of_characters + 1;
			while(i-- != 0)fgetc(temp_file);
		}fgetc(temp_file);
		req_listing[*number_of_requests].booked = 0;

		*number_of_requests = *number_of_requests + 1;
	}
}

enum Return_code delete_service_request_list(int code){

	return OK;
}

enum Return_code find_service_request_list(int code ){

	return OK;
}

