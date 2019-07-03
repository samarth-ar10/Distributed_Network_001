#include "header.h"
#define True 1 	
//------------------------------------------------------------global values
//start
size_t size;
int shmid;
const void* shmaddr;
key_t key;
//end
//-----------------------------------------------------------menu display module
//start
char Menu(){
	printf("\nenter:\n");

	char choice;
	fflush(stdin);
	
	printf("\n1: Creating a Algorithm Module");
	printf("\n2: Creating a Monitor Module");
	printf("\n3: Display\n");
	printf("\nq: Quitting\n");
	printf("\nchoice\t=>\t");
	scanf("%c",&choice);
	if( choice == 'q' ){
		printf("\n!!!!!!-----Quitting-----!!!!!\n\n");
		exit(0);
	}
	fflush(stdin);
	return choice;	
}
//end

void main(int argc, char** argv){
	char *PORT_NO, cmd[200];
	int pid;
	printf("\n\n\n\n---------------------------Starting-Program-----------------------------------");

	printf("\nThis is a Product Developed by Samarth and is meant for experimental purposes. \nThe code is open to everyone as long as it is not used for commertial purposes\n");

	//-----------------------------preparing the shm space 

	uni_shm = (struct universal_shm*) malloc (sizeof(struct universal_shm));
	
	get_shm_file_path(sizeof(uni_shm), argv[1]);
	
	uni_shm = (struct universal_shm *)shm_attach();
	uni_shm->key = key;
	uni_shm->shmid = shmid;
	uni_shm->count = 0;
	uni_shm->module_access_info[0].code = 0;
	printf("%s\n", uni_shm->module_access_info[0].module_info.name);

	//-----------------------------Displaying the menu and taking actions accordingly
	char i =Menu();
	pid=fork();
	while (True){
		fflush(stdin);
		if(pid == 0)
		{	
			switch(i){ 
				case '1'://strcpy(cmd,"mate-terminal -t \"Monitor Module\" -x ./test");
						sprintf(cmd,"xterm -e ./Algorithm %d %s", uni_shm->key, argv[1]);	
						system(cmd);
						break;
				case '2': 
						sprintf(cmd,"xterm -e ./Monitor %d %s", uni_shm->key, argv[1]);
						system(cmd);
						break;
				case '3':
						printf("\nDisplaying");
						break ;
				default:
						printf("\n wrong choice!!");
			}
			fflush(stdin);
			break;
		}
		if( pid != 0){ 
			i=Menu();
			printf("%c\n",i);
			if(!(i == '1' || i == '2' || i == '3'))
				printf("\n!!! wrong Choice !!!\n");
			else{
				pid = fork();
				fflush(stdin);
			}
		}
	}

	return;
}
 