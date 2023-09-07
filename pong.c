#include <stdio.h>
#include <ncurses.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include "bounce.h"

struct ppball the_Ball;
struct pppaddle the_Paddle;

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
int updateScore(int, char *);

void startGame(struct ppball *, struct pppaddle *);
int bounceOrLose(struct ppball *, struct pppaddle *);
void resetBall(struct ppball *);
void ballMove(int);

void menuScreen();
void difficultyScreen();
void lossScreen();

void mmInput(int);
void difficultyInput(int);
void paddleInput(int);
void lossInput(int);
void pauseInput(int);

void showControls(int);

void enable_kbd_signals();
void drawCourt();

int done = 0;
int gameState = 0;

int selectedDif = 2;
int difCol = 50;
int difRow = 16;

// score stuff
int score = 0;
char scoreStr[20];


int main() {
    // int c = getch();
    setUp();

    while (!done) {
        // Title screen
        if (gameState == 0) {
            menuScreen();
            pause();

        }

        // Difficulty screen
        else if (gameState == 1) {
            difficultyScreen();

            // while (gameState == 1)
            pause();
        }

        // Actual game
        else if (gameState == 2) {
            startGame(&the_Ball, &the_Paddle);
            while (gameState == 2 && !done)
                pause();
        }

        // Loss screen
        else if (gameState == 3) {
            clear();
            lossScreen();
            pause();
        }
    }

    wrapUp();
}

void setUp() {
    initscr();
    clear();
    noecho();
    crmode();
    signal(SIGINT, SIG_IGN);
    enable_kbd_signals();
}


void wrapUp() {
    signal(SIGINT, SIG_DFL);
    setTicker(0);
    endwin();
}

void startGame(struct ppball *bp, struct pppaddle *pp) {
    clear();
    resetBall(bp);
    resetPaddle(pp);

    score = 1;
	snprintf(scoreStr, 20, "%d", score);
	mvaddstr(0, 45, "Score ");
	mvaddstr(0, 52, scoreStr);
    refresh();

    drawCourt();


    // signal(SIGINT, SIG_IGN);
    for (int ii = 0; ii < pp->height; ii++) {
        mvaddch(pp->y_pos+ii, pp->x_pos, pp->symbol);
    }
    mvaddch(bp->y_pos, bp->x_pos, bp->symbol);
    refresh();

    signal(SIGALRM, ballMove);
    setDifficulty(selectedDif);
}

void setDifficulty(int dif) {
    switch (dif)
    {
        case 0:
            setTicker(1000/10);
            break;
        case 1:
            setTicker(1000/50);
            break;
        case 2:
            setTicker(1000/100);
            break;
        case 3:
            setTicker(1000/150);
            break;
        case 4:
            setTicker(1000/250);
            break;
    }
}

void resetPaddle(struct pppaddle *pp) {
    for (int ii = 0; ii < pp->height; ii++) {
        mvaddch(pp->y_pos+ii, pp->x_pos, BLANK);
    }
    pp->y_pos = P_Y_INIT;
    pp->x_pos = P_X_POS;
    pp->height = P_HEIGHT;
    pp->symbol = P_SYMBOL;
}

void resetBall(struct ppball *bp) {
    mvaddch(bp->y_pos, bp->x_pos, BLANK);
    bp->y_pos = B_Y_INIT;
    bp->x_pos = B_X_INIT;
    bp->y_ttg = bp->y_ttm = B_Y_TTM;
    bp->x_ttg = bp->x_ttm = B_X_TTM;
    bp->y_dir = 1;
    bp->x_dir = 1;
    bp->symbol = DFL_SYMBOL;
}


void ballMove(int signum) {
    int y_cur, x_cur, moved;

    // signal(SIGALRM, SIG_IGN);
    y_cur = the_Ball.y_pos;
    x_cur = the_Ball.x_pos;
    moved = 0;

    signal(SIGIO, paddleInput);

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
        bounceOrLose(&the_Ball, &the_Paddle);
        move(LINES-1,COLS-1);
        refresh();
    }

    signal(SIGALRM, ballMove);
}

int bounceOrLose(struct ppball *bp, struct pppaddle *pp) {
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
        score = updateScore(score, scoreStr);

        resetBall(bp);
        if (score == 11) {
            gameState = 3;
            clear();
        }
        returnVal = 1;
	}
    else if ((bp->y_pos >= pp->y_pos && bp->y_pos < (pp->y_pos)+5) && bp->x_pos == (pp->x_pos)-1) {
		bp->x_dir = -1;
		returnVal = 1;
	}

    return returnVal;
}

int updateScore(int count, char *str) {
    snprintf(str, 20, "%d", count);
    mvaddstr(0, 55, "Score: ");
    mvaddstr(0, 62, str);

    return count+=1;
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

void enable_kbd_signals()
{
	int  fd_flags;

	fcntl(0, F_SETOWN, getpid());
	fd_flags = fcntl(0, F_GETFL);
	fcntl(0, F_SETFL, (fd_flags|O_ASYNC));
}