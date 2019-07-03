#include "user_header.h"

void main(int argc, char **argv){
	printf("given key is => %s\n", argv[ argc - 1 ]);
	Activate_shared_memory(argc, argv);
	printf("file name %s \n",uni_shm->module_access_info[0].file);
	char choice;

	Activate_network_module();

	choice = getchar();
	switch(choice){
		case '1':
				printf("entering send\n");
				FILE *sendfile = fopen("./images/test_3char.txt","r");
				struct service_file_name *file_names = (struct service_file_name *)malloc(sizeof(struct service_file_name));
				
				if(sendfile == NULL){
					printf("GOOD LUCK \n");
				}
				Network_module_request_msg_file();
				Network_module_create_request_msg("red", sendfile, 1232);
				Network_module_request_service();
				getchar();
				getchar();
				
				//Network_module_send("./images/test1.jpg",file,"req","127.0.0.1","50403","custom", sendfile);
				//Network_module_request_service("red", "./images/test1.jpg", sendfile);
				//Network_module_register_service("red", sendfile, temp_file);
				fclose(sendfile);
				break;
		case '2': 
				Network_module_recv();
				break;
		default:
				break;
	}
	printf("----------------------------------------------completed\n");
	getchar();
	getchar();

}

				// Network_module_register_service("white", file_names);
				
				// FILE *received_file = fopen(file_names->received_file_name,"r");
				// if(received_file == NULL){
				// 	printf("received_file not opening -> %s", file_names->received_file_name);
				// }
				// FILE *result_file = fopen(file_names->result_file_name,"w");
				// if(result_file == NULL){
				// 	printf("\nresult_file not opening -> %s \n", file_names->result_file_name );
				// }
				// fprintf(result_file, "processed data\n");
				// fclose(received_file);
				// fclose(result_file);
				
				// Network_module_serviced_reply(file_names);
