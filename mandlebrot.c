#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>
#include <complex.h>
#include <math.h>
#include <termios.h>            //termios, TCSANOW, ECHO, ICANON
#include <pthread.h>

#include "ANSI_COLOR_CODES.h"

#define OPTIONS "h:w:m:"

#define THREAD_COUNT 16

static char *colors[] = {
    reset,
    REDHB,
    YELHB,
    GRNHB,
    BLUHB,
    MAGHB,
    CYNHB,
    WHTHB
};

typedef enum {
    BLACK,
    RED,
    YELLOW,
    GREEN,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    COLOR_LEN
} color;

typedef struct Point {
    double x;
    double y;
} Point;

typedef struct TPS_STRUCT {
    color **arr;
    size_t width;
    size_t height;
    size_t height_offset;
    Point bl;
    Point tr;
} TPS;

color iterate(Point p, size_t max) {
    double x = 0, y = 0;
    size_t iteration = 0;
    
    while (x * x + y * y < 4 && iteration < max) {
        double xtemp = x * x - y * y + p.x;
        y = 2 * x * y + p.y;
        x = xtemp;

        ++iteration;
    }

    if (iteration < 5) return BLUE;
    if (iteration < 10) return RED;
    if (iteration < 15) return YELLOW;
    if (iteration < 20) return GREEN;
    if (iteration < 30) return CYAN;
    if (iteration < 40) return MAGENTA;
    if (iteration < 50) return BLUE;
    if (iteration < 100) return RED;
    if (iteration < 200) return YELLOW;
    if (iteration < 300) return GREEN;
    if (iteration < 400) return CYAN;
    if (iteration < 500) return MAGENTA;
    if (iteration < 600) return BLUE;
    if (iteration < 700) return RED;
    if (iteration < 800) return YELLOW;
    if (iteration < 900) return GREEN;
    if (iteration < 1000) return CYAN;
    if (iteration < 1100) return MAGENTA;
    if (iteration < 1200) return BLUE;
    if (iteration < 1300) return RED;
    if (iteration < 1400) return YELLOW;
    if (iteration < 1500) return GREEN;
    if (iteration < 1600) return CYAN;
    if (iteration < 1700) return MAGENTA;
    if (iteration < 1800) return WHITE;
    return BLACK;
}

void calculator(TPS *t) {
    double y_diff = (t->tr.y - t->bl.y);
    double x_diff = (t->tr.x - t->bl.x);
    for (size_t i = 0; i < t->height; i++) {
        for (size_t j = 0; j < t->width; j++) {
            Point x;
            x.x = (x_diff * j) / t->width + t->bl.x; 
            x.y = (y_diff * i) / t->height + t->bl.y;

            t->arr[i + t->height_offset][j] = iterate(x, 2000); 
        }
    }
    free(t);
}

void draw_screen(Point bl, Point tr) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    size_t max_height = w.ws_row, max_width = w.ws_col;

    size_t screen_height = max_height - 2, screen_width = max_width;

    double y_diff = (tr.y - bl.y);
    double x_diff = (tr.x - bl.x);
    
    color **arr = malloc(sizeof(*colors) * screen_height);
    for (size_t i = 0; i < screen_height; i++) {
        arr[i] = malloc(sizeof(colors) * screen_width);
    }

    pthread_t p_arr[THREAD_COUNT];
    size_t height_offset = 0;
    for (size_t i = 0; i < THREAD_COUNT; i++) {
        TPS *t = malloc(sizeof(TPS));

        t->arr = arr;
        t->width = screen_width;
        t->height = screen_height / THREAD_COUNT + (screen_height % THREAD_COUNT > i); 
        t->height_offset = height_offset; 

        t->bl.x = bl.x;
        t->bl.y = (y_diff * t->height_offset) / screen_height + bl.y;
        t->tr.x = tr.x;
        t->tr.y = (y_diff * (t->height + t->height_offset)) / screen_height + bl.y;

        height_offset = t->height_offset + t->height;
        //printf("(%lf %lf) (%lf %lf)\n", bl.x, bl.y, tr.x, tr.y);
        //printf("%zu %zu %zu %zu %zu\n", screen_width, screen_height, t->width, t->height, t->height_offset);
        printf("(%lf %lf) (%lf %lf)\n", t->bl.x, t->bl.y, t->tr.x, t->tr.y);
        pthread_create(p_arr + i, NULL, calculator, t);
    }


    char *write_buffer = malloc(sizeof(char) * screen_width * 10 + 1); 
    size_t len = 0;
    for (size_t i = 0; i < THREAD_COUNT; i++) {
        pthread_join(p_arr[i],  NULL);
    }

    for (size_t i = 0; i < screen_height; i++) {
        for (size_t j = 0; j < screen_width; j++) {
            for (int k = 0; colors[arr[i][j]][k] != '\0'; k++) {
                write_buffer[len++] = colors[arr[i][j]][k];
            } 
            write_buffer[len++] = ' ';
        }
        write_buffer[len++] = '\0';
        
        write(1, write_buffer, len);
        len = 0;
    }
    write(1, reset, strlen(reset));

    for (size_t i = 0; i < screen_height; i++) {
        free(arr[i]);
    }
    free(arr);
}

int main(int argc, char *argv[]) {
    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    Point bottom_left, top_right;
    bottom_left.x = -1;
    bottom_left.y = -1;
    top_right.x = 1;
    top_right.y = 1;
    char c;
    printf("\e[?25l");
    draw_screen(bottom_left, top_right);
    while (true) {
        c = getchar();
        printf("\033[1D");
        if (c == '+' || c == '=') {
            float mid_x = (top_right.x + bottom_left.x) / 2, mid_y = (top_right.y + bottom_left.y) / 2;
            bottom_left.x += fabs(mid_x - bottom_left.x) / 2;
            bottom_left.y += fabs(mid_y - bottom_left.y) / 2;
            top_right.x   -= fabs(top_right.x - mid_x) / 2;
            top_right.y   -= fabs(top_right.y - mid_y ) / 2;
        }
        if (c == '_' || c == '-') {
            float mid_x = (top_right.x + bottom_left.x) / 2, mid_y = (top_right.y + bottom_left.y) / 2;
            bottom_left.x -= fabs(mid_x - bottom_left.x) / 2;
            bottom_left.y -= fabs(mid_y - bottom_left.y) / 2;
            top_right.x   += fabs(mid_x - top_right.x) / 2;
            top_right.y   += fabs(mid_y - top_right.y) / 2;
        }
        if (c == 'a' || c == 'd') {
            float offset = fabs(top_right.x - bottom_left.x) * (c == 'a' ? -1 : 1) / 4;
            bottom_left.x += offset; 
            top_right.x += offset; 
        }
        if (c == 'w' || c == 's') {
            float offset = fabs(top_right.y - bottom_left.y) * (c == 'w' ? -1 : 1) / 4; 
            bottom_left.y += offset; 
            top_right.y += offset; 
        }
        
        if (c == 27) {
            break;
        }

        draw_screen(bottom_left, top_right);
        printf("[center: (%f, %f)]\n", (top_right.x + bottom_left.x) / 2, (top_right.y + bottom_left.y) / 2);
        
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    return 0;
}
