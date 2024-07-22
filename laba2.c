#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Displey_laboratornaya_2_3.h"
#include "tft.h"
#include <stdio.h>

#define BUTTON_ONE 0b0001
#define BUTTON_TWO 0b0010
#define BUTTON_THREE 0b0100
#define BUTTON_FOUR 0b1000

struct Planet
{
    char x;
    char y;
    char radius;
    char orbit_radius;
    char angle;
};

char detect_buttons_in_delay(int delay)
{
    static char previous_port = 0xFF;
    int quant_count = delay / 10;
    for (int quant_number = 0; quant_number < quant_count; ++quant_number)
    {
        char current_port = PINB;
        char buttons = ~current_port & previous_port; 
        previous_port = current_port;
        if (buttons)
            return buttons;
        _delay_ms(10);
    }
    return 0x00;
}

void UpdatePlanetPosition(struct Planet *planet)
{
    planet->angle += 1;
    if (planet->angle >= 360)
    {
        planet->angle = 0;
    }

    planet->x = XSIZE / 2 + planet->orbit_radius * cos(planet->angle);
    planet->y = YSIZE / 2 + planet->orbit_radius * sin(planet->angle);
}

void UpdatePlanetPositionObr(struct Planet *planet)
{
    planet->angle -= 1;
    if (planet->angle <= -360)
    {
        planet->angle = 0;
    }

    planet->x = XSIZE / 2 + planet->orbit_radius * cos(planet->angle);
    planet->y = YSIZE / 2 + planet->orbit_radius * sin(planet->angle);
}

void UpdatePlanetPosition1(struct Planet *planet)
{
    planet->angle += 10;
    if (planet->angle >= 360)
    {
        planet->angle = 0;
    }

    planet->x = XSIZE / 2 + planet->orbit_radius * cos(planet->angle);
    planet->y = YSIZE / 2 + planet->orbit_radius * sin(planet->angle);
}

void UpdateSunSize(struct Planet *sun)
{
    sun->radius += 10;

    if (sun->radius >= 70)
    {
        sun->radius = 70;
    }

 
}

int main()
{
    DDRD = 0xFF;
	DDRB = 0x00;
	PORTB = 0xFF;
	PORTD = 0xFF;
	InitTFT();
	char isPlaying = 0;

	struct Planet planets[3] = {

        {XSIZE / 2, YSIZE / 2, 3, 30, 0}, 
		{XSIZE / 2, YSIZE / 2, 5, 40, 163},
        {XSIZE / 2, YSIZE / 2, 7, 50, 45}};

	struct Planet sun[1] = {

        {XSIZE / 2, YSIZE / 2, 10, 0, 0}};

while (1)
{
char buttons = detect_buttons_in_delay(500);
if (buttons & BUTTON_ONE)
    { 
	isPlaying = 1;
	while (isPlaying)
    	{

	    ClearScreen();

        FillCircle(XSIZE / 2, YSIZE / 2, 10, YELLOW);
		Circle(XSIZE / 2, YSIZE / 2, 30, WHITE);
		Circle(XSIZE / 2, YSIZE / 2, 40, WHITE);
       
        UpdatePlanetPosition(&planets[0]);
        FillCircle(planets[0].x, planets[0].y, planets[0].radius, WHITE); 

        UpdatePlanetPosition1(&planets[1]);
        FillCircle(planets[1].x, planets[1].y, planets[1].radius, RED);
		
		UpdatePlanetPosition(&planets[2]);
        FillCircle(planets[2].x, planets[2].y, planets[2].radius, BLUE);  
        _delay_ms(50); 

		buttons = detect_buttons_in_delay(50);

        if (buttons & BUTTON_TWO)
                {
                    isPlaying = 0; 
                    
                }

		}

	}
if (buttons & BUTTON_THREE)
    {
	isPlaying = 1;
	while (isPlaying)
    	{

	    ClearScreen();

        FillCircle(XSIZE / 2, YSIZE / 2, 10, YELLOW);
		Circle(XSIZE / 2, YSIZE / 2, 30, WHITE);
		Circle(XSIZE / 2, YSIZE / 2, 40, WHITE);
		//Circle(XSIZE / 2, YSIZE / 2, 50, WHITE);
       
        UpdatePlanetPositionObr(&planets[0]);
        FillCircle(planets[0].x, planets[0].y, planets[0].radius, WHITE); 

        UpdatePlanetPositionObr(&planets[1]);
        FillCircle(planets[1].x, planets[1].y, planets[1].radius, RED);
		
		UpdatePlanetPositionObr(&planets[2]);
        FillCircle(planets[2].x, planets[2].y, planets[2].radius, BLUE);  
        _delay_ms(50); 

		buttons = detect_buttons_in_delay(50);

        if (buttons & BUTTON_TWO)
                {
                    isPlaying = 0; 
                    
                }
	}
}
if (buttons & BUTTON_FOUR)
    {
	isPlaying = 1;
	while (isPlaying)
    	{

	    ClearScreen();

       
		Circle(XSIZE / 2, YSIZE / 2, 30, WHITE);
		Circle(XSIZE / 2, YSIZE / 2, 40, WHITE);
		//Circle(XSIZE / 2, YSIZE / 2, 50, WHITE);
       
        UpdatePlanetPositionObr(&planets[0]);
        FillCircle(planets[0].x, planets[0].y, planets[0].radius, WHITE); 

        UpdatePlanetPositionObr(&planets[1]);
        FillCircle(planets[1].x, planets[1].y, planets[1].radius, RED);
		
		UpdatePlanetPositionObr(&planets[2]);
        FillCircle(planets[2].x, planets[2].y, planets[2].radius, BLUE);  

		FillCircle(sun[0].x, sun[0].y, sun[0].radius, YELLOW);
		UpdateSunSize(&sun[0]);
        _delay_ms(50); 

		buttons = detect_buttons_in_delay(50);

        if (buttons & BUTTON_TWO)
                {
                    isPlaying = 0; 
                    
                }
	}
}

}

	return 0;
}
