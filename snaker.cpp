#include <iostream>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#include <conio.h>

#elif __linux__

#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

#endif



#define ALTURA 10
#define LARGURA 5

using namespace std;

#ifdef _WIN32

#elif __linux__

struct termios orig_termios;

void disable_getch()  //Retorna para o terminal original // reset_terminal_mode
{
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void enable_getch()  //Configura terminal especial // set_conio_terminal_mode
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(STDIN_FILENO, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(disable_getch);
    cfmakeraw(&new_termios);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

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

int main()
{
    int I(ALTURA/2), J(LARGURA/2);

#ifdef __linux__
    enable_getch();
#endif

#ifdef __linux__
    disable_getch();
#endif

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