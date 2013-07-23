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

#include "curses.h"
#include <stdlib.h>
#include <time.h>

/* one can easily add new figures by adding his own ones to the list below.
 * there should be four forms (positions) for all the figures, order is how they turn clockwise.
 * figures are painted in 4x4 squares with ' ' and '#' symbols. ' ' means nothing, other symbol means figure.
 * don't forget about brackets and commas.
 * also one can comment out figures he doesn't like.
 * (desired block feature will work in unpredictable way if you comment out standard figures or change their order).
 * (do not comment out all the figures).
 * no code editing required!
 */
const char const blocks[][4][5][5]={
	// standard tetris figures:
	{ //O
		{
			"    ",
			" ## ",
			" ## ",
			"    "
		}, {
			"    ",
			" ## ",
			" ## ",
			"    "
		}, {
			"    ",
			" ## ",
			" ## ",
			"    "
		}, {
			"    ",
			" ## ",
			" ## ",
			"    "
		}
	}, { //S
		{
			" #  ",
			" ## ",
			"  # ",
			"    "
		}, {
			"    ",
			"  ##",
			" ## ",
			"    "
		}, {
			" #  ",
			" ## ",
			"  # ",
			"    "
		}, {
			"    ",
			"  ##",
			" ## ",
			"    "
		}
	}, { //Z
		{
			" #  ",
			"##  ",
			"#   ",
			"    "
		}, {
			"    ",
			"##  ",
			" ## ",
			"    "
		}, {
			" #  ",
			"##  ",
			"#   ",
			"    "
		}, {
			"    ",
			"##  ",
			" ## ",
			"    "
		}
	}, { //I
		{
			"    ",
			"    ",
			"#mma",
			"    "
		}, {
			" u  ",
			" l  ",
			" w  ",
			" u  "
		}, {
			"    ",
			"    ",
			"rff#",
			"    "
		}, {
			" #  ",
			" #  ",
			" #  ",
			" #  "
		}
	}, { //J
		{
			"    ",
			"#   ",
			"### ",
			"    "
		}, {
			"    ",
			" ## ",
			" #  ",
			" #  "
		}, {
			"    ",
			"    ",
			"### ",
			"  # "
		}, {
			"    ",
			" #  ",
			" #  ",
			"##  "
		}
	}, { //L
		{
			"  # ",
			"### ",
			"    ",
			"    "
		}, {
			" #  ",
			" #  ",
			" ## ",
			"    "
		}, {
			"    ",
			"### ",
			"#   ",
			"    "
		}, {
			"##  ",
			" #  ",
			" #  ",
			"    "
		}
	}, { //T
		{
			" #  ",
			"### ",
			"    ",
			"    "
		}, {
			" #  ",
			" ## ",
			" #  ",
			"    "
		}, {
			"    ",
			"### ",
			" #  ",
			"    "
		}, {
			" #  ",
			"##  ",
			" #  ",
			"    "
		}
	}
	//additional figures:
	/*, { //C
		{
			"    ",
			"### ",
			"# # ",
			"    "
		}, {
			"##  ",
			" #  ",
			"##  ",
			"    "
		}, {
			"# # ",
			"### ",
			"    ",
			"    "
		}, {
			" ## ",
			" #  ",
			" ## ",
			"    "
		}
	}*//*, { //+
		{
			" #  ",
			"### ",
			" #  ",
			"    "
		}, {
			" #  ",
			"### ",
			" #  ",
			"    "
		}, {
			" #  ",
			"### ",
			" #  ",
			"    "
		}, {
			" #  ",
			"### ",
			" #  ",
			"    "
		}
	}*/
};

enum { O, S, Z, I, J, L, T };

char map[24][17]={
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
	unsigned active;
	unsigned dir : 2;
	unsigned next : 5; //blocks
	unsigned next_dir : 2;
	unsigned fallen_flag : 1; //needed to compensate falling sometimes
	unsigned count : 2; //for not to fall everytime player presses a key
	unsigned score; //: 10; //nothing is impossible
	unsigned speed : 4; //0.1 seconds, delay=11-speed
	unsigned start_speed : 4;
	unsigned lazymode : 1;
} game;

int color(int in) { return (in%7)+1; } //colors of blocks. 7 is number of colors.

void upd_part() { //updates only current block position
	wstandend(window);
	//erase
	short i, j;
	for (i=game.x-1; i<=game.x+4; ++i)
	for (j=game.y-1; j<=game.y+3; ++j)
		if (map[j][i]==' ') mvwaddstr(window, j, 2*(i-2)-1, "  ");
	//print new
	wattrset(window, COLOR_PAIR(color(game.active)));
	for (i=0; i<4; ++i)
	for (j=0; j<4; ++j)
		if (blocks[game.active][game.dir][j][i]!=' ')
			mvwaddstr(window, game.y+j, 2*(game.x+i-2)-1, "##");
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
			if (blocks[game.next][game.next_dir][j][i]!=' ')
				mvwaddstr(info, j+2, 2*i+1, "##");
	} else
		mvwaddstr(info, 3, 3, "Lazy\n   Mode");
	wstandend(info);
	box(info, 0, 0);
	mvwaddstr(info, 0, 2, "Score:");
	mvwprintw(info, 1, 1, "%8d", game.score);
	mvwprintw(info, 6, 1, "Speed:%2d", game.speed);
	wrefresh(info);
}

void upd_all() { //update all map
	wmove(window, 1, 1);
	short i, j;
	for (j=1; j<21; ++j, waddstr(window, "\n "))
	for (i=3; i<13; ++i)
		if (map[j][i]!=' ') {
			wattrset(window, COLOR_PAIR(map[j][i]));
			waddstr(window, "##");
		} else {
			wstandend(window);
			waddstr(window, "  ");
		}
	wrefresh(window);
}

int check(int side, int bottom) { //check fitness of block position and a map
	short i, j;
	for (i=0; i<4; ++i)
	for (j=0; j<4; ++j)
		if (map[game.y+j+bottom][game.x+i+side]!=' ' &&
				blocks[game.active][game.dir][j][i]!=' ') return 0;
	return 1;
}

void clockwise()  { ++game.dir; } //turn
void aclockwise() { --game.dir; } //turn anticlockwise

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

void stop() { //make current active block a part of a map, remove lines if possible
	short i, j;
	for (i=0; i<4; ++i)
	for (j=0; j<4; ++j)
		if (blocks[game.active][game.dir][j][i]!=' ')
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
	short temp_score=game.score;
	game.score+=lines*lines;
	if (game.speed<10 && temp_score/10<game.score/10) {
		game.speed+=1+(game.score-temp_score)/10;
		if (game.speed>10) game.speed=10;
		halfdelay(11-game.speed);
	}
}

void fall_comp() { //compensate falling
	if (game.fallen_flag && game.count) --game.y;
	++game.count;
}

int next_rand() { return rand()%(sizeof(blocks)/sizeof(*blocks)); }

int check_to_up(int j, int i) { //check if column is free
	register int k;
	for (k=j; k>0; --k) if (map[k][i]!=' ') return 0;
	return 1;
}

int next_lasy() {
	struct {
		unsigned short height;
		unsigned short height_left;
		unsigned short height_right;
	} columns[10];

	unsigned short i;
	unsigned short min_height=20;
	unsigned short max_height=0;

	for (i=3; i<13; ++i) {
		unsigned short j;
		for (j=1; ' '==map[j][i-1]; ++j);
		columns[i-3].height_left=20+1-j;

		for (j=1; ' '==map[j][i]; ++j);
		columns[i-3].height=20+1-j;

		if ( min_height > 20+1-j )
			min_height=20+1-j;

		if ( max_height < 20+1-j )
			max_height=20+1-j;

		for (j=1; ' '==map[j][i+1]; ++j);
		columns[i-3].height_right=20+1-j;
	}

	unsigned short flag_random=0;
	unsigned short check_pit=0;
	for (i=0; i<10; ++i) {
		if (columns[i].height==min_height)
			++check_pit;
		else
			check_pit=0;

		if (4<=check_pit) {
			flag_random=1;
			break;
		}
	}

	if ( !max_height || flag_random)
		switch ( rand()%5 ) {
			case 0: return O;
			case 1: return T;
			case 2: return L;
			case 3: return J;
			case 4: return I;
	}

	unsigned short best_n;
	unsigned short max_dif=0;
	for (i=0; i<10; ++i)
		if (columns[i].height==min_height) {
			unsigned short min_neighb=( columns[i].height_left-columns[i].height < columns[i].height_right-columns[i].height ) ?
				columns[i].height_left-columns[i].height :
				columns[i].height_right-columns[i].height;
			if ( max_dif < min_neighb ) {
				best_n=i;
				max_dif=min_neighb;
			}
		}

	if ( max_dif >= 3 )
		return I;

	if ( max_dif==2 ) {
		if (columns[best_n].height_right-columns[best_n].height==2)
			return J;
		else
			return L;
	}

	if ( max_dif==1 ) {
		if (columns[best_n].height_right-columns[best_n].height==1)
			return ( rand()%2 ) ? Z : T;
		else
			return ( rand()%2 ) ? S : T;
	}

	return O; //max_dif==0
}

/*int next_lasy_old() { //returns number of the most suitable block
	//probabilities are disbalanced, but who cares?
	short i, j;
	short max_deep=0;
	short hole_x; //the right end of the pit
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
	switch (length) { //no breaks, it either returns  or falls to the next case
		case 4: if (check_to_up(max_deep, hole_x-3) &&
			    check_to_up(max_deep, hole_x-2) &&
			    check_to_up(max_deep, hole_x-1))
				switch (rand()%5) {
					case 0: return I;
					case 1: return O;
					case 2: return J;
					case 3: return L;
					case 4: return T;
				}
		case 3:
			if (check_to_up(max_deep, hole_x-2) &&
			    check_to_up(max_deep, hole_x-1)) {
				if (check_to_up(max_deep-1, hole_x+1))
					if (0==rand()%4) return S;

				if (check_to_up(max_deep-1, hole_x-3))
					if (0==rand()%4) return Z;

				switch (rand()%3) {
					case 0: return L;
					case 1: return T;
					case 2: return J;
				}
			}
		case 2:
			if (check_to_up(max_deep, hole_x-1)) {
				if (check_to_up(max_deep-1, hole_x-2))
					if (0==rand()%4) return Z;

				if (check_to_up(max_deep-1, hole_x+1))
					if (0==rand()%4) return S;

				switch (rand()%3) {
					case 0: return O;
					case 1: return L;
					case 2: return J;
				}
			}
		case 1:
			if (!check_to_up(max_deep-1, hole_x-1) &&
			     check_to_up(max_deep-2, hole_x-1) &&
			     check_to_up(max_deep-1, hole_x+1))
				switch (rand()%3) {
					case 0: return L;
					case 1: return Z;
					case 2: return T;
				}

			if (!check_to_up(max_deep-1, hole_x+1) &&
			     check_to_up(max_deep-2, hole_x+1) &&
			     check_to_up(max_deep-1, hole_x-1))
				switch (rand()%3) {
					case 0: return J;
					case 1: return S;
					case 2: return T;
				}

			if (check_to_up(max_deep-1, hole_x-1) &&
			    check_to_up(max_deep-1, hole_x+1))
				switch (rand()%3) {
					case 0: return T;
					case 1: return S;
					case 2: return Z;
				}

			if (check_to_up(max_deep-2, hole_x-1) &&
			    check_to_up(max_deep-2, hole_x+1))
				switch (rand()%3) {
					case 0: return L;
					case 1: return J;
				}

			return I;

		default: return next_rand();
	}
}*/

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
			if (!game.lazymode && game.score>=20) {
				game.active=next_lasy();
				game.score-=20;
			}
		break;
		case 'p': {
			fall_comp();
			wclear(window);
			box(window, 0, 0);
			mvwaddstr(window, 3, 7, "Pause");
			wrefresh(window);
			int ch=getch();
			while ('p'!=ch && 'Q'!=ch)
				ch=getch();
			if ('Q'==ch) {
				delwin(window);
				delwin(info);
				endwin();
				exit(0);
			}
			upd_all();
		} break;
		case KEY_DOWN: case ERR: break;
		default: fall_comp(); break;
	}
}

int main(int argc, char * argv[]) {
	printf("\nnicet\nCopyright (C) 2012 Alexander Kromm (mmaulwurff[at]gmail.com)\nThis program comes with ABSOLUTELY NO WARRANTY.\nThis is free software, and you are welcome to redistribute it\nunder certain conditions; see COPYING for details.\n\n");
	srand( (unsigned)time(NULL) );
	initscr(); //start ncurses screen
	start_color();
	noecho(); //for characters not to appear at the screen
	keypad(stdscr, TRUE); //access to additional keys
	curs_set(0); //not to show cursor
	game.score=0;
	game.count=0;
	game.start_speed=1;
	short i;
	for (i=1; i<argc; ++i) if (argv[i][0]=='-') {
		if (argv[i][1]=='l')
			game.lazymode=1;
		else if (argv[i][1]=='s') {
			short temp=atoi(argv[++i]);
			if (temp<11 && temp>0) game.start_speed=temp;
		}
	}
	game.speed=game.start_speed;
	halfdelay(11-game.speed); //game speed
	init_pair(1, COLOR_YELLOW,  COLOR_YELLOW);
	init_pair(2, COLOR_BLUE,    COLOR_BLUE);
	init_pair(3, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(4, COLOR_RED,     COLOR_RED);
	init_pair(5, COLOR_GREEN,   COLOR_GREEN);
	init_pair(6, COLOR_CYAN,    COLOR_CYAN);
	init_pair(7, COLOR_WHITE,   COLOR_WHITE);
	window=newwin(22, 22, 0,  0);
	info  =newwin( 7, 10, 0, 22);
	game.next=next_rand();
	game.next_dir=rand()%4;
	getch();
	while (1) {
		game.fallen_flag=0;
		if (game.lazymode)
			game.active=next_lasy();
		else {
			game.active=game.next;
			game.next=next_rand();
		}
		game.dir=game.next_dir;
		game.next_dir=rand()%4;
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
	delwin(window);
	delwin(info);
	endwin();
	printf("Game Over!\nYour score is %u.\n",
		game.score);
	return 0;
}
