#include <avr/io.h>
#include "system.h"
#include "button.h"
#include "pacer.h"
#include "tinygl.h"
#include "../fonts/font3x5_1.h"
#include "navswitch.h"

/* Define polling rates in Hz.  */
#define NAVSWITCH_TASK_RATE 100

#define DISPLAY_TASK_RATE 250

#define PACER_RATE 500

#define PLAYER_FLASH_RATE 2

#define height 31
#define width 32

int nodes[62] = {0x7FFF, 0xFFFF, 0x5400, 0x4001, 0x55FF, 0x5FFD,
    0x5111, 0x0101, 0x5755, 0xFF7F, 0x5445, 0x4111, 0x557D, 0x5DD5,
    0x5144, 0x5515, 0x5FD7, 0x5575, 0x4111, 0x5505, 0x7D7D, 0xD5FD,
    0x4105, 0x1005, 0x5FF5, 0x5FF5, 0x5005, 0x5115, 0x57FD, 0x7575,
    0x5015, 0x1501, 0x5FD5, 0x55FD, 0x5011, 0x4445, 0x57F7, 0x7F57,
    0x5114, 0x1111, 0x5557, 0xDDFD, 0x5451, 0x0041, 0x5FDD, 0xFFDF,
    0x5011, 0x0111, 0x57F7, 0x7D75, 0x5410, 0x4505, 0x575F, 0xF5FD,
    0x4450, 0x0511, 0x7DF5, 0xFD55, 0x4004, 0x0045, 0x7FFF, 0xFFFF};

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
    while(1)
    {
        for (int loop = 1; loop < 251; loop++) {
            pacer_wait();
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
