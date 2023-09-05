// court constants
#define	TOP_ROW		1
#define	BOT_ROW 	30
#define	LEFT_EDGE	1
#define	RIGHT_EDGE	119

// ball constants
#define	BLANK		' '
#define	DFL_SYMBOL	'o'
#define	B_X_INIT	60
#define	B_Y_INIT	15
#define	B_X_TTM		5
#define	B_Y_TTM		8

// paddle constants


/** the ping pong ball **/
struct ppball {
    int	y_pos, x_pos,
        y_ttm, x_ttm,
        y_ttg, x_ttg,
        y_dir, x_dir;
    char	symbol ;

};

/** the ping pong paddle**/