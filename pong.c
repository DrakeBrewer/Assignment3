#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include "bounce.h"

struct ppball the_Ball;

struct difficulty {int level; char *name;};
struct difficulty difficulties[] = {
    {0, "I'm Too Young To Die"},
    {1, "Hey, Not Too Rough"},
    {2, "Hurt Me Plenty"},
    {3, "Ultra-Violence"},
    {4, "Nightmare!"}
};

void setUp();
void wrapUp();
int setTicker(int);
int bounceOrLose(struct ppball *);
void ballMove(int);
void drawCourt();
void chooseDifficulty(int);

int main() {
    int c;

    setUp();

    while ((c = getchar()) != 'Q') {
        // if (c == 'f') the_Ball.x_ttm--;
        // else if (c == 's') the_Ball.x_ttm++;
        // else if (c == 'F') the_Ball.y_ttm--;
        // else if (c == 'S') the_Ball.y_ttm++;

        switch (c)
        {
            case 13:
                clear();
                drawCourt();
                difficultyMenu();
                chooseDifficulty(&c);
                refresh();
                break;
            
            default:
                break;
        }
    }

    wrapUp();
}

void setUp() {
    initscr();
    clear();

    drawCourt();
    drawPong();
    move(25,52);
    addstr("Press Enter to play");
    move(LINES-1,0);

    refresh();

    noecho();
    crmode();
}

void chooseDifficulty(int select) {
    int selectedDif = 0;
    int cursorPos = 12;
    
    mvaddch(cursorPos,50,'#');
    move(LINES-1,0);
    // refresh();

    switch (select)
    {
        case 'j':
            // if (cursorPos < 20)
            // {
                selectedDif++;
                mvaddch(cursorPos += 2,50,'#');
                refresh();
                move(LINES-1,0);
            // }
        case 'k':
            if (cursorPos > 12) 
            {
                selectedDif--;
                mvaddch(cursorPos -= 2,50,'#');
                refresh();
                move(LINES-1,0);
            }
        case 13:
            // selectDifficulty(selectedDif);
            break;
    }
}

void startGame() {
    void ballMove(int);

    the_Ball.y_pos = Y_INIT;
    the_Ball.x_pos = X_INIT;
    the_Ball.y_ttg = the_Ball.y_ttm = Y_TTM;
    the_Ball.x_ttg = the_Ball.x_ttm = X_TTM;
    the_Ball.y_dir = 1;
    the_Ball.x_dir = 1;
    the_Ball.symbol = DFL_SYMBOL;

    initscr();
    drawCourt();
    noecho();
    crmode();

    signal(SIGINT, SIG_IGN);
    mvaddch(the_Ball.y_pos, the_Ball.x_pos, the_Ball.symbol);
    refresh();

    signal(SIGALRM, ballMove);
    setTicker(1000/TICKS_PER_SEC);
}

void wrapUp() {
    setTicker(0);
    endwin();
}

void ballMove(int signum) {
    int y_cur, x_cur, moved;

    signal(SIGALRM, SIG_IGN);
    y_cur = the_Ball.y_pos;
    x_cur = the_Ball.x_pos;
    moved = 0;

    if (the_Ball.y_ttm > 0 && the_Ball.y_ttg-- == 1) {
        the_Ball.y_pos += the_Ball.y_dir;
        the_Ball.y_ttg += the_Ball.y_ttm;
        moved = 1;
    }

    if (the_Ball.x_ttm > 0 && the_Ball.x_ttg-- == 1) {
        the_Ball.x_pos += the_Ball.x_dir;
        the_Ball.x_ttg += the_Ball.x_ttm;
        moved = 1;
    }

    if (moved) {
        mvaddch(y_cur, x_cur, BLANK);
        mvaddch(y_cur, x_cur, BLANK);
        mvaddch(the_Ball.y_pos, the_Ball.x_pos, the_Ball.symbol);
        bounceOrLose(&the_Ball);
        refresh();
    }

    signal(SIGALRM, ballMove);
}

int bounceOrLose(struct ppball *bp) {
    int returnVal = 0;

    if ( bp->y_pos == TOP_ROW ){
		bp->y_dir = 1 ; 
		returnVal = 1 ;
	} else if ( bp->y_pos == BOT_ROW ){
		bp->y_dir = -1 ;
        returnVal = 1;
	}
	if ( bp->x_pos == LEFT_EDGE ){
		bp->x_dir = 1 ;
        returnVal = 1 ;
	} else if ( bp->x_pos == RIGHT_EDGE ){
		bp->x_dir = -1;
        returnVal = 1;
	}

    return returnVal;
}

int setTicker( int nMsecs )
{
    struct itimerval newTimeset;
    long    nSec, nUsecs;

    nSec = nMsecs / 1000 ;
    nUsecs = ( nMsecs % 1000 ) * 1000L;

    newTimeset.it_interval.tv_sec = nSec;
    newTimeset.it_interval.tv_usec = nUsecs;
    newTimeset.it_value.tv_sec = nSec;
    newTimeset.it_value.tv_usec = nUsecs;

	return setitimer(ITIMER_REAL, &newTimeset, NULL);
}


// Drawing functions
// ============================================

// draws the borders for the court
void drawCourt() {

    for (int ii = 1; ii < 31; ii++) {
        move(ii, 0);
        addstr("*");
    }
    for (int ii = 1; ii < 121; ii++) {
        move(0, ii);
        addstr("*");
        move(31, ii);
        addstr("*");
    }
}

void difficultyMenu() {
    move(8,52);
	addstr("Choose a Difficulty!");
    int yVal = 12;
    move(yVal,52);
    for (int ii=0; ii<=4; ii++) {
        mvaddstr(yVal,52,difficulties[ii].name);
        yVal+=2;  
    }
	move(LINES-1,0);
}

// draws the title of the game
void drawPong() {
    // P
    drawLine(10, 10, 31, "#", 1);
    drawLine(10, 10, 32, "#", 1);
    drawLine(11, 3, 40, "#", 1);
    drawLine(11, 3, 41, "#", 1);

    drawLine(31, 9, 10, "#", 0);
    drawLine(31, 9, 14, "#", 0);
    
    // O
    drawLine(11, 8, 45, "#", 1);
    drawLine(11, 8, 46, "#", 1);
    drawLine(11, 8, 56, "#", 1);
    drawLine(11, 8, 57, "#", 1);

    drawLine(47, 9, 10, "#", 0);
    drawLine(47, 9, 19, "#", 0);

    // N
    drawLine(10, 10, 61, "#", 1);
    drawLine(10, 10, 62, "#", 1);
    drawLine(10, 10, 72, "#", 1);
    drawLine(10, 10, 73, "#", 1);

    for (int ii=0; ii<10; ii++) {
        move(10+ii,63+ii);
        addstr("#");
    }

    // G
    drawLine(11, 8, 77, "#", 1);
    drawLine(11, 8, 78, "#", 1);
    
    drawLine(11, 2, 87, "#", 1);
    drawLine(11, 2, 88, "#", 1);

    drawLine(16, 3, 87, "#", 1);
    drawLine(16, 3, 88, "#", 1);

    drawLine(79, 9, 10, "#", 0);
    drawLine(79, 9, 19, "#", 0);
    
    drawLine(85, 2, 16, "#", 0);

}

// draws a verticle line using a specified string.
// verticle: dir = 1
// horizontal: dir = 0
void drawLine(int start, int len, int constCord, char *str, int dir) {

    if (dir == 1) {
        for (int ii = 0; ii < len; ii++) {
            move(start+ii,constCord);
            addstr(str);
        }
        refresh();
    } else if (dir == 0) {
        for (int ii = 0; ii < len; ii++) {
            move(constCord,start+ii);
            addstr(str);
        }
    }
}