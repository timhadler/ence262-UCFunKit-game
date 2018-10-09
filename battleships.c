#include <avr/io.h>
#include "system.h"
#include "button.h"
#include "pacer.h"
#include "navswitch.h"
#include "pio.h"

#define NUM_SHIPS 4
#define MAX_NUM_SHIPS 6


static const pio_t rows[] =
{
    LEDMAT_ROW1_PIO, LEDMAT_ROW2_PIO, LEDMAT_ROW3_PIO,
    LEDMAT_ROW4_PIO, LEDMAT_ROW5_PIO, LEDMAT_ROW6_PIO,
    LEDMAT_ROW7_PIO
};


static const pio_t cols[] =
{
    LEDMAT_COL1_PIO, LEDMAT_COL2_PIO, LEDMAT_COL3_PIO,
    LEDMAT_COL4_PIO, LEDMAT_COL5_PIO
};



typedef struct Ship_s {
    int length;
    char orientation; // H for horizontal ship, V for verticle
} Ship;


static uint8_t bitmap[5];
Ship ships[NUM_SHIPS];


void display_init(void)
{
    for (int i = 0; i < LED_ROWS_NUM; i++) {
        pio_config_set(rows[i], PIO_OUTPUT_HIGH);
    }

    for (int n = 0; n < LED_COLS_NUM; n++) {
        pio_config_set(cols[n], PIO_OUTPUT_HIGH);
    }
}


void update_display(void)
{
    for
}


void display_ship(Ship ship)
{
    // if ship is verticle
    if (ship.orientation == "V") {
        for (int i = 0; i < ship.length; i++) {
            //pio_output_low(ROW1);
        }
    }
}


// Creates the ships
void ship_init(void)
{
    int lenghts[MAX_NUM_SHIPS] = [4, 3, 3, 2, 3, 3];
    char orientations[MAX_NUM_SHIPS] = ["V", "V", "H", "V", "V", "V"]; // pre-defines the orientation of the ships

    for (int i = 0; i < NUM_SHIPS; i++) {
        Ship ship;
        ship.length = lenghts[i];
        ship.orientation = orientations[i];
        ships[i];
    }
}


void game_init(void)
{
    for (ship = 0; ship < num_ships; ship++) {
        display_ship(ships(ship));
    }
}


int main (void)
{
    system_init ();


    while (1)
    {



    }
}
