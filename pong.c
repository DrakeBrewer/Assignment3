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

void startGame();
int bounceOrLose(struct ppball *);
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

// paddle stuff
int	pRow = 10;
int	pCol = 114;


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

            while (gameState == 1)
                pause();
        }

        // Actual game
        else if (gameState == 2) {
            startGame();
            while (gameState == 2)
                pause();
        }

        // Loss screen
        else if (gameState == 3) {
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
    enable_kbd_signals(); 

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
            signal(SIGINT, SIG_DFL);
            clear();
			gameState = 1;
			break;
	}
}

void difficultyScreen() {
    signal(SIGIO, difficultyInput);
    drawCourt();
    difficultyMenu();
    // move(difRow,difCol);
    mvaddch(difRow,difCol,'>');
    move(LINES-1,COLS-1);
    refresh();
}

void menuScreen() {
    signal(SIGIO, mmInput);
    drawCourt();
    drawPong();
    move(21,52);
    addstr("Press Enter to play");
    showControls(1);
}

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

void lossInput(int signum) {
    int c = getch();

    switch (c)
    {
        case 'Q':
            done = 1;
            break;
        case 'y':
            signal(SIGINT, SIG_DFL);
            clear();
            gameState = 2;
            break;
        case 'Y':
            signal(SIGINT, SIG_DFL);
            clear();
            gameState = 2;
            break;
        case 'n':
            done = 1;
            break;
        case 'N':
            done = 1;
            break;
    }
}

void pauseInput(int signum) {
    int c = getch();

    if (c == 27) {
        switch (selectedDif)
        {
            case 0:
                setTicker(1000/10);
                mvaddstr(15,55,"      ");
                showControls(0);
                refresh();
                break;
            case 1:
                setTicker(1000/50);
                mvaddstr(15,55,"      ");
                showControls(0);
                refresh();
                break;
            case 2:
                setTicker(1000/100);
                mvaddstr(15,55,"      ");
                showControls(0);
                refresh();
                break;
            case 3:
                setTicker(1000/150);
                mvaddstr(15,55,"      ");
                showControls(0);
                refresh();
                break;
            case 4:
                setTicker(1000/250);
                mvaddstr(15,55,"      ");
                showControls(0);
                refresh();
                break;

        }
    }
}

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
        addstr("Quit: Q");
        move(LINES-1,COLS-1);
        refresh();
    }
}

void lossScreen() {
    clear();
    signal(SIGIO, lossInput);
    drawCourt();
    mvaddstr(15,52,"GAME OVER");
    mvaddstr(18,48,"Play Again? (Y/n)");
    mvaddstr(20,53,"Quit: Q");
    move(LINES-1,COLS-1);
    refresh();
}

void startGame() {
    the_Ball.y_pos = Y_INIT;
    the_Ball.x_pos = X_INIT;
    the_Ball.y_ttg = the_Ball.y_ttm = Y_TTM;
    the_Ball.x_ttg = the_Ball.x_ttm = X_TTM;
    the_Ball.y_dir = 1;
    the_Ball.x_dir = 1;
    the_Ball.symbol = DFL_SYMBOL;

    // the_paddle.height = 5;
	// the_paddle.x_pos = PAD_X;
	// the_paddle.y_pos = PAD_Y_INIT;
	// the_paddle.y_pos = PAD_Y_INIT;
    score = 0;
	snprintf(scoreStr, 20, "%d", score);
	mvaddstr(0, 45, "Score ");
	mvaddstr(0, 52, scoreStr);
	for (int ii = 0; ii < 5; ii++) {
        mvaddstr(pRow+ii, pCol, "#");
    }

    drawCourt();


    signal(SIGINT, SIG_IGN);
    mvaddch(the_Ball.y_pos, the_Ball.x_pos, the_Ball.symbol);
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


void ballMove(int signum) {
    int y_cur, x_cur, moved;

    signal(SIGALRM, SIG_IGN);
    y_cur = the_Ball.y_pos;
    x_cur = the_Ball.x_pos;
    moved = 0;

    signal(SIGIO, paddleInput);
	enable_kbd_signals();

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
        move(LINES-1,COLS-1);
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
		score += 1;
        snprintf(scoreStr, 20, "%d", score);
		mvaddstr(0, 51, "Score: ");
        mvaddstr(0, 58, scoreStr);
        mvaddch(the_Ball.y_pos, the_Ball.x_pos, BLANK);
        the_Ball.y_pos = Y_INIT;
        the_Ball.x_pos = X_INIT;
        the_Ball.y_ttg = the_Ball.y_ttm = Y_TTM;
        the_Ball.x_ttg = the_Ball.x_ttm = X_TTM;
        the_Ball.y_dir = 1;
        the_Ball.x_dir = 1;
        if (score == 10) {
            gameState = 3;
            clear();
        }
        returnVal = 1;
	}
    else if ((bp->y_pos >= pRow && bp->y_pos < pRow+5) && bp->x_pos == pCol-1) {
		bp->x_dir = -1;
		returnVal = 1;
	}

    return returnVal;
}

void paddleInput(int signum) {
    int	c = getch();		  /* grab the char */

    switch (c) 
    {
        case 'Q':
            done = 1;
            break;
        case 27:
            setTicker(0);
            signal(SIGIO, pauseInput);
            mvaddstr(15,55,"Paused");
            showControls(1);
            refresh();
            break;
        case 'j':
            if (pRow > 1)
            {
                mvaddstr(pRow+4, pCol, " ");
                pRow--;
                for (int ii = 0; ii < 5; ii++) {
                    mvaddstr(pRow+ii, pCol, "#");
                }
                move(LINES-1,COLS-1);
                refresh();
                break;
            }
        case 'k':
            if (pRow < 25)
            {
                mvaddstr(pRow, pCol, " ");
                pRow++;
                for (int ii = 0; ii < 5; ii++) {
                    mvaddstr(pRow+ii, pCol, "#");
                }
                move(LINES-1,COLS-1);
                refresh();
                break;
            }
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