#ifdef WIN32
#define _MSC_VER 1
#endif // WIN32

#include <OpenDoor.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define DOWN 1
#define UP 2
#define LEFT 3
#define RIGHT 4

#if _MSC_VER
#define snprintf _snprintf
#define strcasecmp _stricmp
#endif


struct user_info {
	char name[32];
	int wins;
	int losses;
};

struct user_info info;
int player_idx = -1;

int player_matrix[10][10];
int computer_matrix[10][10];

int computer_hits;
int player_hits;

void door_quit(void) {
	if(unlink("inuse.flg") != 0) {
		perror("unlink ");
	}
}

void save_player() {
#ifdef _MSC_VER	
	int fno = open("players.dat", O_WRONLY | O_CREAT | O_BINARY, 0644);
#else
	int fno = open("players.dat", O_WRONLY | O_CREAT, 0644);
#endif
	if (fno == -1) {
		od_exit(-1, FALSE);
	}
	lseek(fno, sizeof(struct user_info) * player_idx, SEEK_SET);
	
	write(fno, &info, sizeof(struct user_info));
	
	close(fno);
}

int get_player_idx(char *savefile) {
	FILE *fptr;
	
	char buffer[256];
	
	int idx = 0;
	fptr = fopen("players.idx", "r");
	
	if (fptr != NULL) {
		
		fgets(buffer, 256, fptr);
		
		while (!feof(fptr)) {
			if (strncmp(buffer, savefile, strlen(savefile)) == 0) {
				fclose(fptr);
				return idx;
			}
			idx++;
			fgets(buffer, 256, fptr);			
		}
		
		fclose(fptr);
	}
	fptr = fopen("players.idx", "a");
	if (!fptr) {
		fprintf(stderr, "ERROR OPENING players.idx!\n");
		od_exit(0, FALSE);
	}
	fprintf(fptr, "%s\n", savefile);
	
	return idx;
}

int load_player() {
	char savefile[256];
	FILE *fptr;
	
	snprintf(savefile, 255, "%s+%s", od_control_get()->user_name, od_control_get()->user_handle);
	
	player_idx = get_player_idx(savefile);
	
	fptr = fopen("players.dat", "rb");
	if (!fptr) {
		return 0;
	}
	
	fseek(fptr, sizeof(struct user_info) * player_idx, SEEK_SET);
	
	if (fread(&info, sizeof(struct user_info), 1, fptr) < 1) {
		fclose(fptr);
		return 0;
	}
	
	fclose(fptr);
	
	return 1;
}

void view_scores() {
	FILE *fptr;
	struct user_info inf;
	int lines = 0;
	fptr = fopen("players.dat", "rb");
	if (!fptr) {
		return;
	}
	
	fseek(fptr, sizeof(struct user_info) * player_idx, SEEK_SET);
	
	while (fread(&inf, sizeof(struct user_info), 1, fptr) == 1) {
		if (!(inf.wins == 0 && inf.losses == 0)) {
			od_printf("`bright yellow`%-32.32s `bright white`Wins: `bright green`%d `bright white`Losses: `bright red`%d\r\n", inf.name, inf.wins, inf.losses);
			if (lines == 23) {
				od_printf("`bright white`Press any key to continue...");
				od_get_key(TRUE);
				lines = 0;
			} else {
				lines++;
			}
				
		}
	}
	
	fclose(fptr);
	
	od_printf("`bright white`Press any key to continue...");
	od_get_key(TRUE);
	
	return;
}


void clear_console() {
	od_set_cursor(4, 50);
	od_clr_line();
	od_set_cursor(5, 50);
	od_clr_line();
	od_set_cursor(6, 50);
	od_clr_line();
	od_set_cursor(7, 50);
	od_clr_line();
	od_set_cursor(8, 50);
	od_clr_line();
	od_set_cursor(9, 50);
	od_clr_line();
}

int positionlegal(int x, int y, int direction, int length) {
	int i;
		
	if (direction == 1) {
		for (i=x;i<x+length;i++) {
			if (computer_matrix[i][y] == 1) {
				return 0;
			}
		}
	} else {
		for (i=y;i<y+length;i++) {
			if (computer_matrix[x][i] == 1) {
				return 0;
			}
		}
	}
	return 1;
}

void place_ship_computer(int length) {
	int direction;
	int max_y = 9;
	int max_x = 9;
	int x;
	int y;
	int i;
	direction = rand() % 2 + 1;
	
	if (direction == 1) {
		max_x = 9 - length;
	} else {
		max_y = 9 - length;
	}
	
	do {
		x = rand() % max_x;
		y = rand() % max_y;
	} while(!positionlegal(x, y, direction, length));
	
	if (direction == 1) {
		for (i=x;i<x+length;i++) {
			computer_matrix[i][y] = 1;
		}
	} else {
		for (i=y;i<y+length;i++) {
			computer_matrix[x][i] = 1;
		}		
	}
}

int place_ship(char *name, int length) {
	char ch_x;
	char ch_y;
	
	int fail = 0;
	
	int i;
	
	int x;
	int y;
	
	int pos_x;
	int pos_y;
	
	char direction;
	
	while (1) {

		clear_console();
	
		od_set_cursor(4, 50);
		od_printf("`bright white`Place your %s (%d)", name, length);
		od_set_cursor(6, 50);
		od_printf("  X: ");
		ch_x = od_get_answer("AaBbCcDdEeFfGgHhIiJjQq");
		if (tolower(ch_x) == 'q') {
			return 0;
		}
		od_printf("%c", ch_x);
		od_set_cursor(7, 50);
		od_printf("  Y: ");
		ch_y = od_get_answer("0123456789Qq");
		if (tolower(ch_y) == 'q') {
			return 0;
		}
		od_printf("%c", ch_y);
		od_set_cursor(8, 50);
		od_printf("H/V: ");
		direction = od_get_answer("HhVvQq");
		if (tolower(direction) == 'q') {
			return 0;
		}
		od_printf("%c", direction);
		x = tolower(ch_x) - 'a';
		y = ch_y - '0';
		
		fail = 0;
		
		if (tolower(direction) == 'h') {
			if (x <= 10 - length) {
				for (i=x; i< x + length;i++) {
					if (player_matrix[i][y] == 1) {
						fail = 1;
						break;
					}
				}
				
				if (!fail) {
					for (i=x; i< x + length;i++) {
						player_matrix[i][y] = 1;
					}
					od_printf("`white`");
					pos_y = y * 2 + 4;
					for (pos_x = x * 4 + 7;pos_x < ((x + length) * 4) + 4; pos_x++) {
						od_set_cursor(pos_y, pos_x);
						od_printf("\xDB");
					}
					return 1;
				}
			}
		} else {
			if (y <= 10 - length) {
				for (i=y; i< y + length;i++) {
					if (player_matrix[x][i] == 1) {
						fail = 1;
						break;
					}
				}
				
				if (!fail) {
					for (i=y; i< y + length;i++) {
						player_matrix[x][i] = 1;
					}
					od_printf("`white`");
					pos_x = x * 4 + 7;
					for (pos_y = y * 2 + 4;pos_y < ((y + length) * 2) + 3; pos_y++) {
						od_set_cursor(pos_y, pos_x);
						od_printf("\xDB");
					}
					return 1;
				}	
			}
		}
		
		od_set_cursor(9, 50);
		od_printf("`bright red`Error! Try Again (Press Enter)");
		od_get_key(TRUE);
	}
}


int initialize_game() {
	
	int i;
	int j;
	
	for (i=0;i<10;i++) {
		for (j=0;j<10;j++) {
			player_matrix[i][j] = 0;
			computer_matrix[i][j] = 0;
		}
	}
	
	computer_hits = 0;
	player_hits = 0;
	
	od_clr_scr();
	od_send_file("board.ans");
	
	if (!place_ship("Carrier", 5)) {
		return 0;
	}
	if (!place_ship("Battleship", 4)) {
		return 0;
	}
	if (!place_ship("Cruiser", 3)) {
		return 0;
	}
	if (!place_ship("Submarine", 3)) {
		return 0;
	}
	if (!place_ship("Destroyer", 2)) {
		return 0;
	}
	
	place_ship_computer(5);
	place_ship_computer(4);
	place_ship_computer(3);
	place_ship_computer(3);
	place_ship_computer(2);
	
	return 1;
}

void play_game() {
	int x;
	int y;
	int ch_x;
	int ch_y;
	
	int last_hit = 0;
	int last_hitx = 0;
	int last_hity = 0;
	int direction = 0;
	int dirswitch = 0;
	
	while (1) {
		clear_console();
		od_set_cursor(4, 50);
		od_printf("`bright white`Location to fire at?");
		od_set_cursor(6, 50);
		od_printf("  X: ");
		ch_x = od_get_answer("AaBbCcDdEeFfGgHhIiJjQq");
		if (tolower(ch_x) == 'q') {
			info.losses++;
			save_player();
			return;
		}
		od_printf("%c", ch_x);
		od_set_cursor(7, 50);
		od_printf("  Y: ");
		ch_y = od_get_answer("0123456789Qq");
		if (tolower(ch_y) == 'q') {
			info.losses++;
			save_player();
			return;
		}		
		od_printf("%c", ch_y);
		
		x = tolower(ch_x) - 'a';
		y = ch_y - '0';	
		
		if (computer_matrix[x][y] != 1 && computer_matrix[x][y] != 0) {
			od_set_cursor(9, 50);
			od_printf("`bright red`Error! Try Again (Press Enter)");
			od_get_key(TRUE);
			continue;
		}
		
		if (computer_matrix[x][y] == 1) {
			od_set_cursor(13+y, x * 2 + 55);
			od_printf("`bright red`X");
			computer_hits++;
		} else {
			od_set_cursor(13+y, x * 2 + 55);
			od_printf("`bright cyan`O");			
		}
		
		computer_matrix[x][y] = 2;
		
		if (computer_hits == 17) {
			clear_console();
			od_set_cursor(4, 50);
			od_printf("`bright green`You Win! (Press Enter)");
			od_get_answer("\r");
			info.wins++;
			save_player();
			return;
		}
do_loop:		
		if (last_hit == 0) {
			do {
				x = rand() % 10;
				y = rand() % 10;
			} while (player_matrix[x][y] == 2);
			if (player_matrix[x][y] == 1) {
				last_hit = 1;
				last_hitx = x;
				last_hity = y;
			}
		} else if (last_hit == 1) {
			do {
				do {
					x = last_hitx + rand() % 3 - 1;
					if (x == last_hitx) {
						y = last_hity + rand() % 3 - 1;
					} else {
						y = last_hity;
					}
				} while (x < 0 || x > 9 || y < 0 || y > 9);
			} while (player_matrix[x][y] == 2);
			if (player_matrix[x][y] == 1) {
				last_hit = 2;
				if (y > last_hity) {
					direction = DOWN;
				} else if (y < last_hity) {
					direction = UP;
				} else if (x > last_hitx) {
					direction = RIGHT;
				} else if (x < last_hitx) {
					direction = LEFT;
				}
			}
		} else {
			x = last_hitx;
			y = last_hity;
			while (player_matrix[x][y] == 2) {
				switch(direction) {
					case DOWN:
						y++;
						break;
					case UP:
						y--;
						break;
					case LEFT:
						x--;
						break;
					case RIGHT:
						x++;
						break;
				}
				
				if (y < 0 || x < 0 || y > 9 || x > 9) {
					if (dirswitch == 1) {
						dirswitch = 0;
						last_hit = 0;
					} else {
						switch(direction) {
							case DOWN:
								direction = UP;
								break;
							case UP:
								direction = DOWN;
								break;
							case LEFT:
								direction = RIGHT;
								break;
							case RIGHT:
								direction = LEFT;
								break;
						}
						dirswitch = 1;	
					}
					goto do_loop;
				}
			}
			
			if (player_matrix[x][y] == 0) {
				if (dirswitch) {
					dirswitch = 0;
					last_hit = 0;
				} else {
					switch(direction) {
						case DOWN:
							direction = UP;
							break;
						case UP:
							direction = DOWN;
							break;
						case LEFT:
							direction = RIGHT;
							break;
						case RIGHT:
							direction = LEFT;
							break;
					}
					dirswitch = 1;
				}
			}
		}
				
		if (player_matrix[x][y] == 1) {
			od_set_cursor(y * 2 + 4, x * 4 + 7);
			od_printf("`bright red`X");
			player_hits++;
		} else {
			od_set_cursor(y * 2 + 4, x * 4 + 7);
			od_printf("`bright cyan`O");
		}
		player_matrix[x][y] = 2;
		
		if (player_hits == 17) {
			clear_console();
			od_set_cursor(4, 50);
			od_printf("`bright red`You Lose! (Press Enter)");
			od_get_answer("\r");
			info.losses++;
			save_player();			
			return;
		}		
	}
}


#if _MSC_VER
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int nCmdShow)
{
#else
int main(int argc, char **argv) 
{
#endif

	char ch;
	struct stat s;
	FILE *fptr;

#if _MSC_VER
	od_parse_cmd_line(lpszCmdLine);
#else
	od_parse_cmd_line(argc, argv);
#endif
	
	od_init();

	srand(time(NULL));
	
	od_control_get()->od_page_pausing = FALSE;
	
	if (stat("inuse.flg", &s) == 0) {
		od_printf("\r\nSorry, the game is currently in use. Please try again later\r\nPress any key to continue...");
		od_get_key(TRUE);
		od_exit(0, FALSE);
	} else {
		fptr = fopen("inuse.flg", "w");
		if (!fptr) {
			fprintf(stderr, "Unable to open inuse.flg for writing!\n");
			od_exit(0, FALSE);
		}
		
		fprintf(fptr, "The game is currently in use!\n");
		fclose(fptr);
	}

	atexit(door_quit);

	if (!load_player()) {
		if (strlen(od_control_get()->user_handle) > 0) {
			snprintf(info.name, 32, "%s", od_control_get()->user_handle);
		} else {
			snprintf(info.name, 32, "%s", od_control_get()->user_name);
		}
		
		info.wins = 0;
		info.losses = 0;
		save_player();
	}

	do {
		od_clr_scr();
		od_send_file("intro.ans");

		ch = od_get_answer("PpSsIiQq");

		if (tolower(ch) == 'p') {
			if (initialize_game()) {
				play_game();
			}
		} else if (tolower(ch) == 's') {
			od_clr_scr();
			view_scores();
		} 
	} while(tolower(ch) != 'q');
 
	od_exit(0, FALSE);
}
