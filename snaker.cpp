#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
    if ((r = read(0, &c, sizeof(c))) < 0) {
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

#define ALTURA 10
#define LARGURA 5

using namespace std;

int main()
{
    char input('\0');
    bool fim(false);
    int I(ALTURA/2), J(LARGURA/2);
		enable_getch();

    while(!fim)
	{
		while(kbhit()==0);
        input = getch();

		clearScreen();

		cout << input << endl;
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
		I%=ALTURA;
		J%=LARGURA;
		for(int i = 0; i < ALTURA; i++)
		{
			for(int j = 0; j < LARGURA; j++)
			{
				if ( i==I && j==J ) cout << "# ";
				else cout << ". ";
			}
			cout << endl;
		}
    }
		disable_getch();
	return 0;
}