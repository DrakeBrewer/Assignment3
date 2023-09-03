#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include "bounce.h"

struct ppball the_Ball;

void setUp();
void wrapUp();
int setTicker(int);
int bounceOrLose(struct ppball *);
void ballMove(int);
void drawCourt();

int main() {
    int c;

    setUp();

    while ((c = getchar()) != 'Q') {
        if (c == 'f') the_Ball.x_ttm--;
        else if (c == 's') the_Ball.x_ttm++;
        else if (c == 'F') the_Ball.y_ttm--;
        else if (c == 'S') the_Ball.y_ttm++;
    }

    wrapUp();
}

void setUp() {
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