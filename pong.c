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

struct difficulty {int level; char *name; int value;};
struct difficulty difficulties[] = {
    {0, "I'm Too Young To Die.", 10},
    {1, "Hey, Not Too Rough.", 50},
    {2, "Hurt Me Plenty.", 100},
    {3, "Ultra-Violence.", 200},
    {4, "Nightmare!", 400}
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

// game globals
int done = 0;
int gameState = 0;

// difficulty globals
int selectedDif = 2;
int difCol = 50;
int difRow = 16;

// score globals
int score = 0;
char scoreStr[20];


int main() {
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

// *********************************************************************************
// Setup and wrapup functions
//     initialize and clear settings for
//     the curses functions
// *************************************
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

// *********************************************************************************
// Screens
//     functions to display the 
//     various game states that the 
//     user will see
// ********************************
void menuScreen() {
    signal(SIGIO, mmInput);
    drawCourt();
    drawPong();
    move(21,52);
    addstr("Press Enter to play");
    showControls(1);
}

void difficultyScreen() {

    if (signal(SIGIO, difficultyInput) == SIG_ERR) {
        gameState = 0;
    }
    drawCourt();
    difficultyMenu();
    mvaddch(difRow,difCol,'>');
    move(LINES-1,COLS-1);
    refresh();
}

void lossScreen() {
    setTicker(0);
    clear();
    signal(SIGIO, lossInput);
    drawCourt();
    mvaddstr(15,52,"GAME OVER");
    mvaddstr(18,48,"Play Again? (Y/n)");
    mvaddstr(20,40,"Play Again and change difficulty? (d)");
    move(LINES-1,COLS-1);
    refresh();
}

void mmInput(int signum) {
    int	c = getch();
	switch (c)
	{
		case 'Q':
			done = 1;
			break;
		case 10:
            signal(SIGINT, SIG_DFL);
            clear();
			gameState = 1;
			break;
	}
}

// *********************************************************************************
// Input handlers for signals
//     Functions that when combined with signals, 
//     handle the input at various states in the game.
// ***************************************************
void difficultyInput(int signum) {
    int c = getch();

    switch (c)
    {
        case 'Q':
			done = 1;
			break;
        case 10:
            signal(SIGINT, SIG_DFL);
            clear();
            gameState = 2;
            break;
        case 'j':
            if (difRow > 12)
            {
                selectedDif--;
                mvaddch(difRow, difCol, BLANK);
                difRow -= 2;
                mvaddch(difRow, difCol, '>');
                move(LINES-1,COLS-1);
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
                move(LINES-1,COLS-1);
                refresh();
                break;
            }
        
    }
}

void paddleInput(int signum) {
    int	c = getch();		  /* grab the char */

    switch (c) 
    {
        case 'Q':
            done = 1;
            break;
        case 'r':
            startGame(&the_Ball, &the_Paddle);
            break;
        case 27:
            setTicker(0);
            signal(SIGIO, pauseInput);
            mvaddstr(15,55,"Paused");
            showControls(1);
            refresh();
            break;

        case 'j':
            if (the_Paddle.y_pos > 1)
            {
                mvaddch(the_Paddle.y_pos+4, the_Paddle.x_pos, ' ');
                the_Paddle.y_pos--;
                for (int ii = 0; ii < 5; ii++) {
                    mvaddch(the_Paddle.y_pos+ii, the_Paddle.x_pos, '#');
                }
                move(LINES-1,COLS-1);
                refresh();
                break;
            }

        case 'k':
            if (the_Paddle.y_pos < 25)
            {
                mvaddch(the_Paddle.y_pos, the_Paddle.x_pos, ' ');
                the_Paddle.y_pos++;
                for (int ii = 0; ii < 5; ii++) {
                    mvaddch(the_Paddle.y_pos+ii, the_Paddle.x_pos, '#');
                }
                move(LINES-1,COLS-1);
                refresh();
                break;
            }
    }
}

void pauseInput(int signum) {
    int c = getch();

    if (c == 27) {
        mvaddstr(15,55,"      ");
        showControls(0);
        setDifficulty(selectedDif);
        refresh();
    }
    else if (c == 'Q') {
        done = 1;
    }
    else if (c == 'r') {
        startGame(&the_Ball, &the_Paddle);
    }
    else if (c == 'd') {
        clear();
        setTicker(0);
        gameState = 1;
    }
}

void lossInput(int signum) {
    int c = getch();
    signal(SIGINT, SIG_DFL);

    switch (c)
    {
        case 'y':
            clear();
            gameState = 2;
            break;
        case 'Y':
            clear();
            gameState = 2;
            break;
        case 'n':
            done = 1;
            break;
        case 'N':
            done = 1;
            break;
        case 'd':
            clear();
            setTicker(0);
            gameState = 1;
            break;
    }
}

// *********************************************************************************
// Game functions
//     Used for various functions in the game loop including 
//     but not limited to; ball trajectory, initializing
//     the ball and paddle, moving the ball, etc...
// *********************************************************
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


    for (int ii = 0; ii < pp->height; ii++) {
        mvaddch(pp->y_pos+ii, pp->x_pos, pp->symbol);
    }
    mvaddch(bp->y_pos, bp->x_pos, bp->symbol);
    refresh();

    signal(SIGALRM, ballMove);
    setDifficulty(selectedDif);
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

// *********************************************************************************
// Drawing functions
//    Makes the game nice to look at!
// **********************************
// display controls for the game
void showControls(int visible) {
    if (!visible) {
        move(23,55);
        addstr("         ");
        move(25,45);
        addstr("     ");
        move(27,45);
        addstr("       ");
        move(25,55);
        addstr("          ");
        move(27,55);
        addstr("          ");
        move(25,69);
        addstr("             ");
        move(27,69);
        addstr("       ");
        move(LINES-1,COLS-1);
        refresh();
    } else {    
        move(23,55);
        addstr("Controls:");
        move(25,45);
        addstr("Up: j");
        move(27,45);
        addstr("Down: k");
        move(25,55);
        addstr("Restart: r");
        move(27,55);
        addstr("Pause: Esc");
        move(25,69);
        addstr("Difficulty: d");
        move(27,69);
        addstr("Quit: Q");
        move(LINES-1,COLS-1);
        refresh();
    }
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

void difficultyMenu() {
    move(8,52);
	addstr("Choose Skill Level:");
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

// *********************************************************************************
// Helpers/Misc. functions
//     various functions that have functionality 
//     unique to themselves. Still crucial for 
//     the game to run.
// *********************************************
int updateScore(int count, char *str) {
    snprintf(str, 20, "%d", count);
    mvaddstr(0, 55, "Score: ");
    mvaddstr(0, 62, str);

    return count+=1;
}

void setDifficulty(int dif) {
    switch (dif)
    {
        case 0:
            setTicker(1000/difficulties[0].value);
            break;
        case 1:
            setTicker(1000/difficulties[1].value);
            break;
        case 2:
            setTicker(1000/difficulties[2].value);
            break;
        case 3:
            setTicker(1000/difficulties[3].value);
            break;
        case 4:
            setTicker(1000/difficulties[4].value);
            break;
    }
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