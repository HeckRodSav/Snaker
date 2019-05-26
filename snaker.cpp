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

int main()
{
    list<coord> body, trail;
    char input('\0');
    bool fim(false);

    int I(ALTURA/2), J(LARGURA/2);
    body.push_front(coord(I,J));

	int size(1), t_counter(0);
    bool grow(false);
    bool coinRequest(true);


    char** table = new char*[ALTURA]; // {}
    for(int i=0; i<ALTURA; i++) table[i] = new char[LARGURA];

    for(int i=0; i<ALTURA; i++) memset(table[i],EMPTY_CHAR,sizeof(char)*LARGURA); //Limpa tabuleiro

    double Tnow(double(clock())/CLOCKS_PER_SEC);
    double Tbefore(Tnow);

    coord coin(0,0);

	enable_getch();

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

        if(coinRequest)
        {
            do
            {
                coin.x=(rand()&ALTURA);
                coin.y=(rand()&LARGURA);
                coin.x%=ALTURA;
                coin.y%=LARGURA;
                if(coin.x >= ALTURA || coin.y >= LARGURA) fim = true;
            }
            while(table[coin.x][coin.y]!=EMPTY_CHAR);
            table[coin.x&ALTURA][coin.y&LARGURA] = COIN_CHAR;
            coinRequest = false;
        }

        while(kbhit()==0 && Tnow-Tbefore<DELAY) Tnow = double(clock())/CLOCKS_PER_SEC;
        if(kbhit()) input = getch();
        Tbefore = Tnow;

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

		//int V[ALTURA][LARGURA];
        I=I%ALTURA+ALTURA;
        J=J%LARGURA+LARGURA;

        if(table[I%ALTURA][J%LARGURA]==COIN_CHAR)
        {
            size++;
            grow = true;
            coinRequest = true;
        }

        body.push_front(coord(I%ALTURA,J%LARGURA));
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
            coord C = trail.back();
            trail.pop_back();
            table[C.x][C.y]=EMPTY_CHAR;
        }

        // for(int i=0; i<ALTURA; i++) for(int j=0; j<LARGURA; j++) table[i][j]='.'; //Limpa tabuleiro

        //table[I%ALTURA][J%LARGURA] = '#';
        for(list<coord>::iterator dot=body.begin(); dot != body.end(); dot++) table[dot->x][dot->y]=BODY_CHAR;
        table[body.front().x][body.front().y] = HEAD_CHAR;

		clearScreen();

        cout << input << "; " << size << " (" << I%ALTURA << "," << J%LARGURA << ") -> (" << coin.x <<"," << coin.y << ")" << endl;

        for(int j = 0; j < LARGURA + 2; j++) cout << WALL_CHAR << " "; cout << endl;
        for(int i = 0; i < ALTURA; i++)
        {
            cout << WALL_CHAR << " " ;
            for(int j = 0; j < LARGURA; j++)
            {
                cout << table[i][j] << " ";
            }
            cout << WALL_CHAR << endl;
        }
        for(int j = 0; j < LARGURA + 2; j++) cout << WALL_CHAR << " "; cout << endl;
    }

    for (int i=0; i<ALTURA; i++) delete[] table[i];
    delete[] table;

	disable_getch();

	return 0;
}