#include <iostream>
#include <list>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#if defined(_WIN32)

#include <conio.h>

#elif defined(__linux__) || defined(__APPLE__)

#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

struct termios tty_settings;

int kbhit() //Retorna 0 se nada foi teclado
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch() //Lê o que foi teclado
{
    int r;
    unsigned char c;
    if ((r = read(STDIN_FILENO, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

#endif

int TTY = -1;
int tty_available()
{
    if (TTY < 0)
	{
#if defined(__linux__) || defined(__APPLE__)
        TTY = isatty(STDOUT_FILENO) && isatty(STDIN_FILENO);
#else
        TTY = 0;
#endif
    }
    return TTY;
}

void disable_getch()  //Retorna para o terminal original // reset_terminal_mode
{
#if defined(__linux__) || defined(__APPLE__)
    if(tty_available()) tcsetattr(STDIN_FILENO, TCSANOW, &tty_settings);
#endif
}

void enable_getch()  //Configura terminal especial // set_conio_terminal_mode
{
#if defined(__linux__) || defined(__APPLE__)
	if(tty_available())
	{
		struct termios newattr;

		/* take two copies - one for now, one for later */
		tcgetattr(STDIN_FILENO, &tty_settings);
		memcpy(&newattr, &tty_settings, sizeof(newattr));

		/* register cleanup handler, and set the new terminal mode */
		atexit(disable_getch);
		newattr.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
	}
#endif
}

void clearScreen()
{
#if defined(__linux__) || defined(__APPLE__)
    if (tty_available()) {
        fputs("\x1b[2J\x1b[1;1H", stdout);
    }
#endif
}

#define ALTURA 62
#define LARGURA 113
#define DELAY 0.1
#define TRAIL_SIZE ((ALTURA+LARGURA)/4)

#define KEEPWALK 1
#define AUTOWALK 1
#define AUTOGROW 0
#define INITSIZE 0

#define WALL_CHAR  '#'
#define HEAD_CHAR  '@'
#define BODY_CHAR  'S'
#define TRAIL_CHAR '.'
#define EMPTY_CHAR ' '
#define COIN_CHAR  'O'

using namespace std;

struct coord
{
    coord(int X, int Y)
    {
        this->x=X;
        this->y=Y;
    }
    int x, y;
};

static const char commands[] = {'w','a','s','d'};

bool operator==(coord A, coord B)
{
    return A.x==B.x && A.y==B.y;
}

template <class T> void printAt(int Y, int X, T C)
{
    cerr << "\x1b[" << (Y) << ";" << (X) << "H" << (C);
}

void printTableBorder(int xBase, int yBase, int height = ALTURA, int width = LARGURA, char character = WALL_CHAR)
{
        for(int j = 0; j < width + 2; j++)
        {
            printAt(2,1+j*2,character);
            printAt(3+height,1+j*2,character);
        }
        for(int i = 0; i <= height; i++)
        {
            printAt(2+i,1,character);
            printAt(2+i,1+2*(1+width),character);
        }
}

int main()
{
    srand(time(NULL));

    int I(ALTURA/2), J(LARGURA/2);
    coord head(I,J), coin(0,0), clear(0,0);

    list<coord> body(INITSIZE, head), trail;
    char input('\0');
    bool fim(false);

	int size(1+body.size()), t_counter(0);
    bool grow(false);
    bool coinRequest(true);

#if KEEPWALK
    double Tnow(double(clock())/CLOCKS_PER_SEC);
    double Tbefore(Tnow);
#endif

	enable_getch();
    cerr << "\x1b[?25l";
    clearScreen();
    printTableBorder(5,5);

    while(!fim)
	{
        t_counter++;
#if AUTOGROW
        if(((t_counter)%100)==0)
        {
            size++;
            grow = true;
        }
#endif

#if AUTOWALK
        if(t_counter%1==0)
        {
            char R('\0');
            bool r(false);
            do
            {
                r = false;
                R = commands[rand()%3+1];
                r |= (R=='a' && input=='d');
                r |= (R=='d' && input=='a');
                r |= (R=='w' && input=='s');
                r |= (R=='s' && input=='w');
            }
            while(r);
            input = R;
        }
#endif

#if KEEPWALK
        while(kbhit()==0 && Tnow-Tbefore<DELAY) Tnow = double(clock())/CLOCKS_PER_SEC;
        Tbefore = Tnow;
#else
        while(kbhit()==0);
#endif
        if(kbhit()) input = getch();

        switch(input)
        {
            case 'w':
            case 'W':
                I--;    //Sobe
                break;
            case 's':
            case 'S':
                I++;    //Desce
                break;
            case 'a':
            case 'A':
                J--;
                break;
            case 'd':
            case 'D':
                J++;
                break;
			case 'q':
			case 'Q':
                fim = true;
                break;
            default:;
        }

        I=I%ALTURA+ALTURA;
        J=J%LARGURA+LARGURA;

        body.push_front(head);
        head = coord(J%LARGURA,I%ALTURA);

        if(head == coin)
        {
            size++;
            grow = true;
            coinRequest = true;
        }

        if(coinRequest)
        {
            bool fail = false;
            do
            {
                coin.y=(rand()%ALTURA);
                coin.x=(rand()%LARGURA);
                if(coin.y >= ALTURA || coin.x >= LARGURA) fim = true;
                fail |= coin == head;
                if(fail) continue;
                for(list<coord>::iterator dot=body.begin();  !fail && dot != body.end();  dot++) fail |= coin == *dot;
                if(fail) continue;
                for(list<coord>::iterator dot=trail.begin(); !fail && dot != trail.end(); dot++) fail |= coin == *dot;
                if(fail) continue;
            }
            while(fail);
            coinRequest = false;
        }

        if(grow) grow = false;
        else
        {
            coord C = body.back();
            trail.push_front(C);
            body.pop_back();
        }
        if(trail.size()>TRAIL_SIZE)
        {
            clear = trail.back();
            trail.pop_back();
        }

		// clearScreen();
        printAt(3+coin.y, 1+2*(1+coin.x), COIN_CHAR);
        printAt(3+clear.y, 1+2*(1+clear.x), EMPTY_CHAR);
        for(list<coord>::iterator dot=trail.begin(); dot != trail.end(); dot++) printAt(3+dot->y, 1+2*(1+dot->x), TRAIL_CHAR);
        for(list<coord>::iterator dot=body.begin();  dot != body.end();  dot++) printAt(3+dot->y, 1+2*(1+dot->x), BODY_CHAR);
        printAt(3+head.y, 1+2*(1+head.x), HEAD_CHAR);

        cerr << "\x1b[H" << "\x1b[K" << input << "; " << size << " (" << I%ALTURA << "," << J%LARGURA << ") -> (" << coin.y <<"," << coin.x << ")";

    }

	disable_getch();

    cerr << "\x1b[" << 4+ALTURA << ";1H" << "\x1b[?25h";

	return 0;
}