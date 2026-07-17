#include "fileopp.h"

int r = 4;
int cl = 4;


void menu(){
	char op[7]; 
    printf("\x1b[0;1H");
    char text[3*MAX] = {
        "Press CTRL+N to quit \x1b[1;4;44m\n"
        "| Welcome to file Editor!|\n"
        "|         Options        |\n"
        "|  1 - Open a file       |\n"
        "|  2 - Create a file     |\n\x1b[0;49m\n"  
    };
    printf("%s", text);
    POS(r, cl);
    int c = 0;
    free_move();
    while(1){
        CLEAR();
        read_current_dir();
        POS(1, 1);
        printf("%s", text);
        POS(r, cl);
        c = getchar();
        if(c == 27){
            getchar();
            c = getchar();
            switch(c){
            case 'A': r = r > 4 ? r - 1: r - 0; break;
            case 'B': r = r <= 4? r + 1 : r + 0; break; 
            }
        }
        if(c == 10){
            POS(2, 1);
            switch(r){
            case 4: 
                r_move();
                open_file();
                getchar();
                free_move();
                break;
            case 5: 
                r_move();
                create_file();
                free_move();
                break;
            }
        }
        if(c == CTRL_KEY('N')) break;
    }
    r_move();
}
int main(){
    printf("\x1b[?1049h");
    menu();
    printf("\x1b[?1049l");
}
