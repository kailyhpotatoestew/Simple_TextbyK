#include <unistd.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <stdio.h>
#include <errno.h> 
#include <termios.h>
#include <dirent.h> 

#define CTRL_KEY(k) ((k) & 0x1f)
#define POS(r, c)  do { printf("\x1b[%d;%dH",(r), (c)); } while(0)
#define SAV()  do { printf("\x1b[s"); } while(0);
#define RES() do { printf("\x1b[u"); } while(0);
#define CLEAR() do { printf("\x1b[2J"); } while(0);
#define CLEAR_L() do { printf("\x1b[1F;2K"); } while(0);
#define HIDE_CURSOR() do {printf("\x1b[?25l"); } while(0);
#define SHOW_CURSOR() do { printf("\x1b[?25h"); } while(0);

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


void read_current_dir(){ //prints list of all text files in the directory 
	DIR *current_dir = opendir("."); 

	int rw = 2; 
	int c = 50; 
	POS(rw, c);
	printf("\x1b[1;4m|Files in folder|");
	rw++; 
	struct dirent * read_dir; 
	for(int i = 0;;i++){
		POS(rw, c);
		read_dir = readdir(current_dir);
		if(read_dir == NULL){
			break;
		}
		if(strstr(read_dir->d_name, ".txt") != NULL){
			printf("|%s      ", read_dir->d_name); 
			rw++;  
		}
	} 
	printf("\x1b[0m");
}
void file_des(int row, int clm, char letter){
	SAV();
	POS(1, 1);
	printf("\x1b[1;30;47mCTRL+N - Quit | Position: %d|%d Letter: %c \x1b[39;49m", row, clm, letter);
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
	tcflush(STDIN_FILENO, TCIFLUSH);

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
		printf("\x1b[41;37mError: %d, %s\x1b[49;39m\nPress enter to continue", errno, strerror(errno));
		opt = cget(); 
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
		CLEAR();
		printf("\x1b[1;4;44mSucess!\x1b[49m\nDo you want:\n1 - to edit\n2 - rename\n3 - dupe\n4 - clear file?\n\x1b[22;24m");
		switch(opt = cget()){
			case '1': edit_file(fd); break; 
			case '2': duplicate_file(fd); break; 
			case '3': rename_file(pname); break;
			case '4': clear_file(fd); break;
		}
	}

	else{
		printf("\x1b[41;37mError: %d, %s\x1b[49;39m\nDo you want to overwrite file?(1/0): " , errno, strerror(errno)); 
		switch(opt = cget()){
		case '1': 
			if((p = fd = open(pname, O_RDWR | O_CREAT | O_TRUNC, 0644)) != -1){
				fd = open(pname, O_RDWR | O_CREAT | O_TRUNC, 0644); 
				fdatasync(fd); 
				CLEAR();
				printf("\x1b[1;4;44mSuccess!\x1b[49m\nDo you want:\n1 - to edit\n2 - rename\n3 - dupe\n4 - clear file?\n\x1b[22;24m");
				switch(opt = cget()){
					case '1': edit_file(fd); break; 
					case '2': duplicate_file(fd); break; 
					case '3': rename_file(pname); break;
					case '4': clear_file(fd); break;
				}
			} 
			else{
				printf("\x1b[41;37mError: %d, %s\x1b[49;39m\nDo you want to open a file instead or try again?(1/0) or 2 if u want to quit: ",errno, strerror(errno));
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
	char ebuff[RMAX / 124][124] = {}; //We will copy the contents of rbuff to ebuff to make a 2d array we can move around 
	int r2 = read(fd, rbuff, RMAX);
	if(r2 == -1) printf("Error %d: %s", errno, strerror(errno)); 
	int l = 0; //checks total characters
	int j = 0; //the row 
	int l2 = 0; //tracks characters in row 
	for(j = 0; l <= r2; j++){
		for(;;){
			if(rbuff[l] == '\n' || rbuff[l] == '\0'){ //Goes until it reaces a blank line 
				ebuff[j][l2] = '\0';
				l++;
				break;
			} 
			if(l2 >= 123) { //The terminal only has 123 columns, so we create a new line whenever there is more than 123 charactes in the line 
  			   ebuff[j][l2] = '\0';
 			   break;
			}
			ebuff[j][l2] = rbuff[l];  //copies rbuff into ebuff 

			l++; 
		    l2++;
		}
		l2 = 0;
	}
	int i_mode = 0;
	int pos = 2; 

	free_move();
	printf("Press enter to continue: ");
	i_mode = cget();
	
	POS(pos, 1);
	int ex = 1;
	

	while(1){
		CLEAR();
		POS(pos, 1);
		HIDE_CURSOR();
		if(row < 0) row = 0; 
		int f_row = row + 2;
		int f_clm = clm + 1;
		int dif = f_row - 29;
		if(dif <= 0) dif = 0;
		file_des(f_row, f_clm, ebuff[row][clm]);

		for(int z = dif; z < 28 + dif; z++){ //Scrolling logic 
		    printf("%s", ebuff[z]); // Prints file contents
			POS(pos + ex, 1);
			ex++;
		}
		ex = 1;
		
		POS(f_row, f_clm);
		SHOW_CURSOR();
		i_mode = getchar();

		if(i_mode == 27){
			getchar();
			switch(i_mode = getchar()){ //Cursor movement
			case 'A': row = (row + 2) > 2 ? row - 1: row - 0; break; //UP
           	case 'B': row = (row + 2) < j ? row + 1: row + 0; break;  //DOWN
           	case 'C':
           	 if((clm + 1) < strlen(ebuff[row])) clm = clm + 1;
           	 else{
           	 	row++;
           	 	clm = 0;
           	 } 
           	 break; //RIGHT
           	case 'D': 
           		if((clm + 1) > 1) clm--; 
           		else {
           			row--;
           			clm = strlen(ebuff[row]) - 1;
           		}
           		break; //LEFT
			}
			continue;
		}

		if(i_mode == CTRL_KEY('N')) break;

		if(i_mode == 127 || i_mode == '\b'){ //checks for backspace, moving back one and replaceing the current character with a blank space. If it is it at end of row, then it i will move up to the previous row 
				
				if((clm - 1) == -1){
					
					ebuff[row][clm] = ' ';
					if(row == 0){
						continue; 
					}

					row--;
					clm = 122; // sends you to end of previous row if you are at beginning of current row 
				
				}
				
				else {
					
					ebuff[row][clm] = ' ';
					clm--;
				
				}
				
				continue;
			
			} 
			
		else if(i_mode == 10 || i_mode == '\n'){
			row++;
			if(row > j) j++;
			clm = 0;
		}

		else {
			
			ebuff[row][clm] = i_mode; 
			if((clm + 1) < 123) clm++; //checks if u are at end of row, goes down a row if yo
           	
           	else{
           	 	
           	 	row++;
           	 	clm = 0;
           	
           	}
           	
   
		}
	}
	r_move();
	lseek(fd, 0, SEEK_SET);
	ftruncate(fd, 0);
	for (int y = 0; y < 1000; y++) {
   		if (strlen(ebuff[y]) == 0){
        	break;              // no more lines
   		}
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
