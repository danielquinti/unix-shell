/* Shell by Iván García and Daniel Quintillán.
 * Logins: ivan.garcia.fernandez and daniel.quintillan
 * IDs: 54157405-H and 77679642-V
 * As we changed groups, we were allowed to keep the old implementation
 * of the list.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <locale.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include "list.h"
#include "signal.h"
#define INPUTSIZE 2048
#define MAXNAME 1024
#define MAXSEARCHLIST 128

//The searchlist of assignment 3
char *searchlist[MAXSEARCHLIST] = {NULL};

list allocations;
list l_jobs;

/*
 * Lab assignment 0
*/

void cmd_authors(char *args[]) {
	if (args[0] == NULL || strcmp(args[0], "-n") == 0)
		printf("Authors: Iván García Fernández and "
			"Daniel Quintillán Quintillán\n");
	if (args[0] == NULL || strcmp(args[0], "-n") == 0)
		printf("Logins: ivan.garcia.fernandez and"
			"daniel.quintillan\n");
	else // could handle unexpected arguments
		printf("Authors: Iván García Fernández and "
		"Daniel Quintillán Quintillán\nLogins: ivan.garcia.fernandez"
		" and daniel.quintillan\n");
}
// getpid() should never fail
void cmd_pid(char *args[]) { printf("This process' PID is %d\n", getpid()); }

void cmd_chdir(char *args[]) {
	char cwd[PATH_MAX];
	if (args[0] == NULL) {
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			printf("The current directory is: %s\n", cwd);
			return;
		} else {
			printf("getcwd() error: %s\n", strerror(errno));
			return;
		}
	} else if (chdir(args[0]) != 0)
		printf("Directory %s not found: %s\n", args[0],
			strerror(errno));
}

void cmd_date(char *args[]) {
	time_t now = time(NULL); //Get the system time, as the argument is
	// NULL, time() cannot fail
	struct tm *tm = localtime(&now); // Gets a tm struct from a time_t
	char formatted_date[128]; // Buffer for strftime to work
	//strftime(formatted_date, 128, "The date is %a, %d/%m/%Y.", tm);
	// Prints the date according to the current locale
	strftime(formatted_date, 128, "The date is %x.", tm);	
	printf("%s\n", formatted_date);
	return;
}

// The same as cmd_date, but with option %X in strftime
void cmd_time(char *args[]) {	
	time_t now = time(NULL); //Get the system time, as the argument is
	// NULL, time() cannot fail
	struct tm *tm = localtime(&now); // Gets a tm struct from a time_t
	char formatted_date[128]; // Buffer for strftime to work
	//strftime(formatted_date, 128, "The date is %a, %d/%m/%Y.", tm);
	// Prints the date according to the current locale
	strftime(formatted_date, 128, "The time is %X.", tm);	
	printf("%s\n", formatted_date);
	return;
}

void cmd_exit(char *args[]) { exit(0); }

/*
 * Lab assignment 1
*/

void cmd_create(char *args[]) {
	if (args[0] == NULL) {
		printf("Please introduce valid arguments\n");
	} else if (strcmp(args[0], "-d") == 0) {
		if (args[1] == NULL) {
			printf("Please introduce a name for the directory\n");
		} else if (mkdir(args[1], 0777) == -1) {
			printf("mkdir() error: %s.\n", strerror(errno));
		} else {
			printf("Created directory %s\n", args[1]);
		}
	} else {
		if (open(args[0], 
		O_CREAT | O_EXCL , S_IRUSR | S_IWUSR | S_IXUSR) == -1) {
			printf("%s \n", strerror(errno));
		} else {
			printf("Created file %s\n",args[0]);
		}
	}	
}

int is_empty(char *originalDir) {
	int n = 0;
	struct dirent *d;
	DIR *dir = opendir(originalDir);
	if (dir == NULL)
		return 1;
	while ((d = readdir(dir)) != NULL) {
		if (++n > 2) {
			closedir(dir);
			return 0;
		}
	}
	return 1;
}

int is_directory(char * name) {
	struct stat s;
	if (lstat(name, &s) == -1)
		return 0;
	return S_ISDIR(s.st_mode);
}

void recursively_delete(char *dir) {
	DIR *p;
	struct dirent *d;
	char aux[512];
	if ((p = opendir(dir)) == NULL) {
		if (unlink(dir) == -1)
			printf("Cannot delete '%s': %s\n", dir,
				strerror(errno));
		return;
	} else
		while ((d = readdir(p)) != NULL) {
			sprintf(aux, "%s/%s", dir, d->d_name);
			if ((strcmp(d->d_name, ".") == 0)
				|| (strcmp(d->d_name, "..") == 0))
				continue;
			recursively_delete(aux);
		}
	closedir(p);
	if (rmdir(dir) == -1)
		printf("Cannot delete file: %s\n", strerror(errno));
}

void cmd_delete(char *args[]) {
	if (args[0] == NULL) {
		printf("Please introduce the file/directory to be deleted as "
			"an argument\n");
	} else if (strcmp(args[0], "-r") == 0) {
		if (args[1] == NULL) {
			printf("Please introduce the directory to be deleted "
				"as an argument\n");
		} else {
			recursively_delete(args[1]);
		}
	} else if (is_directory(args[0])) {
		if (is_empty(args[0])) {
			if (rmdir(args[0]) == -1) {
				printf("Directory empty but "
				"couldn't be deleted: %s\n",strerror(errno));
			}
		} else
			printf("Use -r to delete non-empty folders\n");
	} else if (unlink(args[0]) == -1) {
		printf("%s\n",strerror(errno));
	}
}

char file_type(mode_t m) {
	switch (m & S_IFMT) { /*and bit by bit with format bits,0170000 */
		case S_IFSOCK:
			return 's'; /*socket */
		case S_IFLNK:
			return 'l'; /*symbolic link*/
		case S_IFREG:
			return '-'; /* normal file */
		case S_IFBLK:
			return 'b'; /*block device*/
		case S_IFDIR:
			return 'd'; /*directory */
		case S_IFCHR:
			return 'c'; /*char device*/
		case S_IFIFO:
			return 'p'; /* pipe */
		default:
			return '?'; /* unknown, shouldn't appear */
	}
}

char *convert_mode(mode_t m) {
	static char permissions[12];
	strcpy(permissions, "----------");
	permissions[0] = file_type(m);
	if (m & S_IRUSR)
		permissions[1] = 'r'; /*owner*/
	if (m & S_IWUSR)
		permissions[2] = 'w';
	if (m & S_IXUSR)
		permissions[3] = 'x';
	if (m & S_IRGRP)
		permissions[4] = 'r'; /*group*/
	if (m & S_IWGRP)
		permissions[5] = 'w';
	if (m & S_IXGRP)
		permissions[6] = 'x';
	if (m & S_IROTH)
		permissions[7] = 'r'; /*rest*/
	if (m & S_IWOTH)
		permissions[8] = 'w';
	if (m & S_IXOTH)
		permissions[9] = 'x';
	if (m & S_ISUID)
		permissions[3] = 's'; /*setuid, setgid and stickybit*/
	if (m & S_ISGID)
		permissions[6] = 's';
	if (m & S_ISVTX)
		permissions[9] = 't';
	return (permissions);
}

void iterate_query(char *arg) {
	struct group *group;
	struct passwd *name;
	struct stat myStructure;

	if (lstat(arg, &myStructure) != -1) {
		
		if (getgrgid(myStructure.st_gid)!=NULL)
			group = getgrgid(myStructure.st_gid);
		else {printf("An unexpected error ocurred\n"); return;}
		
		if (getpwuid(myStructure.st_uid)!=NULL)	
			name = getpwuid(myStructure.st_uid);
		else {printf("An unexpected error ocurred\n"); return;}

		printf("%7ld %s %3ld %s %s %8ld %.12s ",
			(long)myStructure.st_ino,
			convert_mode(myStructure.st_mode),
			(long)myStructure.st_nlink,
			name->pw_name, group->gr_name,
			(long)myStructure.st_size,
			&ctime(&myStructure.st_atime)[4]);

		// If the file is a link
		if ((myStructure.st_mode & S_IFMT) == S_IFLNK) {
			char link[(int)myStructure.st_size];
			readlink(arg, link, myStructure.st_size + 1);
			link[(int)myStructure.st_size] = '\0';
			printf("%s -> %s", arg, link);
		} else
			printf("%s", arg);
		printf("\n");
	} else {
		printf("Cannot access the file\n");
	}
}

void cmd_query(char *args[]) {
	if (args[0] == NULL) {
		printf("Please introduce a valid argument\n");
	}
	for (int i = 0; args[i] != NULL; i++) {
		iterate_query(args[i]);
	}
}

long file_size(char *name) {
	struct stat s;
	if ((stat(name, &s) == -1))
		return (long)-1;
	return (long)s.st_size;
}

int list_dir(char *dir, int rec, int longlist, int hidden) {
	DIR *dirStream;
	struct dirent *d;
	char aux[512];
	if ((dirStream = opendir(dir)) == NULL)
		return -1;
	while ((d = readdir(dirStream)) != NULL) {
		if (!hidden && d->d_name[0] == '.')
			continue;
		sprintf(aux, "%s/%s", dir, d->d_name);
		if (longlist)
			iterate_query(aux);
		else
			printf("%s: %ld\n", d->d_name, file_size(aux));
		if (rec && !(((strcmp(d->d_name, ".") == 0))
			|| (strcmp(d->d_name, "..") == 0))
			&& is_directory(aux))
			list_dir(aux, rec, longlist, hidden);
	}
	return closedir(dirStream);
}

void cmd_list(char *args[]) {
	int hidden = 0, rec = 0, longlist = 1, i;
	for (i = 0; args[i] != NULL; i++) {
		if (strcmp(args[i], "-r") == 0) {
			rec = 1;
		} else if (strcmp(args[i], "-h") == 0) {
			hidden = 1;
		} else if (strcmp(args[i], "-n") == 0) {
			longlist = 0;
		} else
			break;
	}
	if (args[i] == NULL) {
		list_dir(".", rec, longlist, hidden);
	}
	for (; args[i] != NULL; i++) {
		if (list_dir(args[i], rec, longlist, hidden) == 1) {
			perror("Cannot list");
		} else if (longlist) {
			iterate_query(args[i]);
		} else
			printf("%s: %ld \n", args[i], file_size(args[i]));
	}
}

/*
 * Lab assignment 2
 */

struct element {
	void* mem; // Address of the block
	int size; // Size of the block
	char type[32]; // Type: a for malloc, m for mmap and s for shared
	time_t allocation_time; // Time when the block was allocated
	char file[INPUTSIZE]; // Filename if the type is m, NULL otherwise
	key_t key; // Key of the shared memory or 0 otherwise

};

//Allocate and its helper functions
void print_malloc() {
	char formatted_date[128];
	for (pos p = first(allocations); (p != NULL); p = next(p)) {
		struct element *elem =data(allocations, p);
		if (strcmp(elem->type, "malloc") == 0) {
			printf("%p: ", elem->mem);
			printf("size:%d. ", elem->size);
			printf("%s", elem->type);
			struct tm *tm;
			tm = localtime(&(elem->allocation_time));
			strftime(formatted_date,
				128, "%a %b %d %r %Y", tm);
			printf(" %s\n", formatted_date);
		}
	}
}

void allocate_malloc(char *args[]) {
	if (args[0] == NULL) { // Allocate args[0] memory
		print_malloc();
		return;
	}
	struct element *elem = malloc(sizeof(*elem));
	elem->mem = elem; // mem is the address of the block
	elem->size = atoi(args[0]); // size of the block
	strcpy(elem->type, "malloc"); // the element is of type malloc
	elem->allocation_time = time(NULL);		
	strcpy(elem->file, "");
	elem->key = (key_t) 0;
	insert(&allocations, elem);
	printf("Allocated %s bytes at %p\n", args[0], elem);
}

void *mmap_file (char * file, int protection) {
	int df, map = MAP_PRIVATE, mode = O_RDONLY;
	struct stat s;
	void *p;
	if (protection&PROT_WRITE)  mode = O_RDWR;
	if (stat(file,&s) == -1 || (df = open(file, mode)) == -1)
		return NULL;
	if ((p = mmap(NULL, s.st_size, protection, map, df, 0)) == MAP_FAILED)
		return NULL;

	// Our code
	struct element *elem = malloc(sizeof(*elem));
	elem->mem = elem; // mem is the address of the block
	elem->size = s.st_size; // size of the block
	strcpy(elem->type, "mmap"); // the element is of type mmap
	elem->allocation_time = time(NULL);		
	strcpy(elem->file, file);
	elem->key = (key_t) 0;
	insert(&allocations, elem);

	return p;
}

void print_mmap() {
	char formatted_date[128];
	for (pos p = first(allocations); (p != NULL); p = next(p)) {
		struct element *elem =data(allocations, p);
		if (strcmp(elem->type, "mmap") == 0) {
			printf("%p: ", elem->mem);
			printf("size:%d. ", elem->size);
			printf("%s ", elem->type);
			printf("%s", elem->file);
			struct tm *tm;
			tm = localtime(&(elem->allocation_time));
			strftime(formatted_date,
				128, "%a %b %d %r %Y", tm);
			printf(" %s\n", formatted_date);
		}
	}
}


void allocate_mmap(char *args[]) {
	char *perm;
	void *p;
	int protection = 0;

	if (args[0] == NULL) {
		print_mmap();
		return;
	}
	// If perm argument is valid
	if ((perm = args[1]) != NULL && strlen(perm) < 4) {
		if (strchr(perm,'r') != NULL) protection |= PROT_READ;
		if (strchr(perm,'w') != NULL) protection |= PROT_WRITE;
		if (strchr(perm,'x') != NULL) protection |= PROT_EXEC;
	}
	if ((p = mmap_file(args[0], protection)) == NULL)
		perror ("Mapping impossible to execute\n");
	else
		printf ("File %s mapped at %p\n", args[0], p);
}

void * getShmgetMemory(key_t keyt, off_t size, char *arg[0]){
	void * p;
	int aux,id,flags=0777;
	struct shmid_ds s;
	if (size) //If size is not 0 it creates in exclusive mode
		flags=flags | IPC_CREAT | IPC_EXCL;
	//If size is 0 it tries to access an already created one
	if (keyt==IPC_PRIVATE) /*it doesn't work*/{
		errno=EINVAL; 
		return NULL;
	}
	if ((id=shmget(keyt, size, flags))==-1)
		return (NULL);
	if ((p=shmat(id,NULL,0))==(void*) -1){
		aux=errno; /*It's been created and can't be mapped*/
		if (size) /*It's deleted */
			shmctl(id,IPC_RMID,NULL);
		errno=aux;
		return (NULL);
	}
	shmctl (id,IPC_STAT,&s);
	printf ("Shmget memory with key %s assigned to %p\n",arg[0],p);
	struct element *elem = malloc(sizeof(*elem));
	elem->mem = p;
	elem->size = s.shm_segsz;
	elem->allocation_time = time(NULL);
	elem->key = keyt;
	insert(&allocations,elem);	
	return (p);
}

void print_shared() {
	char formatted_date[128];
	for (pos p = first(allocations); (p != NULL); p = next(p)) {
		struct element *elem =data(allocations, p);
		if (strcmp(elem->type, "shared") == 0) {
			printf("%p: ", elem->mem);
			printf("size:%d. ", elem->size);
			printf("%s", elem->type);
			printf("%s", elem->file);
			struct tm *tm;
			tm = localtime(&(elem->allocation_time));
			strftime(formatted_date,
				128, "%a %b %d %r %Y", tm);
			printf(" %s\n", formatted_date);
		}
	}
}

void allocate_createshared(char *arg[]) {
	key_t k;
	off_t tam = 0;
	void *p;
	if (arg[0] == NULL || arg[1] == NULL) { // Print the list of shared
		print_shared();
		return;
	}
	// Create and map new block of shared memory
	// to the process address space
	k = (key_t) atoi(arg[0]);
	if (arg[1] != NULL)
		tam=(off_t) atoll(arg[1]);
	if ((p=getShmgetMemory(k,tam,&arg[0]))==NULL)
		perror ("Impossible to obtain shmget memory");
}

void allocate_shared(char *arg[]){
	key_t k;
	void *p;
	if (arg[0]==NULL){
		print_shared();
		return;
	}
	k = (key_t) atoi(arg[0]);
	if ((p=getShmgetMemory(k,0,&arg[0]))==NULL)
		perror ("Impossible to obtain shmget memory");
}

void print_all() {
	struct tm *tm;
	char formatted_date[128];
	for (pos p = first(allocations); (p != NULL); p = next(p)) {
		struct element *elem =data(allocations, p);
		printf("%p: ", elem->mem);
		printf("size:%d. ", elem->size);
		if (strcmp(elem->type, "malloc") == 0)
			printf("malloc ");
		if (strcmp(elem->type, "mmap") == 0)
			printf("mmap %s ", elem->file);
		if (strcmp(elem->type, "shared") == 0)
			printf("shared memory (key %d)", elem->key);
		tm = localtime(&elem->allocation_time);
		strftime(formatted_date, 128, "%a %b %d %r %Y", tm);
		printf("%s\n", formatted_date);
	}
}

void cmd_allocate(char *args[]) {
	if (args[0] == NULL)
		print_all();
	else if (strcmp(args[0], "-malloc") == 0)
		allocate_malloc(&args[1]);
	else if (strcmp(args[0], "-mmap")==0)
		allocate_mmap(&args[1]);
	else if (strcmp(args[0], "-createshared")==0)
		allocate_createshared(&args[1]);
	else if (strcmp(args[0], "-shared")==0)
		allocate_shared(&args[1]);
}

void deallocate_malloc(char *args[]) {		
	if (args[0] == NULL) {
		print_malloc();
		return;
	}
	int size = atoi(args[0]);
	struct element *elem;
	pos p;
	for (p = first(allocations); !at_end(allocations,p); p = next(p)) {
		elem = data(allocations, p);
		if (strcmp(elem->type, "malloc") == 0 && elem->size == size)
			break;
	}
	if (at_end(allocations, p)){
			printf("Not found\n");
			return;
	}
	free(elem);
	delete_elem(&allocations, p);//Deletes the element from the list
	printf("Deallocated %s bytes from %p\n", args[0], elem);
}

void deallocate_mmap(char *args[]) {
	if (args[0] == NULL) {
		print_mmap();
		return;
	}
	char *file = args[0];
	printf("your file is: %s\n", file);
	pos s;
	for (s = first(allocations); !at_end(allocations, s); s = next(s)) {
		struct element *t =data(allocations, s);			
		if (strcmp(t->file, file) == 0)
			break;
	} 
	if (at_end(allocations, s)){
		printf("Not found\n");
		return;
	}
	struct element *elem = data(allocations, s);
	printf("Deallocated file %s\n", elem->file);
	printf("elem->mem: %p", elem->mem);
	munmap(elem->mem, elem->size);
	free(elem);
	delete_elem(&allocations, s);
}

void deallocate_shared(char *args[]) {
	if (args[0]==NULL){
		print_shared();
		return;
	}
	if (allocations !=NULL) {
		int key = atoi(args[0]);
		pos s;
		s = first(allocations);
		for (pos s = first(allocations); !at_end(allocations, s);
				s = next(s)){
			struct element *t =data(allocations, s);
			if (t->key == key)
				break;
		} 
		if (at_end(allocations, s)){
			printf("Not found\n");
			return;
		}
		struct element *m = data(allocations, s);
		shmdt(m->mem);
		free(m);
		delete_elem(&allocations, s);
		printf("Deallocated memory with key %d\n",m->key);	
	} 
 }

void deallocate_addr_malloc (pos p) {	
	if (at_end(allocations,p)) {
		printf("Not found\n");
		return;
	}
	struct element *m = data(allocations,p);
	free(m->type);
	free(m);
	delete_elem(&allocations, p);//Deletes the element from the malloc list
	printf("Deallocated bytes from %p\n",m->mem);
}

void deallocate_addr_mmap (pos p) {
	if (at_end(allocations,p)) {
		printf("Not found\n");
		return;
	}
	struct element *m = data(allocations, p);
	munmap(m->mem, m->size);
	free(m->type);
	free(m);
	delete_elem(&allocations, p);
	printf("Deallocated file from %p\n",m->mem);
}

void deallocate_addr_shared (pos p) {
	if (at_end(allocations,p)) {
		printf("Not found\n");
		return;
	}
	struct element *m = data(allocations, p);
	shmdt(m->mem);
	free(m->type);
	free(m);
	delete_elem(&allocations, p);	
	printf("Deallocated memory from address %p\n",m->mem);
}

pos find_mem(list l, void * r) {
	struct element *m;
	pos p;	
	for (p = first(l); !at_end(l, p); p = next(p)) {
		m = data(l, p);
		if (r == m->mem)
			break;
	}
	return p;
}

void deallocate_addr(char* args[]) {		
	// Transforms the args string to a pointer
	void *r= (void*)(long int) strtoul(args[0], NULL, 16);
	pos p = find_mem(allocations, r);
	struct element *elem = data(allocations, p);
	if (strcmp(elem->type, "malloc") == 0)
		deallocate_addr_malloc(p);
	else if (strcmp(elem->type, "mmap") == 0)
		deallocate_addr_mmap(p);
	else if (strcmp(elem->type, "shared") == 0)
		deallocate_addr_shared(p);
	else printf("Memory Adress not found\n"); 
}

void cmd_deallocate(char *args[]) {
	if (args[0] == NULL)
		print_all();
	else if (strcmp(args[0], "-malloc") == 0)
		deallocate_malloc(&args[1]);
	else if (strcmp(args[0], "-mmap")==0)
		deallocate_mmap(&args[1]);
	else if (strcmp(args[0], "-shared")==0)
		deallocate_shared(&args[1]);
	else deallocate_addr(&args[0]);
	
}

void * StringToPointer (char * s){
  void * p;

  p=(void *) strtoull (s,NULL,16);
  return p;
}

/*Auxiliar function of CmdMemdump*/
void Memdump(void * addr, int length){
  int c,i;
  unsigned char *pc = (unsigned char*)addr;
  for (c=0;c<length;c+=25){
    for (i=0;(((i+c)<length)&&i<25);i++){
      if ((pc[c+i]<0x20)||(pc[c+i]>0x7e))
        printf("   ");
      else{
        printf("  %c",pc[c+i]);
      }

    }
    printf("\n");
    for (i = 0; (((i+c)<length)&&i<25); i++)
      printf(" %02x", pc[c+i]);
    printf("\n");
    }
}


/*Shows the contents of cont bytes starting at memory
address addr. If cont is not specified, it shows 25 bytes. For each
byte it prints its hex value and its associate char (a blank if it
is a non-printable character). It prints 25 bytes per line.*/
void cmd_dump(char *tr[]){
    if (tr[1]==NULL)
      Memdump(StringToPointer(tr[0]),25);
    else Memdump(StringToPointer(tr[0]),atoi(tr[1]));
}

void recursive(int n) {
	char automatic[2048];
	static char staticChar[2048];

	printf("parameter n:%d in %p\n", n, &n);
	printf("static array in %p\n", staticChar);
	printf("automatic array in %p\n", automatic);
	n--;
	if (n>0)
		recursive(n);
}

void cmd_recursive(char *ch[]){
	if (ch[0] != NULL)
		recursive(atoi(ch[0]));
	else
		printf("Please introduce a valid n\n");
}

void cmd_rmkey(char *args[]){
	key_t clave;
	int id;
	char *key=args[0];
	if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE){
		printf ("   rmkey clave_valida\n");
		return;
	}
	if ((id=shmget(clave,0,0666))==-1){
		perror ("shmget: Obtention impossible.");
		return;
	}
	if (shmctl(id,IPC_RMID,NULL)==-1)
		perror ("shmctl: Deleting impossible\n");
	else printf("Shared memory succesfully eliminated\n");

}

int globl1 = 1;
int globl2 = 2;
int globl3 = 3;

void cmd_mem() {
	int local1,local2,local3;
	printf("Address of the function CmdRecursive: %p\n",cmd_recursive);
	printf("Address of the function Malloc: %p\n",allocate_malloc);
	printf("Address of the function Cmd_rmkey: %p\n",cmd_rmkey);
	printf("Address of the variable globl1: %p\n",(void*)&globl1);
	printf("Address of the variable globl2: %p\n",(void*)&globl2);
	printf("Address of the variable globl3: %p\n",(void*)&globl3);
	printf("Address of the variable local1: %p\n",(void*)&local1);
	printf("Address of the variable local2: %p\n",(void*)&local2);
	printf("Address of the variable local3: %p\n",(void*)&local3);
}

#define READFINISHED ((ssize_t)-1)

ssize_t read_file(char *fich, void *p, ssize_t n) {
	ssize_t nread, siz = n;
	int df, aux;
	struct stat s;
	if (stat (fich, &s) == -1 || (df = open(fich, O_RDONLY)) == -1)
		return ((ssize_t) - 1);
	if (n == READFINISHED)
		siz = (ssize_t) s.st_size;
	if ((nread = read(df,p, siz)) == -1) {
		aux = errno;
		close(df);
		errno = aux;
		return ((ssize_t) - 1);
	}
	close (df);
	return (nread);
}


void cmd_read(char *args[]) {
	void *mem;
	int cont;
	if (args[0] == NULL) {
		printf("Please introduce arguments\n");
		return;
	}
	if (args[2] == NULL)
		cont = sizeof(args[0]);
	else 
		cont = atoi(args[2]);
	mem = (void*) strtoul(args[1],NULL,0);
	read_file(args[0], mem, cont);
} 


void write_file(char* fich, void*p, ssize_t n, int o){
	int file = 0;
	if((file = open(fich,O_RDONLY)) < 0 && errno == ENOENT){
		file = open(fich, O_CREAT|O_WRONLY, 0666);
		write(file, p, n);
		close(file);
	} else { 
		if (o != 0) {
			file = open(fich, O_WRONLY| O_TRUNC);
			write(file, p, n);
			close(file);
		} else 
			printf("Cannot execute write.\n");
	}
}

void cmd_write(char *args[]) {
	char *name;
	void *mem;
	int o;
	if (args[0] == NULL) {
		printf("Please introduce arguments\n");
		return;
	}
	name = strdup(args[0]);
	mem = (void*) strtoul(args[1], NULL, 0);
	if (args[3] != NULL)
		o = atoi(args[3]);
	else
		o = 0;
	int cont = atoi(args[2]);
	write_file(name, mem, cont, o);
}

/*
 * Lab assignment 3
*/

void cmd_set_priority(char *args[]) {
	int oldpriority;
	if (args[0] == NULL)
		return;
	if (args[1] == NULL) { // Pid provided: read
		if ((oldpriority = getpriority(PRIO_PROCESS,
			(id_t) atoi(args[0]))) == -1)
			printf("Such process cannot be found.\n");
		else 
			printf("The priority of this process is %d\n",
				oldpriority);
		return;
	} // Pid and priority provided: write
	if (setpriority(PRIO_PROCESS, atoi(args[0]), atoi(args[1])) == -1)
		printf("error\n");
	else
		printf("The priority of the process has been changed to %d\n",
			atoi(args[1]));
}

void cmd_fork() {
	pid_t pid;
	if ((pid = fork()) == -1) {
		perror("Impossible to create child process");
		return;
	}
	if (pid == 0)
		printf("entering child process...\n");
	else {
		waitpid(pid, NULL, 0);
		printf("entering father...\n");
	}
}

int search_list_add_dir(char *dir) {
	int i = 0;
	char *p;
	while (i < MAXSEARCHLIST - 2 && searchlist[i] != NULL)
		i++;
	if (i == MAXSEARCHLIST - 2)
		{errno = ENOSPC; return -1;} /*no cabe*/
	if ((p = strdup(dir)) == NULL)
		return -1;
	searchlist[i] = p;
	searchlist[i+1] = NULL;
	return 0;
}

void search_list_new() {
	for (int i = 0; searchlist[i] != NULL; i++) {
		free(searchlist[i]);
		searchlist[i] = NULL;
	}
}

void search_list_show() {
	for (int i = 0; searchlist[i] != NULL; i++)
		printf ("%s\n",searchlist[i]);
}

void search_list_add_path() {
	char *aux;
	char *p;
	if ((p = getenv("PATH")) == NULL){
		printf ("unobtainable system PATH\n");
		return;
	}
	aux = strdup(p);
	if ((p = strtok (aux,":")) != NULL && search_list_add_dir(p) == -1)
		printf ("unable to add %s: %s\n", p, strerror(errno));
	while ((p = strtok(NULL,":")) != NULL)
		if (search_list_add_dir(p) == -1) {
			printf("unable to add %s: %s\n", p, strerror(errno));
			break;
		}
	free(aux);
}

char *find_executable(char *exec) {
	static char aux[MAXNAME];
	int i;
	struct stat s;
	if (exec == NULL)
		return NULL;
	if (exec[0] == '/' || !strncmp (exec,"./",2)
	|| !strncmp (exec,"../",2)) {
		if (stat(exec, &s) != -1)
			return exec;
		else
			return NULL;
	}
	for (i = 0; searchlist[i] != NULL; i++) {
		sprintf(aux,"%s/%s", searchlist[i], exec);
		if (stat(aux,&s) != -1)
			return aux;
	}
	return NULL;
}

void cmd_search_list(char *args[]) {
	if (args[0] == NULL)
		search_list_show();
	else if (args[0][0] == '+')
		search_list_add_dir(args[0] + 1);
	else if (strcmp(args[0],"-path") == 0)
		search_list_add_path();
	else 
		printf("%s\n", find_executable(args[0]));
}

void cmd_exec(char *args[]) {
	char *parmList[INPUTSIZE];
	char *prog;
	int i;
	for(i = 0; (args[i] != NULL && args[i][0] != '@'); i++)
		parmList[i] = args[i];
	if ((args[i] != NULL) && (args[i][0] == '@'))
		setpriority(PRIO_PROCESS, getpid(), atoi(args[i] + 1));
	if ((prog = find_executable(args[0])) == NULL)
		prog = args[0];
	execv(prog,parmList);
	perror("the program could not be found.");
}

void cmd_prog(char *args[]) {
	pid_t pid;
	if ((pid = fork()) == -1) {
		perror("Impossible to create child process");
		return;
	}
	if (pid) {
		waitpid(pid, NULL, 0);
		printf("entering father...\n");
	} else {
		printf("entering child process...\n");
		cmd_exec(args);
		exit(0);
	}
}

struct job_element {
	pid_t pid;
	int priority;
	char program[INPUTSIZE];  
	char parmstring[INPUTSIZE];
	time_t time;
	char *state;
	int sigretvalue;
} job_elem;

char *get_status(pid_t st, pid_t pid) {
	if (st == 0)
		return "ACTIVE";
	if (st == pid)
		return "STOPPED";
	else
		return "TERMINATED";
}

void cmd_background(char *args[]) {
	pid_t pid, status;
	int estado, i;
	char *prog;
	if ((pid = fork()) == -1) {
		perror("Impossible to create child process");
		return;
	}
	if (pid) {
		printf("entering father...\n");
 		struct job_element *elem = malloc(sizeof(job_elem));
		elem->pid = pid;
		elem->priority = getpriority(PRIO_PROCESS, pid);
		strcpy(elem->parmstring, "");
		for(i = 1; (args[i] != NULL); i++){
			strcat(elem->parmstring, args[i]);
			strcat(elem->parmstring, " ");
		}
		if ((prog = find_executable(args[0])) == NULL)
			perror("the program could not be found.");
		strcpy(elem->program, prog);
		elem->time = time(NULL);
		status = waitpid(pid, &estado,
			WNOHANG |WUNTRACED |WCONTINUED);
		elem->state = get_status(status, pid);
		if (strcmp(elem->state, "STOPPED") == 0) {
			elem->sigretvalue = estado;
		}
	//terminated normally, signaled or stopped
		insert(&l_jobs, elem);
	}
	else {
		printf("entering child process...\n");
		cmd_exec(args);
		exit(0);
	}
}

void cmd_jobs(char *args[]){
	pid_t status;
	int estado;
	struct job_element *elem;
	for (pos i = first(l_jobs); !(at_end(l_jobs,i)); i = next(i)) {
		elem = data(l_jobs, i);
		status = waitpid(elem->pid, &estado,
			WNOHANG |WUNTRACED |WCONTINUED);
		elem->state = get_status(status, elem->pid);
		printf("%d ", elem->pid);
		printf("%s ", elem->state);
		if (strcmp(elem->state, "STOPPED") == 0) {
			elem->sigretvalue = estado;
			if (WIFSIGNALED(estado))
				printf("%s ", signal_name(estado));
			else if (WIFEXITED(estado))
				printf("%d ", estado);
		}
		else if(strcmp(elem->state, "ACTIVE") == 0)
			printf(" ");
		else if (strcmp(elem->state, "ACTIVE") == 0)
			printf("255 ");
		printf("p=%d ", getpriority(PRIO_PROCESS,elem->pid));
		struct tm *tm = localtime(&(elem->time));
		char formatted_date[128]; // Buffer for strftime to work
		// Prints the date according to the current locale
		strftime(formatted_date, 128, "%a %b %d %Y %R", tm);	
		printf("%s ", formatted_date);
		printf("%s ", elem->program);
		printf("%s\n", elem->parmstring);
	}
}

void cmd_proc(char *args[]){
	pos i;
	pid_t status;
	int estado;
	if (args[0] == NULL) {
		cmd_jobs(args);
		return;
	}
	for(i = first(l_jobs); !(at_end(l_jobs,i)); i = next(i)) {
		struct job_element *elem = data(l_jobs, i);
		if (elem->pid == (pid_t) atoi(args[0])) {
			status = waitpid(elem->pid, &estado,
				WNOHANG |WUNTRACED |WCONTINUED);
			elem->state = get_status(status, elem->pid);
			printf("%d ", elem->pid);
			if (strcmp(elem->state, "STOPPED") == 0) {
				elem->sigretvalue = estado;
			if (WIFSIGNALED(estado))
				printf("%s ", signal_name(estado));
			else if (WIFEXITED(estado))
				printf("%d ", estado);
			}
			else if(strcmp(elem->state, "ACTIVE") == 0)
				printf("\n");
			printf("p=%d ", elem->priority);
			struct tm *tm = localtime(&(elem->time));
			char formatted_date[128]; // Buffer for strftime
			strftime(formatted_date, 128, "%a %b %d %Y %R", tm);	
			printf("%s ", formatted_date);
			printf("%s ", elem->state);
			printf("%s ", elem->program);
			printf("%s\n", elem->parmstring);
			return;
		}
		else continue;
	}
		cmd_jobs(args);
}

void cmd_pipe(char *args[]) {
	char *parmList1[INPUTSIZE], *parmList2[INPUTSIZE];
	char *prog1, *prog2;
	int i, j, fds[2]; 
	pid_t pid1, pid2;
	pipe(fds);
	if ((pid1 = fork()) == -1) {
		perror("Impossible to create child process");
		return;
 	 }
	if (pid1 == 0) {
		if ((prog1 = find_executable(args[0])) == NULL)
			perror("Cannot find program.");
		for(i = 0; (args[i] != NULL && args[i][0] != '%'); i++)
			parmList1[i] = args[i];
		dup2(fds[1], 1);
		close(fds[0]);
		close(fds[1]);
		execv(prog1, parmList1);
		exit(0);
	}
	if ((pid2 = fork()) == -1) {
		perror("Impossible to create child process");
		return;
	}
	if (pid2 == 0) {
		for(i = 0; (args[i] != NULL && args[i][0] != '%'); i++)
			continue;
		if ((args[i] != NULL) && (args[i][0] =='%')) {
			i++;
			if ((prog2 = find_executable(args[i])) == NULL)
				prog2 = args[i];
			for(j = i; (args[j] != NULL); j++)
				parmList2[j-i] = args[j];
		}
		dup2(fds[0], 0);
		close(fds[0]);
		close(fds[1]);
		execv(prog2, parmList2);
		exit(0);
	}
	close(fds[0]);
	close(fds[1]);
	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);
}

void cmd_clearjobs(char *args[]) {
	while (!(is_empty_list(l_jobs)))
		delete_elem(&l_jobs,first(l_jobs));	
}

/*
 * Command parsing
*/

int split_string(char *input, char *words[]) {
	int i = 1;
	// If the first word is empty
	if ((words[0] = strtok(input, " \n\t")) == NULL)
		return 0; // Then the number of words is 0
	while ((words[i] = strtok(NULL, " \n\t")) != NULL)
		i++;
	return i;
}

struct command {
	char *name;
	void (*pfunc)(char **);
};

void process_input(char *input) {
	// words is an array of pointers to the first char of a word in input
	char *words[INPUTSIZE / 2];
	int number_of_words;
	//this splits the input, calculates n
	if ((number_of_words = split_string(input, words)) == 0)
		return;
	static struct command cmds[] = {
		// Lab assignment 0
		{"authors", cmd_authors},
		{"pid", cmd_pid},
		{"fin", cmd_exit},
		{"end", cmd_exit},
		{"exit", cmd_exit},
		{"date", cmd_date},
		{"fecha", cmd_date},
		{"time", cmd_time},
		{"hora", cmd_time},
		{"chdir", cmd_chdir},
		// Lab assignment 1
		{"create", cmd_create},
		{"delete", cmd_delete},
		{"query", cmd_query},
		{"list", cmd_list},
		// Lab assignment 2
		{"allocate", cmd_allocate},
		{"deallocate", cmd_deallocate},
		{"rmkey", cmd_rmkey},
		{"mem", cmd_mem},
		{"memdump", cmd_dump},
		{"recursivefunction", cmd_recursive},
		{"read", cmd_read},
		{"write", cmd_write},
		// Lab assignment 3
		{"setpriority", cmd_set_priority},
		{"fork", cmd_fork},
		{"searchlist", cmd_search_list},
		{"exec", cmd_exec},
		{"prog", cmd_prog},
		{"background", cmd_background},
		{"jobs", cmd_jobs},
		{"proc", cmd_proc},
		{"clearjobs", cmd_clearjobs},
		{"pipe", cmd_pipe},
		{NULL, NULL}
	};
	// parse cmds list and compare it to words
	for (int i = 0; cmds[i].name != NULL; i++) {
		// if the first word in the list is the i-th cmd
		if (strcmp(cmds[i].name, words[0]) == 0) {
			// call that function, giving it access to the list
			// of arguments (the list of words starting at 1).
			(cmds[i].pfunc)(words + 1);
			return;
		}
	}
	if ((find_executable(words[0])!=NULL))
		cmd_prog(words);
	else 
		printf("Unknown command\n");
}

int main() {
	create_list(&allocations);
	create_list(&l_jobs);
	char input[INPUTSIZE];
	while (1) {
		printf("--> ");
		fgets(input, INPUTSIZE, stdin);
		process_input(input);
	}
}
