#include <avr/io.h>
#include "ir_uart.h"
#include "system.h"
#include "button.h"
#include "pacer.h"
#include "tinygl.h"
#include "../fonts/font3x5_1.h"
#include "navswitch.h"
#include <stdlib.h>

/* Define polling rates in Hz.  */
#define NAVSWITCH_TASK_RATE 100

#define DISPLAY_TASK_RATE 250

#define PACER_RATE 500

#define PLAYER_FLASH_RATE 2

#define ENEMY_FLASH_RATE 5

#define MESSAGE_RATE 10

#define height 31
#define width 32

#define VISIBILITY 10

#define OBJECTS 10

int map[62] = {0x7fff, 0xffff, 0x4000, 0x0001, 0x577d, 0x75fd, 
0x5405, 0x5505, 0x57df, 0xdddd, 0x5050, 0x0011, 0x5fdf, 0x5ff5, 
0x4401, 0x5005, 0x75fd, 0xd577, 0x5505, 0x5551, 0x575d, 0x5f5d, 
0x4011, 0x0005, 0x5dd7, 0x7ffd, 0x5154, 0x5001, 0x5d75, 0x5dfd, 
0x4505, 0x5105, 0x75dd, 0x5f75, 0x5451, 0x4045, 0x5f57, 0xdf5d, 
0x4154, 0x1151, 0x5d77, 0xd755, 0x5400, 0x1445, 0x5777, 0xf57d, 
0x4514, 0x0541, 0x5d55, 0x755d, 0x5155, 0x5551, 0x5777, 0x5d75, 
0x5400, 0x4005, 0x577f, 0x7ffd, 0x4040, 0x0001, 0x7fff, 0xffff};

int* nodes;
    
static bool p1flash;
static bool eflash;
static int change_score;
static bool built;
static bool gg;
static int init_rand;

typedef struct position_s {
    int x;
    int y;
} Position;

Position route[VISIBILITY+1] = {0};
int ds[8] = {1, 0, -1, 0, 0, 1, 0, -1};

Position position_init (int x, int y)
{
    Position position;
    position.x = x;
    position.y = y;
    return position;
}

Position player1;
Position player2;
Position enemy;

Position objects[OBJECTS] = {0};

char p1_score[2] = {'0', '0'};
char p2_score[] = {'0', '0'};


void game_init(void)
{
    system_init();
    navswitch_init ();
    pacer_init(PACER_RATE);
    
    tinygl_init (DISPLAY_TASK_RATE);
    tinygl_font_set (&font3x5_1);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
}


void intro(void)
{
    tinygl_text_speed_set (10);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    
    char* intro_message = "MAZE!";
    tinygl_text(intro_message);
    
    init_rand = 0;
    while (!navswitch_push_event_p(NAVSWITCH_PUSH)) {
        pacer_wait ();
        
        if (init_rand > 1000) {
            init_rand = 0;
        } else {
            init_rand++;
        }
        
        tinygl_update ();
        navswitch_update ();
    }
}


void outro(void)
{
    tinygl_text_speed_set (20);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    
    tinygl_clear();
    
    char* ending_message = "THATS IT.GAME OVER MAN GAME OVER\0";
    tinygl_text(ending_message);
    
    while (1) {
        
    pacer_wait ();
        
    tinygl_update ();
    navswitch_update ();
    }
}


int move(int x, int y)
{
    int n = 0;
    n = nodes[2 * y + (x / 16)];
    int k = 0;
    if (x >= 16)
        x = x - 16;
    k = n >> (15-x);
    if (k & 1)
        return 1;
    else
        return 0;
    }
    
    
void communicate(void)
{
    char p1_x = player1.x + '0';
    char p1_y = player1.y + '0';
    
    char p1_position[] = {p1_x, p1_y, '\0'};
    
    ir_uart_puts(p1_position);
    
    if (ir_uart_read_ready_p ()) {
        player2.x = ir_uart_getc() - '0';
        player2.y = ir_uart_getc() - '0';
    }
}
    

// finds out if player is on an object
bool is_on_object(Position position, int loop)
{
    int max = 1;
    bool taken = 0;
    bool on_object = 0;
    
    for (int i = 0; i < OBJECTS; i++) {
        if (position.x == objects[i].x && position.y == objects[i].y) {
            on_object = 1;

            // change object
            for (int j = 0; j < max; j++) {
                srand(loop+j);
                int x = (rand() % 29) + 2;
                srand((2*loop)+j);
                int y = (rand() % 30) + 1;
                if (!move(x,y)) {
                    taken = 0;
                    for (int k = 0; k < OBJECTS; k++) {
                        if ((objects[k].x == x) && (objects[k].x == x)) {
                            taken = 1;
                        }
                    }
                    if (!taken) {
                        objects[i].x = x;
                        objects[i].y = y;
                    } else {
                        max++;
                    }
                } else {
                    max++;
                }
            }
        }
    }
    return on_object;
}  

void navswitch_task(int loop)
{
    navswitch_update ();
    int x = player1.x;
    int y = player1.y;
    if (navswitch_push_event_p (NAVSWITCH_NORTH)) {
        if ((y > 0) & !(move(x,(y-1)))) {
            player1.y--;
        }
    }

    if (navswitch_push_event_p (NAVSWITCH_SOUTH)) {
        if ((y < height) & !(move(x,(y+1)))) {
            player1.y++;
        }
    }
        
    if (navswitch_push_event_p (NAVSWITCH_EAST)) {
        if ((x < width) & !(move((x+1),y))) {
            player1.x++;
        }
    }
        
    if (navswitch_push_event_p (NAVSWITCH_WEST)) {
        if ((x > 1) & !(move((x-1),y))) {
            player1.x--;
        }
    }
    
    // picking up object event
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
        if (is_on_object(player1, loop)) {
            tinygl_clear();
            p1_score[0]++;
            change_score = 250;
            if (p1_score[0] > '3') {
                gg = 1;
            }
        }
    }
}

void got_rekt(int loop)
{
    tinygl_clear();
    p1_score[1]++;
    change_score = 250;
    int max = 1;
    for (int i = 0; i < max; i++) {
        srand(loop+i);
        int x = (rand() % 29) + 2;
        srand((2*loop)+i);
        int y = (rand() % 30) + 1;
        if (!move(x,y)) {
            player1 = position_init(x,y);
        } else {
            max++;
        }
    }
    if (p1_score[1] > '3') {
        gg = 1;
    }
}

void update (int loop)
{
    if (built) {
        if ((player1.x == enemy.x) && (player1.y == enemy.y)) {
            got_rekt(loop);
        }
        if (change_score) {
            tinygl_text(p1_score);
            change_score--;
        } else {
            tinygl_clear();
            tinygl_draw_point(tinygl_point (2, 3), p1flash);
            int k = 0;
            for ( int i = 0; i < 5; i++ )
            {
                for ( int j = 0; j < 7; j++ )
                {
                    if (((i+player1.x-2) >= 0) & ((i+player1.x-2) < width)) {
                        if (((j+player1.y-3) >= 0) & ((j+player1.y-3) < height)) {
                            for (k = 0; k < OBJECTS; k++) {
                                if (((i+player1.x-2) == objects[k].x) & ((j+player1.y-3) == objects[k].y)) {
                                    tinygl_draw_point(tinygl_point (i, j), !eflash);
                                    }
                                }
                            if (((i+player1.x-2) == enemy.x) & ((j+player1.y-3) == enemy.y)) {
                                tinygl_draw_point(tinygl_point (i, j), eflash);
                            } else if (move((i+player1.x-2),(j+player1.y-3))) {
                                tinygl_draw_point(tinygl_point (i, j), 1);
                            }
                        }
                    }
                }
            }
        }
    } else {
        tinygl_clear();
        tinygl_draw_point(tinygl_point (2, 3), p1flash);
    }
    tinygl_update ();
}

int algorithm(int i, int found)
{
    i++;
    if (i >= VISIBILITY) {
        return 0;
    }
    int x = route[i].x;
    int y = route[i].y;
    for (int d = 0; d < 8; d++) {
        if (!move(x+ds[d],y+ds[d+1])) {
            if (((x+ds[d]) != route[i-1].x) || ((y+ds[d+1]) != route[i-1].y)) {
                route[i+1].x = x + ds[d];
                route[i+1].y = y + ds[d+1];
                if ((route[i+1].x == player1.x) && (route[i+1].y == player1.y)) {
                    return 1;
                }
                found = algorithm(i, found);
                if (found) {
                    return 1;
                }
            }
        }
    d++;
    }
    i--;
    return 0;
}
    
void update_enemy(void)
{
    int found = 0;
    route[0].x = enemy.x;
    route[0].y = enemy.y;
    int i = 0;
    int x = route[i].x;
    int y = route[i].y;
    for (int d = 0; d < 8; d++) {
        if (!move(x+ds[d],y+ds[d+1])) {
            route[i+1].x = x + ds[d];
            route[i+1].y = y + ds[d+1];
            if ((route[i+1].x == player1.x) && (route[i+1].y == player1.y)) {
                found = 1;
                enemy.x = route[1].x;
                enemy.y = route[1].y;
                break;
            }
            found = algorithm(i, found);
            if (found) {
                enemy.x = route[1].x;
                enemy.y = route[1].y;
                break;
            }
        }
        d++;
    }
}

int main ( void )
{
    gg = 0;
    game_init();
    
    enemy = position_init(30,1);
    int max = OBJECTS+2;
    int index = -2;
    built = 0;

    intro();
    tinygl_text_speed_set (10);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    while(!gg)
    {
        for (int loop = 1; loop < 1001; loop++) {
            pacer_wait();
            if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
                if (!built) {
                    for (int i = 0; i < max; i++) {
                        srand(init_rand+i);
                        int x = (rand() % 29) + 2;
                        srand((2*init_rand)+i);
                        int y = (rand() % 30) + 1;
                        if (!move(x,y)) {
                            if (index == -2) {
                                player1 = position_init(x,y);
                            } else if (index == -1) {
                                enemy = position_init(x,y);
                            } else {
                                objects[index].x = x;
                                objects[index].y = y;
                            }
                            index++;
                        } else {
                            max++;
                        }
                    }
                    nodes = map;
                    built = 1;
                    change_score = 0;
                }
            }
            if (built) {
                if (loop % (PACER_RATE/DISPLAY_TASK_RATE) == 0) {
                    update(loop);
                }
                if (loop % (PACER_RATE/NAVSWITCH_TASK_RATE) == 0) {
                    if (built && !change_score) {
                        navswitch_task(loop);
                    }
                }
                if (loop % (PACER_RATE/PLAYER_FLASH_RATE) == 0) {
                    update_enemy();
                    p1flash = !p1flash;
                }
                if (loop % (PACER_RATE/ENEMY_FLASH_RATE) == 0) {
                    eflash = !eflash;
                }
            }
        }
    }
    outro();
}
