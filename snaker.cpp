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

#define ALTURA 30
#define LARGURA 30
#define DELAY 0.1
#define TRAIL_SIZE ((ALTURA+LARGURA)/4)

#define AUTOWALK 1
#define AUTOGROW 0

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

void printAt(int Y, int X, char C)
{
    cerr << "\x1b[" << (Y) << ";" << (X) << "H" << (C);
}

void printTableBorder(int xBase, int yBase, int height = ALTURA, int width = LARGURA, char character = WALL_CHAR)
{
        for(int j = 0; j < width + 2; j++)
        {
            printAt(2,1+j*2,character);
            // cerr << "\x1b[" << 2 << ";" << 1+j*2 << "H" << character;
            printAt(3+height,1+j*2,character);
            // cerr << "\x1b[" << 3+height << ";" << 1+j*2 << "H" << character;
        }
        for(int i = 0; i <= height; i++)
        {
            printAt(2+i,1,character);
            // cerr << "\x1b[" << 2+i << ";" << 1 << "H" << character;
            printAt(2+i,1+2*(1+width),character);
            // cerr << "\x1b[" << 2+i << ";" << 1+2*(1+width) << "H" << character;
        }
}

int main()
{
    list<coord> body, trail;
    char input('\0');
    bool fim(false);

    int I(ALTURA/2), J(LARGURA/2);
    coord head(I,J), coin(0,0), clear(0,0);

	int size(1), t_counter(0);
    bool grow(false);
    bool coinRequest(true);


    char** table = new char*[ALTURA]; // {}
    for(int i=0; i<ALTURA; i++) table[i] = new char[LARGURA];

    for(int i=0; i<ALTURA; i++) memset(table[i],EMPTY_CHAR,sizeof(char)*LARGURA); //Limpa tabuleiro

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

        //table[I%ALTURA][J%LARGURA];
        I=I%ALTURA+ALTURA;
        J=J%LARGURA+LARGURA;

        body.push_front(head);
        head = coord(I%ALTURA,J%LARGURA);
        table[head.x][head.y] = HEAD_CHAR;

        if(table[I%ALTURA][J%LARGURA]==COIN_CHAR)
        {
            size++;
            grow = true;
            coinRequest = true;
        }

        if(coinRequest)
        {
            do
            {
                coin.x=(rand()%ALTURA);
                coin.y=(rand()%LARGURA);
                // coin.x%=ALTURA;
                // coin.y%=LARGURA;
                if(coin.x >= ALTURA || coin.y >= LARGURA) fim = true;
            }
            while(table[coin.x][coin.y]!=EMPTY_CHAR);
            table[coin.x%ALTURA][coin.y%LARGURA] = COIN_CHAR;
            coinRequest = false;
        }


        if(grow) grow = false;
        else
        {
            coord C = body.back();
            trail.push_front(C);
            table[C.x][C.y] = TRAIL_CHAR;
            body.pop_back();
        }
        if(trail.size()>TRAIL_SIZE)
        {
            clear = trail.back();
            table[clear.x][clear.y]=EMPTY_CHAR;
            trail.pop_back();
        }

        for(list<coord>::iterator dot=body.begin(); dot != body.end(); dot++) table[dot->x][dot->y]=BODY_CHAR;
        table[head.x][head.y] = HEAD_CHAR;

		// clearScreen();

        printAt(3+coin.x, 1+2*(1+coin.y), COIN_CHAR);
        printAt(3+clear.x, 1+2*(1+clear.y), EMPTY_CHAR);
        for(list<coord>::iterator dot=trail.begin(); dot != trail.end(); dot++) printAt(3+dot->x, 1+2*(1+dot->y), TRAIL_CHAR);
        for(list<coord>::iterator dot=body.begin();  dot != body.end();  dot++) printAt(3+dot->x, 1+2*(1+dot->y), BODY_CHAR);
        printAt(3+head.x, 1+2*(1+head.y), HEAD_CHAR);

        cerr << "\x1b[H" << "\x1b[K" << input << "; " << size << " (" << I%ALTURA << "," << J%LARGURA << ") -> (" << coin.x <<"," << coin.y << ")";

        #if 0 /* Classic interface generator */
        cout << "\x1b[1;1H" <<input << "; " << size << " (" << I%ALTURA << "," << J%LARGURA << ") -> (" << coin.x <<"," << coin.y << ")";
        cerr << endl;
        for(int j = 0; j < LARGURA + 2; j++) cerr << WALL_CHAR << " "; cerr << endl;
        for(int i = 0; i < ALTURA; i++)
        {
            cerr << WALL_CHAR << " " ;
            for(int j = 0; j < LARGURA; j++)
            {
                cerr << table[i][j] << " ";
            }
            cerr << WALL_CHAR << endl;
        }
        for(int j = 0; j < LARGURA + 2; j++) cerr << WALL_CHAR << " "; cerr << endl;
        #endif

    }

    for (int i=0; i<ALTURA; i++) delete[] table[i];
    delete[] table;

	disable_getch();

    cerr << "\x1b[" << 4+ALTURA << ";1H" << "\x1b[?25h";

	return 0;
}