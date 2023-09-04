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
void enable_kbd_signals();
void movePaddle(int);
void mmInput(int);

int done = 0;
int gameState = 0;
int score = 0;

int selectedDif = 0;
int difCol = 50;
int difRow = 16;


int main() {
    // int c = getch();
    setUp();


    while (!done) {
        // Title screen
        if (gameState == 0) {
            clear();
            signal(SIGIO, mmInput);
            enable_kbd_signals();
            drawCourt();
            drawPong();
            move(21,52);
            addstr("Press Enter to play");
            move(23,55);
            addstr("Controls:");
            move(25,45);
            addstr("Up: j");
            move(27,45);
            addstr("Down: k");
            move(25,55);
            addstr("Restart: r");
            move(27,55);
            addstr("Pause: p");
            move(25,69);
            addstr("Main Menu: M");
            move(27,69);
            addstr("Quit: Q");
            move(LINES-1,0);
            refresh();
            pause();

        }

        // Difficulty screen
        else if (gameState == 1) {
            clear();
            signal(SIGIO, chooseDifficulty);
            enable_kbd_signals();
            drawCourt();
            difficultyMenu();
            move(difRow,difCol);
            addch('>');
            refresh();

            while (gameState == 1 && !done)
                pause();
        }

        // Actual game
        else if (gameState == 2) {
            // clear();
            // signal(SIGIO, playGame);
            // drawCourt();
            // drawPaddle();
            // startGame();


        }

        // Pause screen
        else if (gameState == 3) {

        }
    }

    wrapUp();
}

void setUp() {
    initscr();
    clear();
    noecho();
    crmode();
    // enable_kbd_signals(); 

}


void wrapUp() {
    setTicker(0);
    endwin();
}

void mmInput(int signum) {
    int	c = getch();
	switch (c)
	{
		case 'Q':
			done = 1;
			break;
		case 10:
			gameState = 1;
			break;
		case 13:
			gameState = 1;
			break;
	}
}

void chooseDifficulty(int signum) {
    int c = getch();

    switch (c)
    {
        case 'Q':
			done = 1;
			break;
        case 'M':
			gameState = 0;
			break;
        case 10:
            gameState = 2;
            break;
        case 'j':
            if (difRow > 12)
            {
                selectedDif--;
                mvaddch(difRow, difCol, BLANK);
                difRow -= 2;
                mvaddch(difRow, difCol, '>');
                refresh();
                break;
            }
        case 'k':
            if (difRow < 20) 
            {
                selectedDif++;
                mvaddch(difRow, difCol, BLANK);
                difRow += 2;
                mvaddch(difRow, difCol, '>');
                refresh();
                break;
            }
        
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

// ============================================
// Signal handling
// ============================================
void enable_kbd_signals()
{
	int  fd_flags;

	fcntl(0, F_SETOWN, getpid());
	fd_flags = fcntl(0, F_GETFL);
	fcntl(0, F_SETFL, (fd_flags|O_ASYNC));
}


// ============================================
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
        mvaddstr(yVal, 52, difficulties[ii].name);
        yVal += 2;
    }
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