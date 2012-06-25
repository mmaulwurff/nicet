/* nicet, nice tetris
 * Copyright (C) 2012  Alexander Kromm (mmaulwurff[at]gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <curses.h>
#include <stdlib.h>

char blocks[][4][4]={
	{//0
		"    ",
		" ## ",
		" ## ",
		"    "
	}, {//1
		" #  ",
		" ## ",
		"  # ",
		"    "
	}, {//2
		"    ",
		"  ##",
		" ## ",
		"    "
	}, {//3
		" #  ",
		"##  ",
		"#   ",
		"    "
	}, {//4
		"    ",
		"##  ",
		" ## ",
		"    "
	}, {//5
		"    ",
		"    ",
		"####",
		"    "
	}, {//6
		" #  ",
		" #  ",
		" #  ",
		" #  "
	}, {//7
		"    ",
		"#   ",
		"### ",
		"    "
	}, {//8
		"    ",
		" ## ",
		" #  ",
		" #  "
	}, {//9
		"    ",
		"    ",
		"### ",
		"  # "
	}, {//10
		"    ",
		" #  ",
		" #  ",
		"##  "
	}, {//11
		"  # ",
		"### ",
		"    ",
		"    "
	}, {//12
		" #  ",
		" #  ",
		" ## ",
		"    "
	}, {//13
		"    ",
		"### ",
		"#   ",
		"    "
	}, {//14
		"##  ",
		" #  ",
		" #  ",
		"    "
	}, {//15
		" #  ",
		"### ",
		"    ",
		"    "
	}, {//16
		" #  ",
		" ## ",
		" #  ",
		"    "
	}, {//17
		"    ",
		"### ",
		" #  ",
		"    "
	}, {//18
		" #  ",
		"##  ",
		" #  ",
		"    "
	}
};

char map[24][16]={
	"################",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###",
	"###          ###", //20
	"################",
	"################",
	"################",
};

WINDOW * window, * info;
struct {
	unsigned x : 4; //left upper corner of active block
	unsigned y : 5;
	unsigned active : 5;
	unsigned next : 5; //blocks
	unsigned fallen_flag : 1; //needed to compensate falling sometimes
	unsigned count : 2; //for not to fall everytime player presses a key
	unsigned score : 10;
	unsigned delay : 4; //0.1 seconds, speed=11-delay
	unsigned lazymode : 1;
} game;

int color(int in) { //colors of blocks. used in wattrset
	if (in==0) return 1;
	else if (in<3)  return 2;
	else if (in<5)  return 3;
	else if (in<7)  return 4;
	else if (in<11) return 5;
	else if (in<15) return 6;
	return 7;
}

void upd_part() { //updates only current block position
	wstandend(window);
	//erase
	short i, j;
	for (i=game.x-1; i<=game.x+4; ++i)
	for (j=game.y-1; j<=game.y+3; ++j)
		if (map[j][i]==' ') mvwprintw(window, j, 2*(i-2)-1, "  ");
	//print new
	wattrset(window, COLOR_PAIR(color(game.active)));
	for (i=0; i<4; ++i)
	for (j=0; j<4; ++j)
		if (blocks[game.active][j][i]=='#')
			mvwprintw(window, game.y+j, 2*(game.x+i-2)-1, "##");
	wstandend(window);
	box(window, 0, 0);
	wrefresh(window);
}

void print_next() { //next block preview
	werase(info);
	if (!game.lazymode) {
		wattrset(info, COLOR_PAIR(color(game.next)));
		short i, j;
		for (i=0; i<4; ++i)
		for (j=0; j<4; ++j)
			if (blocks[game.next][j][i]=='#')
				mvwprintw(info, j+1, 2*i+1, "##");
	} else
		mvwprintw(info, 2, 3, "Lazy\n   Mode");
	wstandend(info);
	box(info, 0, 0);
	mvwprintw(info, 0, 0, "Score:%4d", game.score);
	mvwprintw(info, 5, 1, "Speed:%2d", 11-game.delay);
	wrefresh(info);
}

void upd_all() { //update all map
	wmove(window, 1, 1);
	short i, j;
	for (j=1; j<21; ++j, wprintw(window, "\n "))
	for (i=3; i<13; ++i)
		if (map[j][i]!=' ') {
			wattrset(window, COLOR_PAIR(map[j][i]));
			wprintw(window, "##");
		} else {
			wstandend(window);
			wprintw(window, "  ");
		}
	wrefresh(window);
}

int check(int side, int bottom) { //check fitness of block position and a map
	short i, j;
	for (i=0; i<4; ++i)
	for (j=0; j<4; ++j)
		if (map[game.y+j+bottom][game.x+i+side]!=' ' &&
				blocks[game.active][j][i]=='#') return 0;
	return 1;
}

int clockwise() { //turn
	switch (game.active) {
		case  1: game.active=2;  break;
		case  2: game.active=1;  break;

		case  3: game.active=4;  break;
		case  4: game.active=3;  break;

		case  5: game.active=6;  break;
		case  6: game.active=5;  break;

		case  7: game.active=8;  break;
		case  8: game.active=9;  break;
		case  9: game.active=10; break;
		case 10: game.active=7;  break;

		case 11: game.active=12; break;
		case 12: game.active=13; break;
		case 13: game.active=14; break;
		case 14: game.active=11; break;

		case 15: game.active=16; break;
		case 16: game.active=17; break;
		case 17: game.active=18; break;
		case 18: game.active=15; break;
		default: break;
	}
}

int aclockwise() { //turn anticlockwise
	switch (game.active) {
		case 1:  game.active=2;  break;
		case 2:  game.active=1;  break;

		case 3:  game.active=4;  break;
		case 4:  game.active=3;  break;

		case 5:  game.active=6;  break;
		case 6:  game.active=5;  break;

		case 7:  game.active=10; break;
		case 8:  game.active=7;  break;
		case 9:  game.active=8;  break;
		case 10: game.active=9;  break;

		case 11: game.active=14; break;
		case 12: game.active=11; break;
		case 13: game.active=12; break;
		case 14: game.active=13; break;

		case 15: game.active=18; break;
		case 16: game.active=15; break;
		case 17: game.active=16; break;
		case 18: game.active=17; break;
		default: break;
	}
}

int check_clock() { //check clockwise turn possibility
	clockwise();
	short ret=check(0, 0);
	aclockwise();
	return ret;
}

int check_aclock() { //check anticlockwise turn possibility
	aclockwise();
	short ret=check(0, 0);
	clockwise();
	return ret;
}

inline stop() { //make current active block a part of a map, remove lines if possible
	short i, j;
	for (i=0; i<4; ++i)
	for (j=0; j<4; ++j)
		if (blocks[game.active][j][i]=='#')
			map[game.y+j][game.x+i]=color(game.active);
	//check lines
	short lines=0, flag;
	for (j=1; j<21;  ++j) {
		flag=1;
		for (i=3;  i<13; ++i) if (map[j][i]==' ') {
			flag=0;
			break; //line not complete
		}
		if (flag) { //line complete
			short k;
			++lines;
			for (i=j; i>2; --i)
			for (k=3; k<13; ++k)
				map[i][k]=map[i-1][k];
		}
	}
	game.score+=lines*lines;
	if (game.score>10 && game.delay>1){
		game.delay=1+(100-game.score)/10;
		nocbreak();
		halfdelay(game.delay);
	}
}

void fall_comp() { //compensate falling
	if (game.fallen_flag && game.count) --game.y;
	++game.count;
}

int rand_num() { //random number
	FILE * file=fopen("/dev/urandom", "r");
	int temp=fgetc(file);
	fclose(file);
	return temp;
}

int rand_active() { //random block
	short temp=rand_num()%21;
	if (temp>18) return 0; //for 'O' block to spawn more often
	return temp;
}

int check_to_up(int j, int i) { //check if column is free
	register k;
	for (k=j; k>0; --k) if (map[k][i]!=' ') return 0;
	return 1;
}

int next_easy() { //returns number of the most suitable block
	short i, j;
	short max_deep=0, hole_x;
	for (i=3; i<13; ++i) {
		for (j=0; map[j+1][i]==' '; ++j);
		if (j>=max_deep) {
			max_deep=j;
			hole_x=i;
		}
	}
	short length=0;
	while (map[max_deep][hole_x-length]==' ') ++length;
	//now we have everything to determine the best block
	switch (length) { //no breaks, ih either returns  or falls to the next case
		case 4: if (check_to_up(max_deep, hole_x-3) &&
			    check_to_up(max_deep, hole_x-2) &&
			    check_to_up(max_deep, hole_x-1))
				return 5+rand_num()%2; //I blocks
		case 3:
			if (check_to_up(max_deep, hole_x-2) &&
			    check_to_up(max_deep, hole_x-1))
				return 7+rand_num()%8; //J, L blocks
		case 2:
			if (check_to_up(max_deep, hole_x-1)) {
				if (check_to_up(max_deep-1, hole_x-2) &&
				    check_to_up(max_deep-1, hole_x+1))
					return rand_num()%5; //O, S, Z
				else if (check_to_up(max_deep-1, hole_x-2))
					return 3+rand_num()%2; //Z
				else if (check_to_up(max_deep-1, hole_x+1))
					return rand_num()%3; //S, O
			}
		case 1:
			if (check_to_up(max_deep-1, hole_x-1) ||
			    check_to_up(max_deep-1, hole_x+1)) return 15+rand_num()%4; //T
			else if (check_to_up(max_deep-2, hole_x-1))
				return 11+rand_num()%4; //L
			else if (check_to_up(max_deep-2, hole_x+1))
				return 7+rand_num()%4; //J
			else return 5+rand_num()%2; //I

		default: return rand_active();
	}
}

void key_act() { //get key and do something with it
	int c=getch();
	switch (c) {
		case 'Q': delwin(window); delwin(info); endwin(); exit(0); break;
		case KEY_LEFT : if (check(-1, 0)) --game.x; fall_comp(); break;
		case KEY_RIGHT: if (check( 1, 0)) ++game.x; fall_comp(); break;
		case KEY_UP:
			clockwise();
			if (!check(0, 0)) aclockwise();
			fall_comp();
		break;
		case ' ':
			aclockwise();
			if (!check(0, 0)) clockwise();
			fall_comp();
		break;
		case 'b':
			fall_comp();
			if (!game.lazymode && game.score>21) {
				game.active=next_easy();
				game.score-=20;
			}
		break;
		case 'p':
			fall_comp();
			wclear(window);
			box(window, 0, 0);
			mvwprintw(window, 3, 7, "Pause");
			wrefresh(window);
			while ('p'!=getch());
			upd_all();
		break;
		case KEY_DOWN: case ERR: break;
		default: fall_comp(); break;
	}
}

main(int argc, char * argv[]) {
	printf("\nnicet\nCopyright (C) 2012 Alexander Kromm (mmaulwurff[at]gmail.com)\nThis program comes with ABSOLUTELY NO WARRANTY.\nThis is free software, and you are welcome to redistribute it\nunder certain conditions; see COPYING for details.\n\n");
	initscr(); //start ncurses screen
	start_color();
	noecho(); //for characters not to appear at the screen
	keypad(stdscr, TRUE); //access to additional keys
	curs_set(0); //not to show cursor
	game.score=0;
	game.count=0;
	game.delay=10;
	short i;
	for (i=1; i<argc; ++i) if (argv[i][0]=='-') {
		if (argv[i][1]=='l')
			game.lazymode=1;
		else if (argv[i][1]=='s') {
			short temp=atoi(argv[++i]);
			if (temp<11 && temp>0) game.delay=11-temp;
		}
	}
	halfdelay(game.delay); //game speed
	init_pair(1, COLOR_YELLOW,  COLOR_YELLOW);
	init_pair(2, COLOR_BLUE,    COLOR_BLUE);
	init_pair(3, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(4, COLOR_RED,     COLOR_RED);
	init_pair(5, COLOR_GREEN,   COLOR_GREEN);
	init_pair(6, COLOR_CYAN,    COLOR_CYAN);
	init_pair(7, COLOR_WHITE,   COLOR_WHITE);
	window=newwin(22, 22, 0,  0);
	info  =newwin( 6, 10, 0, 22);
	game.next=rand_active();
	getch();
	while (1) {
		game.fallen_flag=0;
		if (game.lazymode)
			game.active=next_easy();
		else {
			game.active=game.next;
			game.next=rand_active();
		}
		print_next();
		game.x=6;
		game.y=1;
		if (!check(0, 0)) break;
		upd_part();
		while (check(0, 1) || check(-1, 0) || check(1, 0) || check_clock() || check_aclock()) {
			key_act();
			if (check(0, 1)) {
				++game.y;
				game.fallen_flag=1;
			} else {
				game.fallen_flag=0;
				break;
			}
			upd_part();
		}
		stop();
		upd_all();
	}
	wstandend(window);
	box(window, 0, 0);
	mvwprintw(window, 0, 1, "Game Over!");
	wrefresh(window);
	while (getch()!='Q');
	delwin(window);
	delwin(info);
	endwin();
}
