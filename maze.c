#include <avr/io.h>
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

#define height 31
#define width 32

#define VISIBILITY 10

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

// Tims edit
Position objects1[20] = {0};
Position objects2[20] = {0};

Position* objects_maps[] = {objects1, objects2};

static int player_score = 0;
static int r = 0;


void game_init(void)
{
    system_init();
    button_init ();
    navswitch_init ();
    pacer_init(PACER_RATE);
    
    tinygl_init (DISPLAY_TASK_RATE);
    tinygl_font_set (&font3x5_1);
    tinygl_text_speed_set (10);
}


void intro(void)
{
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    
    char* intro_message = "MAZE!";
    tinygl_text(intro_message);
    
    while (!navswitch_push_event_p(NAVSWITCH_PUSH)) {
        pacer_wait ();
        
        tinygl_update ();
        navswitch_update ();
    }
}


void outro(void)
{
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    
    char* ending_message;
    tinygl_text(ending_message);
    
    while (1) {
    pacer_wait ();
        
    tinygl_update ();
    navswitch_update ();
    }
}
//.


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
    

// finds out if player is on an object
bool is_on_object(Position position)
{
    bool on_object = 0;
    
    for (int i = 0; i < 20; i++) {
        if (position.x == objects_maps[r][i].x && position.y == objects_maps[r][i].y) {
            on_object = 1;
            
            // remove object
            objects_maps[r][i].x = 0x00;
            objects_maps[r][i].y = 0x00;
        }
    }
    return on_object;
}    


Position navswitch_task(Position player1)
{
    navswitch_update ();
    int x = player1.x;
    int y = player1.y;
    if (navswitch_push_event_p (NAVSWITCH_NORTH))
        if ((y > 0) & !(move(x,(y-1))))
            player1.y--;

    if (navswitch_push_event_p (NAVSWITCH_SOUTH))
        if ((y < height) & !(move(x,(y+1))))
            player1.y++;
        
    if (navswitch_push_event_p (NAVSWITCH_EAST))
        if ((x < width) & !(move((x+1),y)))
            player1.x++;
        
    if (navswitch_push_event_p (NAVSWITCH_WEST))
        if ((x > 1) & !(move((x-1),y)))
            player1.x--;
    
    // picking up object event
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) 
        if (is_on_object(player1))
            player_score++;
            
    return player1;
}



void update (Position player1, Position enemy)
{
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_clear();
    tinygl_draw_point(tinygl_point (2, 3), p1flash);
    int k = 0;
    for ( int i = 0; i < 5; i++ )
	{
		for ( int j = 0; j < 7; j++ )
		{
            if (((i+player1.x-2) >= 0) & ((i+player1.x-2) < width)) {
                if (((j+player1.y-3) >= 0) & ((j+player1.y-3) < height)) {
                    for (k = 0; k < 5; k++) {
                        if (((i+player1.x-2) == objects_maps[0][k].x) & ((j+player1.y-3) == objects_maps[0][k].y)) {
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
    tinygl_update ();
}

int algorithm(int i, int found, Position player1, Position enemy)
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
                found = algorithm(i, found, player1 ,enemy);
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
    
Position update_enemy(Position player1, Position enemy)
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
            found = algorithm(i, found, player1 ,enemy);
            if (found) {
                enemy.x = route[1].x;
                enemy.y = route[1].y;
                break;
            }
        }
        d++;
    }
    return enemy;
}

int main ( void )
{
    game_init();
    
    Position player1 = position_init(2,1);
    Position enemy = position_init(4,1);
    int max = 5;
    int built = 0;

    intro();
    while(1)
    {
        for (int loop = 1; loop < 251; loop++) {
            pacer_wait();
            if (navswitch_push_event_p (NAVSWITCH_PUSH) & !built) {
                for (int i = 0; i < max; i++) {
                    srand(loop+i);
                    int x = (rand() % 29) + 2;
                    srand((2*loop)+i);
                    int y = (rand() % 30) + 1;
                    if (!move(x,y)) {
                        objects_maps[0][i].x = x;
                        objects_maps[0][i].y = y;
                    } else {
                        max++;
                    }
                }
                nodes = map;
                built = 1;
            }
            if (loop % (PACER_RATE/DISPLAY_TASK_RATE) == 0) {
                update(player1, enemy);
            }
            if (loop % (PACER_RATE/NAVSWITCH_TASK_RATE) == 0) {
                player1 = navswitch_task(player1);
            }
            if (loop % (PACER_RATE/PLAYER_FLASH_RATE) == 0) {
                if (built) {
                    enemy = update_enemy(player1, enemy);
                }
                p1flash = !p1flash;
            }
            if (loop % (PACER_RATE/ENEMY_FLASH_RATE) == 0) {
                eflash = !eflash;
            }
        }
    }
    outro();
}
