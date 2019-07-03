#define number_of_connections 2 	
#define __MSG_SIZE 1024 // last 7 should not be used to account for error msg
#define __HOP_COUNT_LIMIT 50

enum entry{ HARD, SOFT };

enum Return_code Send_data( char*, char*, int );

enum Return_code Receive_data( char* );

enum Return_code Network_module_initialize_outer( int , int , char *, int 	 );

enum Return_code Network_module_manager_outer( int , int , char *, char * );

enum Return_code Network_module_inner_send( char *);

enum Return_code Request_service( char* _category_, char* _client_address_, int port);

enum Return_code Register_service( void );

enum Return_code connect_to_service( void );

enum Return_code msg_word_extract (char *, char *, int *);

enum Return_code insert_service_listing(char *, int, char *, enum entry );

enum Return_code file_next_word_extract(FILE *, char *);

enum Return_code respond_req_categories(char *, char *, char *, int *, int);

enum Return_code find_lowest_pos(int *, int);

enum Return_code compare_forward_requesting_listing_with_available( char *, FILE *, struct hop_ip_list *, int hop_count, int max_hop_count, struct req_service_listing *, int ,struct network_variables *, struct network_variables *);

enum Return_code extract_request_listing( struct req_service_listing *, FILE *, int *, enum entry);

enum Return_code extract_hop_list(FILE *, struct hop_ip_list *, int );

enum Return_code hop_count_check(FILE *, int *, int *);

enum Return_code no_of_characters_file(char *file_name, int *msg_size);

enum Return_code file_send(struct network_variables *_client_network_variable_ , FILE *msg_ptr, int size);

enum Return_code file_recv(struct network_variables *_server_network_variable_ , struct network_variables *_client_network_variable_, FILE *msg_ptr);

enum Return_code add_request_to_file(FILE *msg_ptr, FILE *temp_file, struct req_service_listing req_listing, enum entry con);

enum Return_code kill_packet_forward(struct network_variables *, struct network_variables *, FILE *, char *, char *, int, enum entry);

enum Return_code packet_forward(struct network_variables *, struct network_variables *, FILE *, char *,struct hop_ip_list *, int , enum entry , enum entry );

enum Return_code packet_retrace_forward( char * file_name_temp, FILE *temp_file, struct hop_ip_list *hop_list, int hop_count, int max_hop_count,struct network_variables *server, struct network_variables *client);

enum Return_code packet_retrace_backward( char * file_name_temp, FILE *temp_file, struct hop_ip_list *hop_list, int hop_count, int max_hop_count,struct network_variables *server, struct network_variables *client);


enum Return_code Network_module_initialize_outer( int key, int avail, char *ori_ip_address, int ori_port ){
	pid_t child = fork();
	char ori_port_temp[20];
	if(child < 0)
		return FAILED;
	else if( child != 0)
		return OK;
	sprintf(ori_port_temp, "%d", ori_port);
	Network_module_manager_outer( key ,avail, ori_ip_address, ori_port_temp);
	exit(1);
}

enum Return_code Network_module_manager_outer( int key, int avail, char *ori_ip_address, char *ori_port){

	struct network_variables Outer_network_module_server, Outer_network_module_client;
	int  error, temp_var;
	char log_file_name[160], temp_ip_address[20];// extra 10 for the outer manager
	FILE *log_file;
	GetIPaddress(temp_ip_address);
	Outer_network_module_server._sockfd_ = socket(AF_INET, SOCK_STREAM, 0 );

	Outer_network_module_server.client_server_selection._server_address_.sin_family = AF_INET;
	Outer_network_module_server.client_server_selection._server_address_.sin_addr.s_addr = inet_addr(temp_ip_address);
	Outer_network_module_server.client_server_selection._server_address_.sin_port = 0;
	Outer_network_module_server._len_= sizeof(Outer_network_module_server.client_server_selection._server_address_);

	memcpy(&Outer_network_module_client.client_server_selection._client_address_, &Outer_network_module_server.client_server_selection._server_address_, Outer_network_module_server._len_);
	uni_shm = (struct universal_shm*) malloc (sizeof(struct universal_shm)); 
	get_shm_key_value( sizeof(uni_shm), key);
	uni_shm = (struct universal_shm *) shm_attach();

	bind( Outer_network_module_server._sockfd_, (struct sockaddr *)&Outer_network_module_server.client_server_selection._server_address_, Outer_network_module_server._len_ );
	listen( Outer_network_module_server._sockfd_, 0);

	if ( 0 > getsockname(Outer_network_module_server._sockfd_, (struct sockaddr *)&Outer_network_module_server.client_server_selection._server_address_, &Outer_network_module_server._len_ )){
		perror("[ERROR][Network][Outer]-getsockname error--");
		return FAILED;
	}
	printf("bind socket port [OUTER] -> %d\n",ntohs(Outer_network_module_server.client_server_selection._server_address_.sin_port));
	uni_shm->port_number = ntohs(Outer_network_module_server.client_server_selection._server_address_.sin_port);

	sprintf(log_file_name, "%s.OuterLog", uni_shm->module_access_info[0].file);

	log_file = fopen(log_file_name, "w+");
	time_t	_rawtime_ ;
	struct tm * __time_initial_info_, *__time_info_;
	time ( &_rawtime_ );
	__time_initial_info_ = localtime( &_rawtime_ );
	
	fprintf( log_file, "loading... at -> %s\ncurrent time -> 0\n", asctime(__time_initial_info_) );

	if(avail == 1)
		insert_service_listing(ori_ip_address, atoi(ori_port), "default", SOFT);

	while(1){
		
		Outer_network_module_server.client_server_selection._server_address_.sin_port = htons(uni_shm->port_number);
		bind( Outer_network_module_server._sockfd_, (struct sockaddr *)&Outer_network_module_server.client_server_selection._server_address_, Outer_network_module_server._len_ );
		listen( Outer_network_module_server._sockfd_, 0);
		fseek(log_file, 0, SEEK_SET);
		__time_info_ = localtime( &_rawtime_ );	
		fprintf( log_file, "loading... at -> %s\ncurrent time -> %s\n", asctime(__time_initial_info_), asctime(__time_info_) );
		printf("%s\n", asctime(__time_info_));
		fprintf( log_file, " ip_address - %s\nport number - %d\n", temp_ip_address, uni_shm->port_number);

		char msg[__MSG_SIZE], test_msg[__MSG_SIZE], buf[64], resp_msg[__MSG_SIZE], echo_msg[__MSG_SIZE], file_name_temp[200];
		char temp_ip_address[20];
		int temp_port_number;

		sprintf(file_name_temp, "%s.outer.temp",uni_shm->module_access_info[0].file);
		FILE *temp_file = fopen(file_name_temp, "w+");
		if(temp_file == NULL){
			printf ("YOU ARE done for");
		}
		printf("[OUTER] waiting to recv\n");
		int word_position = 0;
		file_recv( &Outer_network_module_server, &Outer_network_module_client, temp_file);
		fseek(temp_file, 0, SEEK_SET);

		file_next_word_extract(temp_file, buf);	
		printf("[OUTER]receive -> %s\n", buf);
		printf("receied -> %s\n", buf);
		if( strcmp(buf, "req") == 0){
			getchar();
			getchar();
			getchar();
			getchar();
			printf("[OUTER]received req \n");//--------------------------
			int number_of_requests = 0, max_hop_count, hop_count;
			struct hop_ip_list *hop_list;
			struct req_service_listing req_listing[MAX_LISTING_SIZE];
			
			extract_request_listing(req_listing, temp_file, &number_of_requests, SOFT);
			hop_count_check(temp_file, &hop_count, &max_hop_count);
			if(hop_count == max_hop_count) continue;
			printf("%d  ~= %d\n", hop_count, max_hop_count);
			hop_list = (struct hop_ip_list *)malloc(hop_count * sizeof(struct hop_ip_list));
			extract_hop_list(temp_file, hop_list, hop_count);
			fclose(temp_file);
			
			for(int i =0 ; i < hop_count; i++){
				printf("%d --- %s\n", hop_list[i].port_number, hop_list[i].ip_address);
			}

			printf("O forwarding req \n");//--------------------------
			temp_file = fopen(file_name_temp, "w+");
			printf("\"req\"");
			compare_forward_requesting_listing_with_available(file_name_temp, temp_file , hop_list, hop_count, max_hop_count, req_listing, number_of_requests, &Outer_network_module_server, &Outer_network_module_client);
			printf("O forwarded req \n");//--------------------------
		}
		else if(strcmp(buf, "boo") == 0 ){
			printf("[OUTER]received boo\n");
			getchar();
			getchar();
			getchar();
			getchar();
			int hop_count, max_hop_count, i;
			char send_file[350], temp_char;
			struct hop_ip_list *hop_list;
		
			hop_count_check(temp_file, &hop_count, &max_hop_count);						
			if(hop_count == max_hop_count){
				return OK;
			}
			hop_list = (struct hop_ip_list *)malloc(hop_count * sizeof(struct hop_ip_list));
			extract_hop_list(temp_file, hop_list, hop_count);
			for(int i =0 ; i < hop_count; i++){
				printf("%d --- %s\n", hop_list[i].port_number, hop_list[i].ip_address);
			}
			fseek(temp_file, 0 , SEEK_SET);
			packet_retrace_forward(file_name_temp, temp_file, hop_list, hop_count, max_hop_count,&Outer_network_module_server, &Outer_network_module_client);

		}
		else if(strcmp(buf, "hoo") == 0){
			getchar();
			getchar();
			getchar();
			getchar();
			printf("[OUTER]received hoo\n");
			int hop_count, max_hop_count, i;
			char send_file[350], temp_char;
			struct hop_ip_list *hop_list;
		
			hop_count_check(temp_file, &hop_count, &max_hop_count);						
			if(hop_count == max_hop_count){
				return OK;
			}
			hop_list = (struct hop_ip_list *)malloc(hop_count * sizeof(struct hop_ip_list));
			extract_hop_list(temp_file, hop_list, hop_count);
			fseek(temp_file, 0 , SEEK_SET);
			packet_retrace_backward(file_name_temp, temp_file, hop_list, hop_count, max_hop_count,&Outer_network_module_server, &Outer_network_module_client);
		}
		// else if(strcmp (buf, "rep") == 0){
		 	
		// }
		else if(strcmp (buf, "res") == 0){
			getchar();
			getchar();
			getchar();
			getchar();
			printf("[OUTER]received res\n");
			int hop_count, max_hop_count, i;
			char send_file[350], temp_char;
			struct hop_ip_list *hop_list;
		
			hop_count_check(temp_file, &hop_count, &max_hop_count);						
			if(hop_count == max_hop_count){
				return OK;
			}
			hop_list = (struct hop_ip_list *)malloc(hop_count * sizeof(struct hop_ip_list));
			extract_hop_list(temp_file, hop_list, hop_count);
			fseek(temp_file, 0 , SEEK_SET);
			printf("forwarding res\n");
			
			for(i =0 ; i<hop_count ; i++){
				printf("%d -> %s - %d\n", i, hop_list[i].ip_address, hop_list[i].port_number);
			}
			packet_retrace_backward(file_name_temp, temp_file, hop_list, hop_count, max_hop_count,&Outer_network_module_server, &Outer_network_module_client);
			printf("got skip_backward\n");
		}
		else if(strcmp (buf, "rej") == 0){
			getchar();
			getchar();
			getchar();
			getchar();
			printf("[OUTER]received rej\n");

			file_next_word_extract(temp_file, buf);
			printf("[OUTER][REJ]ip address -> %s\n", buf);
			strcpy(temp_ip_address, buf);

			file_next_word_extract(temp_file, buf);
			printf("[OUTER][REJ]port -> %s\n", buf);
			temp_port_number = atoi(buf);
			printf("testing here -------- %d\n", temp_port_number);

			insert_service_listing(temp_ip_address, temp_port_number, uni_shm->module_access_info[0].module_info.feature, HARD);

			printf("returned\n");
			
		}
		else if(strcmp (buf, "kill") == 0){
			getchar();
			getchar();
			getchar();
			getchar();
			printf("[OUTER]received boo\n");
			int i=0;
			for (i = 1; i< 11; i++){
				double_kill[i].code = double_kill[i-1].code;			
				strcpy(double_kill[i].ip_address, double_kill[i-1].ip_address);
				strcpy(double_kill[i].port_number, double_kill[i-1].port_number);
			}

			file_next_word_extract(temp_file, buf);
			strcpy(double_kill[0].ip_address, buf);

			file_next_word_extract(temp_file, buf);
			strcpy(double_kill[0].ip_address, buf);

			char code_temp[7];
			file_next_word_extract(temp_file, code_temp);
			double_kill[0].code = atoi(code_temp);
			GetIPaddress(temp_ip_address);
			kill_packet_forward(&Outer_network_module_server, &Outer_network_module_client,temp_file, file_name_temp, temp_ip_address, uni_shm->port_number, HARD);
		}
		// else if(strcmp(buf, "recv") == 0){
		// 	lookup_forward_request(msg, &word_position, Outer_network_module_server, Outer_network_module_client);
		// }
		// else if(strcmp(buf, "send") == 0){
		// }

		else if(strcmp(buf, "error") == 0){
			printf("[OUTER]received error\n");
		}
		else {
			// sprintf(msg, "\"error\"%s", msg);
			// respond_as_error(msg, resp_msg);
			// write(Outer_network_module_client._sockfd_, resp_msg, sizeof(resp_msg));
		}
		fclose(temp_file);

		for(int i =0 ; i<11; i++){
			fprintf( log_file ,"%s\t\t-\t\t%s\t\t-\t\t%s\t\t-\t\t%d\t\t-\t\t%d\n", uni_shm->service_list[i].ip_address, uni_shm->service_list[i].port_number, uni_shm->service_list[i].feature, uni_shm->service_list[i].request_count, uni_shm->service_list[i].local_remote);
		}
		fprintf(log_file, "\n\n");
		// char cmd[200];
		// sprintf(cmd, "gedit %s",file_name_temp);
		// printf("command %s\n", cmd);
		// system(cmd);

	}
	fclose(log_file);

	return OK;
}

inline enum Return_code Send_data( char* _data_, char *_client_address_, int _port_){	
	struct network_variables *_client_network_variable_ = &network_variable[1];

	_client_network_variable_->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if( _client_network_variable_->_sockfd_ < 0){
		perror("[ERROR][NETWORK][INNER]-socket function error");
		return FAILED;
	}

	_client_network_variable_->client_server_selection._client_address_.sin_family = AF_INET;
	_client_network_variable_->client_server_selection._client_address_.sin_addr.s_addr = inet_addr(_client_address_);
	_client_network_variable_->client_server_selection._client_address_.sin_port = htons(_port_);
	_client_network_variable_->_len_ = sizeof (_client_network_variable_->client_server_selection._client_address_);

	// int len = sizeof(_client_network_variable_->client_server_selection._client_address_);

	int result = connect(_client_network_variable_->_sockfd_,(struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_,_client_network_variable_->_len_);
	if(result==-1){
		perror("Client Error\n");
		return FAILED;
	}
	char buf[256];
	write(_client_network_variable_->_sockfd_, _data_, sizeof(_data_));
	while(read(_client_network_variable_->_sockfd_, buf, sizeof(buf)));
	printf("%s\n", buf);

	return OK;
}	
inline enum Return_code Send_file( char* _data_, char *_client_address_, int _port_){	
	struct network_variables *_client_network_variable_ = &network_variable[1];

	_client_network_variable_->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if( _client_network_variable_->_sockfd_ < 0){
		perror("[ERROR][NETWORK][INNER]-socket function error");
		return FAILED;
	}

	_client_network_variable_->client_server_selection._client_address_.sin_family = AF_INET;
	_client_network_variable_->client_server_selection._client_address_.sin_addr.s_addr = inet_addr(_client_address_);
	_client_network_variable_->client_server_selection._client_address_.sin_port = htons(_port_);
	_client_network_variable_->_len_ = sizeof (_client_network_variable_->client_server_selection._client_address_);

	// int len = sizeof(_client_network_variable_->client_server_selection._client_address_);

	int result = connect(_client_network_variable_->_sockfd_,(struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_,_client_network_variable_->_len_);
	if(result==-1){
		perror("Client Error\n");
		return FAILED;
	}
	char buf[256];
	write(_client_network_variable_->_sockfd_, _data_, sizeof(_data_));
	while(read(_client_network_variable_->_sockfd_, buf, sizeof(buf)));
	printf("%s\n", buf);

	return OK;
}	

enum Return_code Receive_data( char* _data_){
	struct network_variables *_server_network_variable_ = (struct network_variables *)malloc(sizeof(struct network_variables));
	struct network_variables *_client_network_variable_ = (struct network_variables *)malloc(sizeof(struct network_variables));
	
	_server_network_variable_->_sockfd_ = socket( AF_INET, SOCK_STREAM, 0 );
	if( _server_network_variable_->_sockfd_ < 0 ){
		perror("[ERROR][NETWORK][INNER]-socket function error");
		return FAILED;
	}

	_server_network_variable_->client_server_selection._server_address_.sin_family = AF_INET;
	_server_network_variable_->client_server_selection._server_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
	_server_network_variable_->client_server_selection._server_address_.sin_port = 0;
	_server_network_variable_->_len_ = sizeof (_server_network_variable_->client_server_selection._server_address_);

	if( 0 > bind(_server_network_variable_->_sockfd_, (struct sockaddr *)&_server_network_variable_->client_server_selection._server_address_, _server_network_variable_->_len_)){
		perror("[ERROR][NETWORK][INNER]-bind function error");
		return FAILED;
	}
	if( 0 > listen(_server_network_variable_->_sockfd_, 0)){
		perror("[ERROR][NETWORK][INNER]-bind function error");
		return FAILED;
	}
	getsockname(_server_network_variable_->_sockfd_,(struct sockaddr *) &_server_network_variable_->client_server_selection._server_address_,  &_server_network_variable_->_len_);
	printf("bind socket port [INNER] -> %d\n",ntohs( _server_network_variable_->client_server_selection._server_address_.sin_port ));

	_client_network_variable_->_sockfd_ = accept(_server_network_variable_->_sockfd_, (struct sockaddr *) &_client_network_variable_->client_server_selection._client_address_, &_client_network_variable_->_len_);

	if (_client_network_variable_->_sockfd_ < 0 )
	{
		perror("[ERROR][NETWORK][INNER]-accept function error");
		return FAILED;
	}		

	if( 0 > read(_client_network_variable_->_sockfd_, _data_, sizeof(_data_))){
		perror("[ERROR][NETWORK][INNER]-recv function error");
		return FAILED;
	}
		printf("%s -received\n", _data_ );
		
	//}	
	return OK;
}
enum Return_code msg_word_extract (char *msg, char *buf, int *count){
	char temp_char = ' ';
	int seperator_count = 0, temp_var1 = 0, temp_var2 = 0;
	while(seperator_count != *count && temp_char != '\0'){
		temp_char = msg[temp_var1++];
		if(temp_char == ',')
			seperator_count++;
	}
	if(temp_char == '\0')
		return FAILED;

	temp_char = msg[temp_var1++];
	temp_char = msg[temp_var1++];
	
	while(temp_char != '"' && temp_char != '\0'){
		buf[temp_var2++] = temp_char;
		temp_char = msg[temp_var1++];
	}
	if(temp_char == '\0')
		return FAILED;
	buf[temp_var2] = '\0';
	return OK;
}

enum Return_code file_next_word_extract(FILE *temp_file, char *buf){
	int temp_var = -1;
	
	while(fgetc(temp_file) != '\"');
	do{
		buf[++temp_var] = fgetc(temp_file);
	}while(buf[temp_var] != '\"');
	buf[temp_var]   = '\0';
	return OK;
}

enum Return_code insert_service_listing(char *temp_ip_address, int temp_port_number, char *feature, enum entry insert_flag){
	int lowest_pos;

	if(insert_flag == HARD){
		find_lowest_pos(&lowest_pos, 9);
		
		sprintf(uni_shm->service_list[lowest_pos].ip_address, "%s", temp_ip_address);
		sprintf(uni_shm->service_list[lowest_pos].port_number, "%d", temp_port_number);
		sprintf(uni_shm->service_list[lowest_pos].feature, "%s", feature);
		uni_shm->service_list[lowest_pos].request_count = 0;
		uni_shm->service_list[lowest_pos].request_probability = (float)1;
		uni_shm->service_list[lowest_pos].local_remote = 1;
		
		return OK;
	}
	
	find_lowest_pos(&lowest_pos, 10);
	if(lowest_pos != 10 && uni_shm->service_list[lowest_pos].request_count !=0){
		sprintf(uni_shm->service_list[lowest_pos].ip_address, "%s", uni_shm->service_list[10].ip_address);
		sprintf(uni_shm->service_list[lowest_pos].port_number, "%s", uni_shm->service_list[10].port_number);
		sprintf(uni_shm->service_list[lowest_pos].feature, "%s", uni_shm->service_list[10].feature);
		uni_shm->service_list[lowest_pos].request_count = uni_shm->service_list[10].request_count;
		uni_shm->service_list[lowest_pos].request_probability = uni_shm->service_list[10].request_probability;
		uni_shm->service_list[lowest_pos].local_remote = uni_shm->service_list[10].local_remote;

		lowest_pos = 10;
		uni_shm->service_list[lowest_pos].request_count = 0;
	}
	//printf("%d ----- %s\n", temp_port_number, temp_ip_address);
	sprintf(uni_shm->service_list[lowest_pos].ip_address, "%s", temp_ip_address);
	sprintf(uni_shm->service_list[lowest_pos].port_number, "%d", temp_port_number);
	sprintf(uni_shm->service_list[lowest_pos].feature, "%s", feature);
	uni_shm->service_list[lowest_pos].request_count++ ;
	uni_shm->service_list[lowest_pos].request_probability = (float)0;
	uni_shm->service_list[lowest_pos].local_remote = 0;
	return OK;
}

enum Return_code find_lowest_pos(int *lowest_pos, int case_numbers){
	*lowest_pos = case_numbers;
	
	for(int i = 0 ; i < case_numbers ; i++){
		if(uni_shm->service_list[i].request_count == 0 && uni_shm->service_list[i].local_remote == 0){
			*lowest_pos = i;
			return OK;
		}
		if(uni_shm->service_list[i].request_count < uni_shm->service_list[i+1].request_count && uni_shm->service_list[i].local_remote == 0)
			*lowest_pos = i;
		else 
			*lowest_pos = i+1;
	}
	
	return OK;
}

enum Return_code Network_module_inner_send(char req[6]){
	struct network_variables *_client_network_variable_ = &network_variable[1];
	struct network_variables *_server_network_variable_ = &network_variable[0];
	int port_assigned;
	int opt = 1;
	_client_network_variable_->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if( _client_network_variable_->_sockfd_ < 0){
		perror("[ERROR][NETWORK][INNER]-socket function error [1] ");
		return FAILED;
	}
	if (setsockopt(_client_network_variable_->_sockfd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 

	_client_network_variable_->client_server_selection._client_address_.sin_family = AF_INET;
	_client_network_variable_->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
	_client_network_variable_->client_server_selection._client_address_.sin_port = htons(uni_shm->port_number);
	_client_network_variable_->_len_ = sizeof (_client_network_variable_->client_server_selection._client_address_);


	_server_network_variable_->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if( _client_network_variable_->_sockfd_ < 0){
		perror("[ERROR][NETWORK][INNER]-socket function error  [0]");
		return FAILED;
	}

	_server_network_variable_->client_server_selection._server_address_.sin_family = AF_INET;
	_server_network_variable_->client_server_selection._server_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
	_server_network_variable_->client_server_selection._server_address_.sin_port = 0;
	_server_network_variable_->_len_ = sizeof (_server_network_variable_->client_server_selection._server_address_);

	bind( _server_network_variable_->_sockfd_, (struct sockaddr *)&_server_network_variable_->client_server_selection._server_address_, _server_network_variable_->_len_ );	

	// int len = sizeof(_client_network_variable_->client_server_selection._client_address_);
	int result = connect(_client_network_variable_->_sockfd_,(struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_,_client_network_variable_->_len_);
	
	if(result==-1){
		perror("Client Error\n");
		return FAILED;
	}

	if ( 0 > getsockname(_client_network_variable_->_sockfd_, (struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_, &_client_network_variable_->_len_)){
		perror("[ERROR][Network][Inner]-getsockname error-- [1]");
		return FAILED;
	}

	if ( 0 > getsockname(_server_network_variable_->_sockfd_, (struct sockaddr *)&_server_network_variable_->client_server_selection._server_address_, &_server_network_variable_->_len_)){
		perror("[ERROR][Network][Inner]-getsockname error-- [0]");
		return FAILED;
	}
	port_assigned = ntohs(_server_network_variable_->client_server_selection._server_address_.sin_port);

	char ipaddress[30], file_name_temp[200], file_name_temp1[200];
	int  msg_size, temp_var;
	
	GetIPaddress(ipaddress);
	sprintf(file_name_temp, "%s.inner.temp", uni_shm->module_access_info[0].file);

	FILE *temp_file, *temp_file1;
	FILE *msg_ptr = fopen(uni_shm->module_access_info[0].file, "a");
	if(msg_ptr == NULL)
		printf("dammm...\n");

	fprintf(msg_ptr, ",\"%s\",\"%d\",\"1\",\"%d\"", ipaddress, port_assigned, __HOP_COUNT_LIMIT);
	fclose(msg_ptr);

	if(strcmp(req, "req") == 0){
		
		int number_of_requests = 0;
		struct req_service_listing req_listing[MAX_LISTING_SIZE];
		msg_ptr = fopen(uni_shm->module_access_info[0].file, "r");
	
		while(fgetc(msg_ptr) != ',');
		extract_request_listing(req_listing, msg_ptr, &number_of_requests, HARD);
		fclose(msg_ptr);

		msg_ptr = fopen(uni_shm->module_access_info[0].file, "r");
		temp_file = fopen(file_name_temp, "w+");
		if(temp_file == NULL)
			printf("you are done\n");
		
		fprintf(temp_file, "\"req\"");
		for(temp_var = 0; temp_var < number_of_requests ; temp_var++)
			add_request_to_file(msg_ptr, temp_file, req_listing[temp_var], SOFT);
		fprintf(temp_file, ",\"%s\",\"%d\",\"1\",\"%d\"", ipaddress, ntohs(_server_network_variable_->client_server_selection._server_address_.sin_port), __HOP_COUNT_LIMIT);
		fputc('\0',temp_file);

		fclose(temp_file);
		fclose(msg_ptr);
	
		no_of_characters_file(file_name_temp, &msg_size);
	
		temp_file = fopen(file_name_temp, "r");
		file_send(_client_network_variable_, temp_file, msg_size);
		printf("sent req\n");//------------------------------------------
		fclose(temp_file);
		int condition = 0;

		sprintf(file_name_temp1, "%s.inner.temp1", uni_shm->module_access_info[0].file);
		temp_file1 = fopen(file_name_temp1, "w+");
		fprintf(temp_file1, "\"res\"");
		fclose(temp_file1);
		msg_ptr = fopen(uni_shm->module_access_info[0].file, "r");
		while(condition < number_of_requests  ){
			fseek(msg_ptr, 0, SEEK_SET);
			char buf[256], int_count[7];
			temp_file = fopen(file_name_temp, "w+");
			file_recv(_server_network_variable_, _client_network_variable_, temp_file);
			fclose(temp_file);
			_server_network_variable_->client_server_selection._server_address_.sin_port = htons(port_assigned);
			bind( _server_network_variable_->_sockfd_, (struct sockaddr *)&_server_network_variable_->client_server_selection._server_address_, _server_network_variable_->_len_ );	

			temp_file = fopen(file_name_temp,"r");
			file_next_word_extract(temp_file, buf);
			printf("[INNER] received ->%s\n", buf);
			if( strcmp(buf, "req") == 0) continue;
			else if(strcmp(buf, "kill") == 0) continue;
			else if(strcmp(buf, "rej") == 0) continue;
			else if(strcmp(buf, "res") == 0){
				printf("entered res -res \n");
				struct req_service_listing temp_list[MAX_LISTING_SIZE];
				struct hop_ip_list *hop_list;
				int temp_number_of_requests,hop_count, max_hop_count;

				extract_request_listing(temp_list, temp_file, &temp_number_of_requests, SOFT);
				
				hop_count_check(temp_file, &hop_count, &max_hop_count);
				hop_list = (struct hop_ip_list *)malloc(hop_count * sizeof(struct hop_ip_list));
				printf("checkpoint\n");
				extract_hop_list(temp_file, hop_list, hop_count);
				printf("checkpoint %d - %d\n", temp_number_of_requests, temp_var);
				temp_var = 0;
				fclose(temp_file);
				while(temp_var < temp_number_of_requests){
					printf("%d - %d   -- %s - %s\n",temp_list[0].code ,  req_listing[temp_var].code, temp_list[0].feature, req_listing[temp_var].feature);
					if(temp_list[0].code == req_listing[temp_var].code && strcmp(temp_list[0].feature, req_listing[temp_var].feature) == 0){
						if(strcmp(temp_list[0].data_location, req_listing[temp_var].data_location) == 0 && req_listing[temp_var].booked == 0){
							int i;
							
							printf("entered res -res %d\n", hop_count);							
							
						
							temp_file = fopen(file_name_temp, "w+");
							fprintf(temp_file,"\"boo\"");
							add_request_to_file(msg_ptr, temp_file, req_listing[temp_var], SOFT);
							for( i = hop_count -1; i >-1; i--)
								fprintf(temp_file, ",\"%s\",\"%d\"", hop_list[i].ip_address, hop_list[i].port_number);
							fprintf(temp_file, ",\"%d\",\"%d\"", hop_count, max_hop_count);
							fclose(temp_file);

							no_of_characters_file(file_name_temp, &msg_size);
							temp_file = fopen(file_name_temp, "r");
							_client_network_variable_->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
							_client_network_variable_->client_server_selection._client_address_.sin_family = AF_INET;
							_client_network_variable_->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
							_client_network_variable_->client_server_selection._client_address_.sin_port = htons(uni_shm->port_number);
							_client_network_variable_->_len_ = sizeof (_client_network_variable_->client_server_selection._client_address_);
							result = connect(_client_network_variable_->_sockfd_,(struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_,_client_network_variable_->_len_);
							if(result==-1){
								perror("Client Error\n");
								return FAILED;
							}
							printf("sending boo\n");
							file_send(_client_network_variable_, temp_file, msg_size);
							fclose(temp_file);
							req_listing[temp_var].booked = 1;
						}break;
					}temp_var++;
				}
			}
			else if(strcmp(buf, "hoo") == 0){
				printf("[INNER]received hoo\n" );
				struct req_service_listing temp_list[MAX_LISTING_SIZE];
				int temp_number_of_requests, hop_count, max_hop_count;
				struct hop_ip_list *hop_list;
				extract_request_listing(temp_list, temp_file, &temp_number_of_requests, SOFT);
				hop_count_check(temp_file, &hop_count, &max_hop_count);	
				hop_list = (struct hop_ip_list *)malloc(hop_count*sizeof(struct hop_ip_list));
				extract_hop_list(temp_file, hop_list, hop_count);
				fclose(temp_file);
				printf("check hoo here\n");
				getchar();
				getchar();
				getchar();
				getchar();

				temp_var = 0;
				temp_file = fopen(file_name_temp, "w+");
				fprintf(temp_file,"\"rep\"");
				while(temp_var < temp_number_of_requests){
					if(temp_list[1].code == req_listing[temp_var].code && strcmp(temp_list[1].feature, req_listing[temp_var].feature) == 0){
						add_request_to_file(msg_ptr, temp_file, req_listing[temp_var], HARD);		
						break;
					} temp_var++;
				}
				fprintf(temp_file, ",\"%s\",\"%d\"", ipaddress, port_assigned);
				fprintf(temp_file, ",\"%d\",\"%d\"", 1, __HOP_COUNT_LIMIT);
				printf("sent hoooooooo %d\n", port_assigned);
				fclose(temp_file);
				_client_network_variable_->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
				_client_network_variable_->client_server_selection._client_address_.sin_family = AF_INET;
				_client_network_variable_->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
				_client_network_variable_->client_server_selection._client_address_.sin_port = htons(hop_list[0].port_number);
				_client_network_variable_->_len_ = sizeof (_client_network_variable_->client_server_selection._client_address_);
				printf("sending rep %s - %d, %s - %d\n", hop_list[0].ip_address, hop_list[0].port_number, ipaddress, port_assigned);
				result = connect(_client_network_variable_->_sockfd_,(struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_,_client_network_variable_->_len_);
				if(result==-1){
					perror("Client Error\n");
					return FAILED;
				}
				no_of_characters_file(file_name_temp, &msg_size);
				temp_file = fopen(file_name_temp, "r");
				file_send(_client_network_variable_, temp_file, msg_size);
				fclose(temp_file);

				_client_network_variable_->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
				_client_network_variable_->client_server_selection._client_address_.sin_family = AF_INET;
				_client_network_variable_->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
				_client_network_variable_->client_server_selection._client_address_.sin_port = htons(uni_shm->port_number);
				_client_network_variable_->_len_ = sizeof (_client_network_variable_->client_server_selection._client_address_);
				result = connect(_client_network_variable_->_sockfd_,(struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_,_client_network_variable_->_len_);
			}
			else if(strcmp(buf, "rep") == 0){
				// struct req_service_listing temp_list[MAX_LISTING_SIZE];
				// int temp_number_of_requests;
				// extract_request_listing(temp_list, temp_file, &temp_number_of_requests, HARD);
				// for (int i =0; i < temp_number_of_requests; i++);
				// temp_file1 = fopen(file_name_temp1, "a");
				// add_request_to_file(temp_file, temp_file1, temp_list[0], HARD);
				// fputc('\0', temp_file1);
				// fclose(temp_file1);
				fclose(temp_file);
				temp_file1 = fopen(file_name_temp1, "a");
				fprintf(temp_file1, ",");
				fclose(temp_file1);

				char cmd[600];
				sprintf(cmd, "cat %s ", file_name_temp);
				int tempo = system(cmd);
				sprintf(cmd, "cat %s >> %s", file_name_temp, file_name_temp1);
				system(cmd);
				condition++;
				printf("condition %d - %d\n", condition, number_of_requests);
			}
			else if(strcmp(buf, "error") == 0){
				struct req_service_listing temp_list[MAX_LISTING_SIZE];
				int temp_number_of_requests, temp_var = 0;
				extract_request_listing(temp_list, temp_file, &temp_number_of_requests, SOFT);
				while(temp_var < number_of_requests){
					if(temp_list[1].code == req_listing[temp_var].code && strcmp(temp_list[1].feature, req_listing[temp_var].feature) == 0){
							if(req_listing[temp_var].booked == 1)
								req_listing[temp_var].booked = 0;
					}
					temp_var++;
				}
			}
		}
		fclose(msg_ptr);
		char cmd[350];

		sprintf(cmd, "cp %s %s", file_name_temp1, uni_shm->module_access_info[0].file);
		system(cmd);
		
	}
	if(strcmp(req, "res") == 0){
		printf("[INNER]entered res\n");
		int hop_count, max_hop_count;
		struct hop_ip_list *hop_list;
		temp_file = fopen(uni_shm->module_access_info[0].file, "r");
		hop_count_check(temp_file, &hop_count, &max_hop_count);	
		hop_list = (struct hop_ip_list *)malloc(hop_count * sizeof(struct hop_ip_list));
		extract_hop_list(temp_file, hop_list, hop_count);
		fclose(temp_file);
		_client_network_variable_->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
		_client_network_variable_->client_server_selection._client_address_.sin_family = AF_INET;
		_client_network_variable_->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
		_client_network_variable_->client_server_selection._client_address_.sin_port = htons(uni_shm->module_access_info[0].module_info.r_port);
		_client_network_variable_->_len_ = sizeof (_client_network_variable_->client_server_selection._client_address_);
		printf("sending result to %s - %d \n",hop_list[0].ip_address, uni_shm->module_access_info[0].module_info.r_port);
		result = connect(_client_network_variable_->_sockfd_,(struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_,_client_network_variable_->_len_);
		if(result==-1){
			perror("Client Error\n");
			return FAILED;
		}

		temp_file = fopen(uni_shm->module_access_info[0].file, "r");
		printf("NAME %s\n", uni_shm->module_access_info[0].file);
		no_of_characters_file(file_name_temp, &msg_size);
		file_send(_client_network_variable_, temp_file, msg_size);
		fclose(temp_file);
		printf("result sent\n");
		return OK;
	}
	if(strcmp(req, "rej") == 0){
		char buf[256], int_count[7];
		uni_shm->module_access_info[0].module_info.booked = 0;
		no_of_characters_file(uni_shm->module_access_info[0].file, &msg_size);
		msg_ptr = fopen(uni_shm->module_access_info[0].file, "r");
		file_send(_client_network_variable_, msg_ptr, msg_size);
		fclose(msg_ptr);
	}
	
	while(strcmp(req, "rej") == 0){
		printf("[INNER LOOP]\n" );
		char buf[256], int_count[7];
		temp_file = fopen(file_name_temp, "w+");
		printf("[INNER] waiting in rej %d\n", port_assigned);
		_server_network_variable_->client_server_selection._server_address_.sin_port = htons(port_assigned);
		bind( _server_network_variable_->_sockfd_, (struct sockaddr *)&_server_network_variable_->client_server_selection._server_address_, _server_network_variable_->_len_ );	
		file_recv(_server_network_variable_, _client_network_variable_, temp_file);
		fclose(temp_file);
		
		printf("received file to work on\n");
		while(access(file_name_temp, R_OK) == -1); 
		printf("file READY to work on\n");
		temp_file = fopen(file_name_temp, "r");
		file_next_word_extract(temp_file, buf);
		printf("%s\n", buf);
		if(strcmp(buf, "req") == 0){
			printf("uuuuuu-- req\n");
			struct req_service_listing temp_list[MAX_LISTING_SIZE];
			int temp_number_of_requests, hop_count, max_hop_count, i;
			struct hop_ip_list *hop_list;
			printf("uuuuuu-- req\n");
			extract_request_listing(temp_list, temp_file, &temp_number_of_requests, SOFT);
			hop_count_check(temp_file, &hop_count, &max_hop_count);						
			hop_list = (struct hop_ip_list *)malloc(hop_count * sizeof(struct hop_ip_list));
			extract_hop_list(temp_file, hop_list, hop_count);

			fclose(temp_file);

			printf("extraction complete\n");
			while(access(file_name_temp, W_OK) == -1); 
			temp_file = fopen(file_name_temp, "w+");
			fprintf(temp_file, "\"res\"");
			add_request_to_file(msg_ptr, temp_file, temp_list[temp_var], SOFT);
			for( i = hop_count -1; i >-1; i--)
				fprintf(temp_file, ",\"%s\",\"%d\"", hop_list[i].ip_address, hop_list[i].port_number);
			fprintf(temp_file, ",\"%s\",\"%d\"", ipaddress,port_assigned);
			printf("[INNER][REJ] askdfkasjdfkjasdlf -> %d\n", port_assigned);
			fprintf(temp_file, ",\"%d\",\"%d\"", hop_count+1, max_hop_count);
			fclose(temp_file);

			no_of_characters_file(file_name_temp, &msg_size);
			while(access(file_name_temp, R_OK) == -1); 
			temp_file = fopen(file_name_temp, "r");
			_client_network_variable_->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
			_client_network_variable_->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
			_client_network_variable_->client_server_selection._client_address_.sin_port = htons(uni_shm->port_number);
			result = connect(_client_network_variable_->_sockfd_,(struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_,_client_network_variable_->_len_);
			printf("sent haha\n");
			file_send(_client_network_variable_, temp_file, msg_size);
			fclose(temp_file);

		}
		else if(strcmp(buf, "boo") == 0 && uni_shm->module_access_info[0].module_info.booked == 0){
			printf("uuuuuu-- boo\n");
			int temp_number_of_requests, hop_count, max_hop_count, i;
			struct hop_ip_list *hop_list;
			struct req_service_listing temp_list[MAX_LISTING_SIZE];
			extract_request_listing(temp_list, temp_file, &temp_number_of_requests, SOFT);
			hop_count_check(temp_file, &hop_count, &max_hop_count);						
			hop_list = (struct hop_ip_list *)malloc(hop_count * sizeof(struct hop_ip_list));
			extract_hop_list(temp_file, hop_list, hop_count);
			fclose(temp_file);
					
			temp_file = fopen(file_name_temp, "w+");
			fprintf(temp_file,"\"hoo\"");
			add_request_to_file(msg_ptr, temp_file, temp_list[temp_var], SOFT);
			for( i = hop_count -1; i >-1; i--)
				fprintf(temp_file, ",\"%s\",\"%d\"", hop_list[i].ip_address, hop_list[i].port_number);
			fprintf(temp_file, ",\"%d\",\"%d\"", hop_count, max_hop_count);
			fclose(temp_file);

			no_of_characters_file(file_name_temp, &msg_size);
			temp_file = fopen(file_name_temp, "r");
			_client_network_variable_->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
			_client_network_variable_->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
			_client_network_variable_->client_server_selection._client_address_.sin_port = htons(uni_shm->port_number);
			result = connect(_client_network_variable_->_sockfd_,(struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_,_client_network_variable_->_len_);

			file_send(_client_network_variable_, msg_ptr, msg_size);
			fclose(temp_file);
			uni_shm->module_access_info[0].module_info.booked = 1;
		}
		else if(strcmp(buf, "rep") == 0 && uni_shm->module_access_info[0].module_info.booked == 1){
			printf("uuuuuu-- rep\n");
			struct hop_ip_list *hop_list;
			struct req_service_listing temp_list[MAX_LISTING_SIZE];
			int hop_count, max_hop_count;
			hop_count_check(temp_file, &hop_count, &max_hop_count);						
			hop_list = (struct hop_ip_list *)malloc(hop_count * sizeof(struct hop_ip_list));
			extract_hop_list(temp_file, hop_list, hop_count);
			fclose(temp_file);
			uni_shm->module_access_info[0].module_info.r_port = hop_list[0].port_number;
			strcpy(uni_shm->module_access_info[0].module_info.r_add , hop_list[0].ip_address);

			char cmd[350];
			sprintf(cmd, "cat %s > %s && cat %s", file_name_temp, uni_shm->module_access_info[0].file, uni_shm->module_access_info[0].file);
			system(cmd);
			return OK;
		}
		printf("looping rej\n");
	}
	return OK;
}

enum Return_code add_request_to_file(FILE *msg_ptr, FILE *temp_file, struct req_service_listing req_listing, enum entry con){
	char temp_char;
	int i;
	fprintf(temp_file, ",\"file\",\"%d\",\"%d\",\"%s\",\"%s\"", req_listing.number_of_characters, req_listing.code, req_listing.feature, req_listing.data_location);
	
	if( con == HARD ){
		fprintf(temp_file,",\"");
		fseek(msg_ptr, req_listing.data_position, SEEK_SET);
		for(i = 0 ; i < req_listing.number_of_characters ; i++)
			fputc(fgetc(msg_ptr), temp_file);
		temp_char = fgetc(msg_ptr);
		while(temp_char != ',' || temp_char == EOF){
			fputc(temp_char, temp_file);
			temp_char = fgetc(msg_ptr);
		}
	}
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

enum Return_code skip_backward( FILE *temp_file, int value){
	while(value-- != 0){
		while(fgetc(temp_file) != ',')fseek(temp_file, -2, SEEK_CUR);
		fseek(temp_file, -2, SEEK_CUR);
	}
	fseek(temp_file, 1, SEEK_CUR);
	return OK;
}
enum Return_code extract_hop_list(FILE *temp_file, struct hop_ip_list *hop_list, int hop_count){
	char int_count[7], buf[20];
	
	fseek(temp_file, 0, SEEK_END);
	skip_backward(temp_file, (hop_count+1)*2);
	int i = hop_count;
	while(hop_count-- != 0){
		file_next_word_extract(temp_file, buf);
		file_next_word_extract(temp_file, int_count);

		strcpy(hop_list[hop_count].ip_address, buf);
		hop_list[hop_count].port_number = atoi(int_count);
	}
	hop_count = i;	
	return OK;
}

enum Return_code hop_count_check(FILE *temp_file, int *hop_count, int *max_hop_count){
	
	char int_count[7], buf[20];

	fseek(temp_file, 0, SEEK_END);
	fseek(temp_file, -1, SEEK_CUR);
	
	skip_backward(temp_file, 2);
	
	file_next_word_extract(temp_file, int_count);
	*hop_count = atoi(int_count);

	file_next_word_extract(temp_file, int_count);
	*max_hop_count = atoi(int_count);

	skip_backward(temp_file, (*hop_count + 1)*2);

	return OK;
}



enum Return_code no_of_characters_file(char *file_name, int *msg_size){
	char cmd[1000];
	sprintf(cmd,"wc -c < %s", file_name);
	FILE *temp_pointer = popen(cmd, "r");
	fscanf(temp_pointer, "%d", msg_size);
	*msg_size--;
	return OK;
}

enum Return_code file_recv(struct network_variables *_server_network_variable_, struct network_variables *_client_network_variable_, FILE *msg_ptr){
	
	listen(_server_network_variable_->_sockfd_,0);
	printf("waiting at port-> %d\n", ntohs(_server_network_variable_->client_server_selection._server_address_.sin_port));
	_client_network_variable_->_sockfd_ = accept(_server_network_variable_->_sockfd_, (struct sockaddr *)&_server_network_variable_->client_server_selection._server_address_, &_server_network_variable_->_len_ );
	if (_server_network_variable_->_sockfd_ < 0 ){
			perror("[ERROR][NETWORK][INNER]-accept function error");
			return FAILED;
	}
		
	char msg[__MSG_SIZE], test_msg[__MSG_SIZE];
	int file_size, breakdown;
	int word_position = 0;
	if( 0 > read(_client_network_variable_->_sockfd_, msg, sizeof(msg))){
		perror("[ERROR][NETWORK][INNER]-recv function error");
		return FAILED;
	}
	printf("file recv[OUTPUT] %s\n",msg );
	word_position = 0;
	msg_word_extract(msg, test_msg, &word_position);
	
	if(strcmp(test_msg, "sen") == 0){
		word_position = 1;
		msg_word_extract(msg, test_msg, &word_position);
		file_size = atoi(test_msg);
		word_position = 2;
		msg_word_extract(msg, test_msg, &word_position);
		breakdown = atoi(test_msg);
		
		sprintf(msg, "\"che\",\"%d\",\"%d\"", file_size, breakdown);
		printf("[recv]%s\n", msg);
		write(_client_network_variable_->_sockfd_, msg, sizeof(msg));
		do{
			if( 0 > read(_client_network_variable_->_sockfd_, msg, sizeof(msg))){
				perror("[ERROR][NETWORK][INNER]-recv function error");
				return FAILED;
			}
			fprintf(msg_ptr, "%s", msg);
			
		}while( --breakdown > 0 );
	}
	return OK;
}

enum Return_code file_send(struct network_variables *_client_network_variable_ , FILE *msg_ptr, int size){
	char temp_msg[__MSG_SIZE], test[__MSG_SIZE];
	int checkpoint = 0, word_position = 0;
	do{
		sprintf(temp_msg, "\"sen\",\"%d\",\"%d\"", size, (size % __MSG_SIZE)? (size/__MSG_SIZE) + 1 : size/__MSG_SIZE);
		printf("%s --- size ->%lu -- %d\n", temp_msg, sizeof(temp_msg), getsockname(_client_network_variable_->_sockfd_, (struct sockaddr *)&_client_network_variable_->client_server_selection._client_address_, &_client_network_variable_->_len_));

		write(_client_network_variable_->_sockfd_, temp_msg, sizeof(temp_msg));
		printf("written\n");
		read(_client_network_variable_->_sockfd_, temp_msg, sizeof(temp_msg));
		
		word_position = 0;
		msg_word_extract(temp_msg, test, &word_position);
		
		if(strcmp(test, "che") == 0){
			word_position = 1;
			msg_word_extract(temp_msg, test, &word_position);
			
			if( atoi(test) == size)
				checkpoint = 1;
			else{
				checkpoint = 0;
				sprintf(temp_msg, "\"error\"");
				write(_client_network_variable_->_sockfd_, temp_msg, sizeof(temp_msg));
			}
		}
	}while(checkpoint == 0);
	
	word_position = 0 ;
	checkpoint = 0 ;
	while(word_position < size){
		temp_msg[checkpoint++] = fgetc(msg_ptr);
		word_position++ ;
		if(checkpoint == __MSG_SIZE - 2){
			temp_msg[checkpoint] = '\0';
			write(_client_network_variable_->_sockfd_, temp_msg, sizeof(temp_msg));	
			checkpoint = 0;
		}
	}
	if(checkpoint > 0){
		temp_msg[checkpoint] = '\0';
		write(_client_network_variable_->_sockfd_, temp_msg, sizeof(temp_msg));	
	}
	checkpoint = 0;
}

enum Return_code kill_packet_forward(struct network_variables *server, struct network_variables *client, FILE *temp_file, char *temp_file_name, char *temp_ip_address, int port_number, enum entry test){
	int hop_count, max_hop_count, i;
	char send_file[350], temp_char;
	struct hop_ip_list *hop_list;
	
	hop_count_check(temp_file, &hop_count, &max_hop_count);						
	if(hop_count == max_hop_count){
		return OK;
	}
	hop_list = (struct hop_ip_list *)malloc(hop_count * sizeof(struct hop_ip_list));
	extract_hop_list(temp_file, hop_list, hop_count);


	sprintf(send_file, "%s.temp", temp_file_name);
	FILE *msg_ptr = fopen(send_file, "w+");
	fseek(temp_file, 0, SEEK_SET);

	i = 4;
	while(i != 0){
		temp_char = fgetc(temp_file);
		fputc(temp_char, msg_ptr);
		if(temp_char == ',') i--;
	}
	fseek(msg_ptr, -1, SEEK_CUR);

	for( i = hop_count -1; i >-1; i--)
		fprintf(msg_ptr, ",\"%s\",\"%d\"", hop_list[i].ip_address, hop_list[i].port_number);
	fprintf(msg_ptr, ",\"%s\",\"%d\"", temp_ip_address, port_number);	
	fprintf(msg_ptr, ",\"%d\",\"%d\"", hop_count + 1, max_hop_count);

	fclose(msg_ptr);
	fclose(temp_file);

	msg_ptr = fopen(send_file, "r");
	packet_forward(server, client, msg_ptr, send_file, hop_list, hop_count+1, HARD, HARD);
	fclose(msg_ptr);
	return OK;
}

enum Return_code compare_forward_requesting_listing_with_available( char * file_name_temp, FILE *temp_file, struct hop_ip_list *hop_list, int hop_count, int max_hop_count, struct req_service_listing *req_listing, int number_of_requests, struct network_variables *server, struct network_variables *client){
	int i, msg_size; 
	fclose(temp_file);
	for(i=0 ; i< number_of_requests; i++){
		printf("requested -> %s\n", req_listing[i].feature);
	}
	for(i = 0; i < 11; i++ ){
			int j = number_of_requests;
			
			// if(strcmp(uni_shm->service_list[i].ip_address,hop_list[hop_count].ip_address) == 0){
			// 	char port_number[7];
			// 	sprintf(port_number,"%d",hop_list[hop_count].port_number);
			// 	if(strcmp(uni_shm->service_list[i].port_number, port_number) == 0){
			// 		test = 1;
			// 		break;
			// 	}
			// }

			if(uni_shm->service_list[i].local_remote == 1 && uni_shm->module_access_info[0].module_info.booked == 0){
				while(j-- != 0)
					if(strcmp(uni_shm->service_list[i].feature,req_listing[j].feature)== 0)
						break;
				printf("found feature match [OUTER] - %s\n",req_listing[j].feature);
				if(strcmp(uni_shm->service_list[i].feature,req_listing[j].feature) != 0)continue;
				printf("found %s ---- %s\n", uni_shm->service_list[i].ip_address, uni_shm->service_list[i].port_number);
				client->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
				client->client_server_selection._client_address_.sin_family = AF_INET;
				
				client->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");// inet_addr(uni_shm->service_list[i].ip_address);
				client->client_server_selection._client_address_.sin_port = htons(atoi(uni_shm->service_list[i].port_number));
				client->_len_ = sizeof (client->client_server_selection._client_address_);
				int result = connect(client->_sockfd_,(struct sockaddr *)&client->client_server_selection._client_address_,client->_len_);
				
				temp_file = fopen(file_name_temp, "w+");
				fprintf(temp_file, "\"req\"");
				add_request_to_file(temp_file, temp_file, req_listing[j], SOFT);
				for( j = hop_count -1; j >-1; j--)
					fprintf(temp_file, ",\"%s\",\"%d\"", hop_list[j].ip_address, hop_list[j].port_number);
				char temp_ip_address[20];
				GetIPaddress(temp_ip_address);
				fprintf(temp_file, ",\"%s\",\"%d\"", temp_ip_address, uni_shm->port_number);
				printf("wrote port number as ->%d\n", uni_shm->port_number);
				fprintf(temp_file, ",\"%d\",\"%d\"", hop_count + 1, max_hop_count);
				fclose(temp_file);
				printf("forwarding now\n");

				temp_file = fopen(file_name_temp,"r");
				no_of_characters_file(file_name_temp, &msg_size);
				printf("number of number_of_characters in the file -> %d\n", msg_size);
				file_send(client, temp_file, msg_size);
			}
			
			else if(uni_shm->service_list[i].local_remote == 0 && uni_shm->service_list[i].request_count != 0){
				j = hop_count;
				int test = 0;
				while(hop_count-- != 0){
					if(strcmp(uni_shm->service_list[i].ip_address,hop_list[hop_count].ip_address) == 0){
						char port_number[7];
						sprintf(port_number,"%d",hop_list[hop_count].port_number);
						if(strcmp(uni_shm->service_list[i].port_number, port_number) == 0){
							test = 1;
							break;
						}
					}
				}
				hop_count = j;
				if(test == 1) continue;

				printf("forwarding to %s ---- %d\n", uni_shm->service_list[i].ip_address, atoi(uni_shm->service_list[i].port_number));
				client->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
				client->client_server_selection._client_address_.sin_family = AF_INET;
				
				client->client_server_selection._client_address_.sin_addr.s_addr = inet_addr(uni_shm->service_list[i].ip_address);
				client->client_server_selection._client_address_.sin_port = htons(atoi(uni_shm->service_list[i].port_number));
				client->_len_ = sizeof (client->client_server_selection._client_address_);
				int result = connect(client->_sockfd_,(struct sockaddr *)&client->client_server_selection._client_address_,client->_len_);
				
				j = number_of_requests;

				temp_file = fopen(file_name_temp, "w+");
				fprintf(temp_file, "\"req\"");
				while(j-- != 0)
					add_request_to_file(temp_file, temp_file, req_listing[j], SOFT);
				for( j = hop_count -1; j >-1; j--)
					fprintf(temp_file, ",\"%s\",\"%d\"", hop_list[j].ip_address, hop_list[j].port_number);
				char temp_ip_address[20];
				GetIPaddress(temp_ip_address);
				fprintf(temp_file, ",\"%s\",\"%d\"", temp_ip_address, uni_shm->port_number);
				printf("wrote port number as ->%d\n", uni_shm->port_number);
				fprintf(temp_file, ",\"%d\",\"%d\"", hop_count + 1, max_hop_count);
				fclose(temp_file);
				printf("forwarding now\n");

				temp_file = fopen(file_name_temp,"r");
				no_of_characters_file(file_name_temp, &msg_size);
				printf("number of number_of_characters in the file -> %d\n", msg_size);
				file_send(client, temp_file, msg_size);
			}
	}
	return OK;
}


enum Return_code packet_retrace_backward( char * file_name_temp, FILE *temp_file, struct hop_ip_list *hop_list, int hop_count, int max_hop_count,struct network_variables *server, struct network_variables *client){
	char ipaddress[20];
	GetIPaddress(ipaddress);
	int i, msg_size;
	fclose(temp_file);

	for(i = 0; i < hop_count; i++){
		if(hop_list[i].port_number == uni_shm->port_number && strcmp(hop_list[i].ip_address, ipaddress) == 0){
			client->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
			client->client_server_selection._client_address_.sin_family = AF_INET;
			
			client->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
			printf(" retrace backward ipaddress -> %s\n", hop_list[i + 1].ip_address);
			client->client_server_selection._client_address_.sin_port = htons(hop_list[i + 1].port_number);
			printf(" retrace backward ipaddress -> %d\n", hop_list[i + 1].port_number);
			client->_len_ = sizeof (client->client_server_selection._client_address_);
			int result = connect(client->_sockfd_,(struct sockaddr *)&client->client_server_selection._client_address_,client->_len_);
			
			temp_file = fopen(file_name_temp,"r");
			no_of_characters_file(file_name_temp, &msg_size);
			printf("number of number_of_characters in the file -> %d\n", msg_size);
			file_send(client, temp_file, msg_size);
			fclose(temp_file);
			return OK;
		}
	}
	return FAILED;
}

enum Return_code packet_retrace_forward( char * file_name_temp, FILE *temp_file, struct hop_ip_list *hop_list, int hop_count, int max_hop_count,struct network_variables *server, struct network_variables *client){
		char ipaddress[20];
	GetIPaddress(ipaddress);
	int i, msg_size;
	fclose(temp_file);

	for(i = 0; i < hop_count; i++){
		if(hop_list[i].port_number == uni_shm->port_number && strcmp(hop_list[i].ip_address, ipaddress) == 0){
			client->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
			client->client_server_selection._client_address_.sin_family = AF_INET;
			
			client->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
			printf(" retrace forward ipaddress -> %s\n", hop_list[i - 1].ip_address);
			client->client_server_selection._client_address_.sin_port = htons(hop_list[i - 1].port_number);
			printf(" retrace forward ipaddress -> %d\n", hop_list[i - 1].port_number);
			client->_len_ = sizeof (client->client_server_selection._client_address_);
			int result = connect(client->_sockfd_,(struct sockaddr *)&client->client_server_selection._client_address_,client->_len_);
			
			temp_file = fopen(file_name_temp,"r");
			no_of_characters_file(file_name_temp, &msg_size);
			printf("number of number_of_characters in the file -> %d\n", msg_size);
			file_send(client, temp_file, msg_size);
			fclose(temp_file);
			return OK;
		}
	}
	return FAILED;
}

enum Return_code packet_forward(struct network_variables *server, struct network_variables *client, FILE *msg_ptr, char *send_file,struct hop_ip_list *hop_list, int hop_count, enum entry directon, enum entry new_old){
	int i = 0, msg_size; 
	if(new_old == HARD){

		for(i = 0; i < 11; i++ ){
			int test = 0;
			while(hop_count-- != 0){
				if(strcmp(uni_shm->service_list[i].ip_address,hop_list[hop_count].ip_address) == 0){
					char port_number[7];
					sprintf(port_number,"%d",hop_list[hop_count].port_number);
					if(strcmp(uni_shm->service_list[i].port_number, port_number) == 0){
						test = 1;
						break;
					}
				}
				printf("listing %s -- %d\n",uni_shm->service_list[i].ip_address,hop_list[hop_count].port_number);
			}
			if(test == 1) continue;
			client->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
			printf("socket -> %d\n", client->_sockfd_);
			
				client->client_server_selection._client_address_.sin_family = AF_INET;
				
				client->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
				client->client_server_selection._client_address_.sin_port = htons(atoi(uni_shm->service_list[i].port_number));
				client->_len_ = sizeof (client->client_server_selection._client_address_);
				int result = connect(client->_sockfd_,(struct sockaddr *)&client->client_server_selection._client_address_,client->_len_);
				
			// client->client_server_selection._client_address_.sin_family = AF_INET;
			// client->client_server_selection._client_address_.sin_addr.s_addr = inet_addr(uni_shm->service_list[i].ip_address);
			// client->client_server_selection._client_address_.sin_port = htons(atoi(uni_shm->service_list[i].port_number));
			// client->_len_ = sizeof (client->client_server_selection._client_address_);
			printf("[packet forward - HARD ]forwarding to %s - %s \n", uni_shm->service_list[i].ip_address, uni_shm->service_list[i].port_number);
			// int result = connect(client->_sockfd_,(struct sockaddr *)&client->client_server_selection._client_address_,client->_len_);

			no_of_characters_file(send_file, &msg_size);
			file_send(client, msg_ptr, msg_size);
			close(client->_sockfd_);
		}
	}
	else if(directon == HARD){
		char ip_address[7];
		int port_number = uni_shm->port_number;
		GetIPaddress(ip_address);
		int test = 0;
		while(hop_count-- != 0){
			if(strcmp(ip_address,hop_list[hop_count].ip_address) == 0)
				if(hop_list[hop_count].port_number == port_number){
					test = hop_count;
					break;	
				}
			printf("listing %s -- %d\n",uni_shm->service_list[i].ip_address, hop_list[hop_count].port_number);
		}
		hop_count = test;
		client->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
		printf("socket -> %d\n", client->_sockfd_);
		
				client->client_server_selection._client_address_.sin_family = AF_INET;
				
				client->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
				client->client_server_selection._client_address_.sin_port = htons(hop_list[hop_count + 1].port_number);
				client->_len_ = sizeof (client->client_server_selection._client_address_);
				int result = connect(client->_sockfd_,(struct sockaddr *)&client->client_server_selection._client_address_,client->_len_);
				
		// client->client_server_selection._client_address_.sin_family = AF_INET;
		// client->client_server_selection._client_address_.sin_addr.s_addr = inet_addr(hop_list[hop_count + 1].ip_address);
		// client->client_server_selection._client_address_.sin_port = htons(hop_list[hop_count + 1].port_number);
		// client->_len_ = sizeof (client->client_server_selection._client_address_);
		printf("[packet forward HARD SOFT ]forwarding to %s - %d \n", hop_list[hop_count+1].ip_address, hop_list[hop_count+1].port_number);
		// int result = connect(client->_sockfd_,(struct sockaddr *)&client->client_server_selection._client_address_,client->_len_);
		
		no_of_characters_file(send_file, &msg_size);
		file_send(client, msg_ptr, msg_size);
		close(client->_sockfd_);
	}
	else if(directon == SOFT){
		char ip_address[7];
		int port_number = uni_shm->port_number;
		GetIPaddress(ip_address);
		int test = 0;
		while(hop_count-- != 0){
			if(strcmp(ip_address,hop_list[hop_count].ip_address) == 0)
				if(hop_list[hop_count].port_number == port_number){
					test = hop_count;
					break;	
				}
			printf("listing %s -- %d\n",uni_shm->service_list[i].ip_address,hop_list[hop_count].port_number);
		}
		client->_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
		printf("socket -> %d\n", client->_sockfd_);
		
				client->client_server_selection._client_address_.sin_family = AF_INET;
				
				client->client_server_selection._client_address_.sin_addr.s_addr = inet_addr("127.0.0.1");
				client->client_server_selection._client_address_.sin_port = htons(hop_list[hop_count - 1].port_number);
				client->_len_ = sizeof (client->client_server_selection._client_address_);
				int result = connect(client->_sockfd_,(struct sockaddr *)&client->client_server_selection._client_address_,client->_len_);
				
		// client->client_server_selection._client_address_.sin_family = AF_INET;	
		// client->client_server_selection._client_address_.sin_addr.s_addr = inet_addr(hop_list[hop_count -1].ip_address);
		// client->client_server_selection._client_address_.sin_port = htons(hop_list[hop_count - 1].port_number);
		// client->_len_ = sizeof (client->client_server_selection._client_address_);
		printf("[packet forward SOFT SOFT ]forwarding to %s - %d \n", hop_list[hop_count -1].ip_address, hop_list[hop_count -1].port_number);
		// int result = connect(client->_sockfd_,(struct sockaddr *)&client->client_server_selection._client_address_,client->_len_);
		
		no_of_characters_file(send_file, &msg_size);
		file_send(client, msg_ptr, msg_size);
		printf("sent\n");
		close(client->_sockfd_);
	}
	printf("returning\n");
	getchar();
	getchar();
	getchar();
	getchar();
	return OK;
}