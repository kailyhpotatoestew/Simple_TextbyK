#include <unistd.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <stdio.h>
#include <errno.h> 
#include <termios.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define POS(r, c)  do { printf("\x1b[%d;%dH",(r), (c)); } while(0)
#define SAV()  do { printf("\x1b[s"); } while(0);
#define RES() do { printf("\x1b[u"); } while(0);
#define CLEAR() do { printf("\x1b[2J"); } while(0);
#define CLEAR_L() do { printf("\x1b[1F;2K"); } while(0);

#define WMAX 10000
#define MAX 100
#define RMAX 10000

int cget(void);
int open_file();
int create_file();

void edit_file(int fd);
void clear_file(int fd);
void duplicate_file(int fd);
void rename_file(char name[]);
void read_file(int fd);
void enable_raw_mode();
void disable_raw_mode();


void file_des(int row, int clm, char letter){
	SAV();
	POS(1, 1);
	printf("\x1b[1;30;47mCTRL+N- Quit Position: %d|%d Letter: %c \x1b[39;49m", row, clm, letter);
	RES();
}
//Exclusively for 1 character options
int cget(){
	int op = getchar();
	if(op == '\n') op = getchar();
	return op; 
}

void free_move() { //Allows for free cursor movement using arrow keys, and reads input without you needing to press enter
    struct termios raw;

    tcgetattr(STDIN_FILENO, &raw);    

    raw.c_lflag &= ~(ICANON | ECHO);  
    raw.c_cc[VMIN] = 1;                
    raw.c_cc[VTIME] = 0;              

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); 
}
void r_move() { //disables free_move();
    struct termios normal;
    tcgetattr(STDIN_FILENO, &normal);

    normal.c_lflag |= (ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &normal);
}

int open_file(){
	char name[MAX]; 
	int opt = 0;
	int fd = 0;

	CLEAR();
	POS(1,1);

	printf("\x1b[1mEnter File name:\x1b[22m ");
	scanf("%s", name);
	getchar(); 
	strcat(name, ".txt");
	printf("\x1b[1F\x1b[1;30;47mDo you want open for: Write only -> 1, or Read&Write->2:\x1b[22;39;49m "); 
	switch(opt = cget()){
		case '1':
			opt = O_WRONLY;
			break;
		case '2': opt = O_RDWR;
	} 
	CLEAR();
	if((fd = open(name, opt, 0644)) == -1){
		printf("Error: %d, %s", errno, strerror(errno));
		return 0;
	} 
	else{
		fdatasync(fd);

		printf("\x1b[1;4;44mFile opened!\x1b[49m\nDo you want:\n1 - to edit\n2 - rename\n3 - dupe\n4 - clear file?\n\x1b[22;24m");
		opt = cget();

		switch(opt){
		case '1': edit_file(fd); break;
		case '2': rename_file(name); break;
		case '3': duplicate_file(fd); break;
		case '4': clear_file(fd); break;
		}
	}	
}

int create_file(){
	CLEAR();
	POS(1,1);
	
	int fd = 0;
	int opt = 0; 
	int p = 0;
	
	char name[MAX]; 
	char pname[MAX];

	printf("\x1b[1mEnter File name:\x1b[22m");
	scanf("%s", name);
	getchar(); 
	strcat(name, ".txt");
	CLEAR_L();

	strcpy(pname, name);

	if((p = fd = open(pname, O_RDWR | O_CREAT | O_EXCL, 0644)) != -1){
		printf("Success! Do you want to edit file?(1/0): ");
		switch(opt = cget()){
		case '1': edit_file(fd); break;
		case '2': return fd;
		}
	}

	else{
		printf("Error: %d, %s\nDo you want to overwrite file?(1/0): " , errno, strerror(errno)); 
		switch(opt = cget()){
		case '1': 
			if((p = fd = open(pname, O_RDWR | O_CREAT | O_TRUNC, 0644)) != -1){
				fd = open(pname, O_RDWR | O_CREAT | O_TRUNC, 0644); 
				printf("Success! Do you want to edit file, duplicate, or rename?(1/2/3)");
				switch(opt = cget()){
					case '1': edit_file(fd); break; 
					case '2': duplicate_file(fd); break; 
					case '3': rename_file(pname); break;
				}
				;
			} 
			else{
				printf("Error: %d, %s\nDo you want to open a file instead or try again?(1/0) or 2 if u want to quit: ",errno, strerror(errno));
				switch(opt = cget()){
					case '1': open_file(); break;
					case '0': create_file(); break;
					case '2':return 0; 
				}
			}
		case '0': return fd; 
		}
	}
}

void edit_file(int fd){
	CLEAR();
	int m = 1; 
	int row = 0;
	int clm = 0;
	char rbuff[RMAX] = {};
	char input_t[WMAX] = {};
	char ebuff[1000][1000] = {};  
	int r2 = read(fd, rbuff, RMAX);
	if(r2 == -1) printf("Error %d: %s", errno, strerror(errno)); 
	int i = 1;
	int l = 0;
	int j = 0;
	int l2 = 0;
	for(j = 0; j <= r2; j++){
		for(;;){
			if(rbuff[l] == '\n' || rbuff[l] == '\0'){
				ebuff[j][l2] = '\0';
				l++;
			
				break;
			} 

			ebuff[j][l2] = rbuff[l]; 
			l++; 
		        l2++;
		}
		l2 = 0;
	}
	
	printf("%d", j);
	getchar();

	int i_mode = 0;
	int pos = 2; 

	free_move();
	
	POS(pos, 1);
	int ex = 1;
	

	while(1){
		CLEAR();
		POS(pos, 1);
		for(int z = 0; z <= j; z++){
			printf("%s", ebuff[z]); // Prints file contents
			POS(pos + ex, 1);
			ex++;
		}
		ex = 1;
		int f_row = row + 2;
		int f_clm = clm + 1;

		file_des(f_row, f_clm, ebuff[row][clm]);

		
		POS(f_row, f_clm);

		i_mode = getchar();

		if(i_mode == 27){
			getchar();
			switch(i_mode = getchar()){ //Cursor movement
			case 'A': row = (row + 2) >= 2 ? row - 1: row - 0; break;
           	case 'B': row = (row + 2) < j ? row + 1: row + 0; break; 
           	case 'C': clm = (clm + 1) <= strlen(ebuff[row]) ? clm  + 1 : clm + 0; break;
           	case 'D': clm = (clm + 1) >= 1 ? clm - 1: clm - 0; break;
			}
			continue;
		}

		if(i_mode == CTRL_KEY('N')) break;

		if(i_mode == 127 || i_mode == '\b'){
				if((clm - 1) == -1){
					row--;
					clm = strlen(ebuff[row]);
				}
				else {
					ebuff[row][clm] = ' ';
					clm--;
				}
				continue;
			} 
			
		else ebuff[row][clm] = i_mode;
			if(ebuff[row][clm] == 10 || ebuff[row][clm] == '\n'){
				row++;
				if(row > j) j++;
				clm = 0;
			}
			
			else clm++;
	}
	r_move();
	lseek(fd, 0, SEEK_SET);
	ftruncate(fd, 0);
	for (int y = 0; y < 1000; y++) {
   		if (ebuff[y][0] == '\0')
        	break;              // no more lines
   		int w2 = write(fd, ebuff[y], strlen(ebuff[y]));
    	if (w2 == -1) {
        	printf("Error %d: %s\n", errno, strerror(errno));
        	break;
  	    }

   		 write(fd, "\n", 1);
	}
	sync();
}

void clear_file(int fd){
	CLEAR();
	POS(1,1);
	ftruncate(fd, 0);
	printf("Successfully wiped file!");
}

void duplicate_file(int fd){
	CLEAR();
	POS(1,1);
	char name[MAX]; 
	char rbuff[RMAX]; 
	printf("\x1b[1mName your file:\x1b[22m"); 
	scanf("%s", name);
	strcat(name, ".txt");
	int ff = open(name, O_RDWR | O_CREAT, 0644); 
	lseek(fd,0,SEEK_SET);
	int rr = read(fd, rbuff, RMAX);
	write(ff, rbuff, rr);
	CLEAR();
}

void rename_file(char name[]){
	CLEAR();
	POS(1,1);
	char nname[MAX] = {}; 
	printf("Enter the new name: ");
	scanf("%s", nname);
	strcat(nname, ".txt");
	link(name, nname);
	unlink(name);
	CLEAR();
}
