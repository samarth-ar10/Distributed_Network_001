
//----------------------------------------------------------shm module
//start
key_t key;
int shmid;
const void* shmaddr;

key_t get_key(char* path){
	
	int check = creat(path,0777);
	if(check == -1) {
		perror("file not created by c++");
		exit(1);
	}
	key = ftok(path,'P');
	printf("key allocated => %d\n", (int)key);
	if(key == -1){
		perror("Error :shared Memory Key Creation\n");
		exit (1);
	}

	return key;
}
/*
	function 
		name		-	get_shm_file_path
		description	-	to assign and get the key to the shared memory based on the size and the file path provided
	
	Parameters
		size_t		size 		-	size of the shared memory required
		char *		path 		-	path name/string for the shared file
*/
void get_shm_file_path(size_t size, char* path){
	key = get_key(path);
	shmid = shmget(key , size , IPC_CREAT | 0777);
	if(shmid == -1){
		perror("\nError : shared memory creation\n");
	}
	return;
}

/*
	function 
		name		-	get_shm_key_path
		description	-	to assign and get the key to the shared memory based on the size and the key assigned to the file path provided
	
	Parameters
		size_t		size 		-	size of the shared memory required
		int 		temp_key	-	key value of the previously created shared memory
*/
void get_shm_key_value(size_t size, int temp_key){
	key = (key_t)temp_key;
	shmid = shmget(key , size , IPC_CREAT | 0777);
	if(shmid == -1){
		perror("\nError : shared memory creation\n");
	}
	return;
}

const void* shm_attach(){
	shmaddr = shmat(shmid,NULL,0);
	if(shmaddr == (void *)-1){
		perror("\nError : shared memory attach\n");
		exit(1);
	}
	return shmaddr;
}

void shm_detach(){
 	int check = shmdt(shmaddr);
 	if (check == -1){
 		perror("\nError : shared memory detach");
 	}
 	return;
}

key_t check_key(){
	return key;
}

int check_shmid(){
	return shmid;
}
//end