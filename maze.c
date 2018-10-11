#include <avr/io.h>
#include "system.h"
#include "button.h"
#include "pacer.h"
#include "tinygl.h"
#include "../fonts/font3x5_1.h"
#include "navswitch.h"
#include <time.h>
#include <stdlib.h>

/* Define polling rates in Hz.  */
#define NAVSWITCH_TASK_RATE 100

#define DISPLAY_TASK_RATE 250

#define PACER_RATE 500

#define PLAYER_FLASH_RATE 2

#define height 31
#define width 32

int nodes1[62] = {0x7fff, 0xffff, 0x4000, 0x0001, 0x577d, 0x75fd, 
0x5405, 0x5505, 0x57df, 0xdddd, 0x5050, 0x0011, 0x5fdf, 0x5ff5, 
0x4401, 0x5005, 0x75fd, 0xd577, 0x5505, 0x5551, 0x575d, 0x5f5d, 
0x4011, 0x0005, 0x5dd7, 0x7ffd, 0x5154, 0x5001, 0x5d75, 0x5dfd, 
0x4505, 0x5105, 0x75dd, 0x5f75, 0x5451, 0x4045, 0x5f57, 0xdf5d, 
0x4154, 0x1151, 0x5d77, 0xd755, 0x5400, 0x1445, 0x5777, 0xf57d, 
0x4514, 0x0541, 0x5d55, 0x755d, 0x5155, 0x5551, 0x5777, 0x5d75, 
0x5400, 0x4005, 0x577f, 0x7ffd, 0x4040, 0x0001, 0x7fff, 0xffff};

int nodes2[62] = {0x7fff, 0xffff, 0x4001, 0x0001, 0x5ddd, 0xddfd, 
0x5544, 0x4511, 0x575f, 0x5d5d, 0x5041, 0x5145, 0x55dd, 0x7d75, 
0x5515, 0x105, 0x5575, 0x7f75, 0x4511, 0x4055, 0x5ddf, 0x7dd5, 
0x5040, 0x455, 0x57dd, 0xf75d, 0x5415, 0x141, 0x5575, 0xff7d, 
0x5544, 0x45, 0x5555, 0xdfdd, 0x4545, 0x5111, 0x575d, 0x7dd7, 
0x5050, 0x455, 0x5757, 0xf755, 0x5454, 0x111, 0x5fd7, 0xff5d, 
0x4050, 0x45, 0x5fdd, 0xfddd, 0x5005, 0x411, 0x5d5d, 0xd7dd, 
0x4550, 0x5045, 0x5ddf, 0xd07d, 0x5000, 0x1001, 0x7fff, 0xffff};

int nodes3[62] = {0x7fff, 0xffff, 0x4400, 0x0141, 0x5d5c, 0x1775, 
0x5550, 0x1415, 0x5770, 0x177d, 0x5000, 0x1041, 0x575c, 0x1f71, 
0x5450, 0x0011, 0x5777, 0x1dd1, 0x4145, 0x1151, 0x5ddd, 0xf155, 
0x4400, 0x0115, 0x5d7c, 0x7155, 0x5104, 0x5155, 0x5d7f, 0xddd5, 
0x4140, 0x0005, 0x775f, 0x7075, 0x5450, 0x1051, 0x545c, 0x77dd, 
0x4444, 0x0005, 0x5c77, 0xdc07, 0x4040, 0x5001, 0x5777, 0x5c01, 
0x5114, 0x4001, 0x5dd7, 0x5c01, 0x5040, 0x0001, 0x5fd7, 0x77f5, 
0x4011, 0x1015, 0x5c11, 0xf01d, 0x4010, 0x0001, 0x7fff, 0xffff};

int nodes4[62] = {0x7fff, 0xffff, 0x4100, 0x1101, 0x577f, 0xddf7, 
0x5140, 0x0411, 0x5f7f, 0xd5d5, 0x4005, 0x5455, 0x5dd5, 0x77dd, 
0x4545, 0x0101, 0x5d7d, 0xf75d, 0x5001, 0x4051, 0x5f77, 0x7ddd, 
0x4154, 0x0501, 0x5757, 0x5d5d, 0x5011, 0x4141, 0x5fd5, 0xdd5d, 
0x4004, 0x1055, 0x5ff7, 0x5fd5, 0x5010, 0x4011, 0x5f75, 0xddf7, 
0x4055, 0x1445, 0x57d5, 0x55dd, 0x5005, 0x5101, 0x57dd, 0x777d, 
0x5151, 0x0441, 0x715f, 0x7775, 0x4100, 0x1015, 0x5ddd, 0xdfdd, 
0x4455, 0x4141, 0x5f75, 0x5f7d, 0x4000, 0x4001, 0x7fff, 0xffff};

int nodes5[62] = {0x7fff, 0xffff, 0x4010, 0x0001, 0x5f7d, 0xd7dd, 
0x4501, 0x5515, 0x5557, 0x5d75, 0x5151, 0x0505, 0x5d5d, 0x55d5, 
0x4501, 0x5015, 0x55ff, 0x5ddd, 0x5400, 0x4101, 0x77f5, 0x7d75, 
0x4414, 0x0555, 0x5555, 0xdd55, 0x5441, 0x5555, 0x75df, 0x5555, 
0x5514, 0x5115, 0x5555, 0xd755, 0x5150, 0x0055, 0x5777, 0x7755, 
0x5401, 0x5455, 0x57f7, 0x5775, 0x5004, 0x5155, 0x5df7, 0x5d55, 
0x4141, 0x4045, 0x5f57, 0xfddd, 0x5454, 0x0501, 0x55d5, 0xdddd, 
0x4515, 0x1051, 0x5dd5, 0x7f5d, 0x4041, 0x0005, 0x7fff, 0xffff};

int* maps[5] = {nodes1, nodes2, nodes3, nodes4, nodes5};
    
int* nodes = 0;
    
static bool flash;

typedef struct position_s {
    int x;
    int y;
} Position;

Position position_init (void)
{
    Position position;
    position.x = 2;
    position.y = 1;
    return position;
}

// Tims edit
Position objects1[20] = {0};
Position objects2[20] = {0};

Position* objects_maps[] = {objects1, objects2};

static int player_score = 0;
static int r = 0;
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


Position navswitch_task(Position position)
{
    navswitch_update ();
    int x = position.x;
    int y = position.y;
    if (navswitch_push_event_p (NAVSWITCH_NORTH))
        if ((y > 0) & !(move(x,(y-1))))
            position.y--;

    if (navswitch_push_event_p (NAVSWITCH_SOUTH))
        if ((y < height) & !(move(x,(y+1))))
            position.y++;
        
    if (navswitch_push_event_p (NAVSWITCH_EAST))
        if ((x < width) & !(move((x+1),y)))
            position.x++;
        
    if (navswitch_push_event_p (NAVSWITCH_WEST))
        if ((x > 1) & !(move((x-1),y)))
            position.x--;
    
    // picking up object event
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) 
        if (is_on_object(position))
            player_score++;
            
    return position;
}



void update (Position position)
{
    tinygl_clear();
    tinygl_draw_point(tinygl_point (2, 3), flash);
    for ( int i = 0; i < 5; i++ )
	{
		for ( int j = 0; j < 7; j++ )
		{
            if (((i+position.x-2) >= 0) & ((i+position.x-2) < width)) {
                if (((j+position.y-3) >= 0) & ((j+position.y-3) < height)) {
                    if (move((i+position.x-2),(j+position.y-3))) {
                        tinygl_draw_point(tinygl_point (i, j), 1);
                    }
                }
            }
        }
    }
    tinygl_update ();
}

int main ( void )
{
    system_init();
    button_init ();
    navswitch_init ();
    tinygl_init (DISPLAY_TASK_RATE);
    tinygl_font_set (&font3x5_1);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    pacer_init(PACER_RATE);
    Position position = position_init();
    
    int built = 0;
    while(1)
    {
        for (int loop = 1; loop < 251; loop++) {
            pacer_wait();
            if (navswitch_push_event_p (NAVSWITCH_PUSH) & !built) {
                r = loop / 50;
                nodes = maps[r];
                built = 1;
            }
            
            if (loop % (PACER_RATE/DISPLAY_TASK_RATE) == 0) {
                update(position);
            }
            if (loop % (PACER_RATE/NAVSWITCH_TASK_RATE) == 0) {
                position = navswitch_task(position);
            }
            if (loop % (PACER_RATE/PLAYER_FLASH_RATE) == 0) {
                flash = !flash;
            }
        }
    }
}
