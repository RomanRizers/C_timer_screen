#include <avr/io.h>
#include <util/delay.h>
#include "tft.h"
#include "math.h"

// Определение кнопок

#define BUTTON_TIMER_ON   0b00000001
#define BUTTON_TIMER_OFF  0b00000010
#define BUTTON_TIMER_NULL 0b00000100

// Глобальные переменные

static char minutes = 0;
static char seconds = 0;
static char prev_minute = 0;
static char prev_second = 0;
static int minutes_color = LIME;
static int seconds_color = WHITE;
static int background_color = BLACK;

// Координаты центра и радиус часов

static byte centerX = 64;
static byte centerY = 60;
static byte clockRadius = 45;

// Функция определения нажатых кнопок

char detect_buttons() {
    static char previous_port = 0xFF;
    char current_port = PINB;
    char buttons = ~current_port & previous_port;
    previous_port = current_port;
    return buttons;
}

// Функция рисования циферблата часов

void DrawClockFace() {
    // 12 делений для каждой минуты
    for (int i = 0; i < 12; ++i) { // 30 градусов для каждой минуты
        double angle = i * 30 * M_PI / 180.0;
        byte x1 = centerX + (clockRadius - 5) * cos(angle);
        byte y1 = centerY - (clockRadius - 5) * sin(angle);
        byte x2 = centerX + clockRadius * cos(angle);
        byte y2 = centerY - clockRadius * sin(angle);
        Line(x1, y1, x2, y2, LIME); // линии для каждой минуты
    }
}

// Функция стирания предыдущего времени

void erase_prev_time() {
    double minutesAngle = (90 - (prev_minute % 12) * 30 - (prev_second / 60.0) * 30) * M_PI / 180.0;
    byte minutesLength = clockRadius - 15;

    double secondsAngle = (90 - prev_second * 6) * M_PI / 180.0;
    byte secondsLength = clockRadius - 7;

    Line(centerX, centerY, centerX + secondsLength * cos(secondsAngle), centerY - secondsLength * sin(secondsAngle), background_color);

    Line(centerX, centerY, centerX + minutesLength * cos(minutesAngle), centerY - minutesLength * sin(minutesAngle), background_color);
}

// Функция рисования текущего времени

void draw_time() {
    double minutesAngle = (90 - (minutes % 12) * 30 - (seconds / 60.0) * 30) * M_PI / 180.0;
    byte minutesLength = clockRadius - 15; // рассчитываем угол для минутной стрелки

    double secondsAngle = (90 - seconds * 6) * M_PI / 180.0;
    byte secondsLength = clockRadius - 7; // для секундной
    // отрисовка стрелок
    Line(centerX, centerY, centerX + secondsLength * cos(secondsAngle), centerY - secondsLength * sin(secondsAngle), seconds_color);

    Line(centerX, centerY, centerX + minutesLength * cos(minutesAngle), centerY - minutesLength * sin(minutesAngle), minutes_color);

    prev_minute = minutes;
    prev_second = seconds;

    write_time();
}

// Функция записи времени на экран

void write_time() {
    if (minutes < 10) {
        GotoXY(11, 15);
        WriteInt(0);
        GotoXY(12, 15);
    }
    else {
        GotoXY(11, 15);
    }

    WriteInt(minutes);
    GotoXY(13,15);
    WriteString(":", LIME);
    GotoXY(5,15);
    WriteString("Time: ", LIME);
    if (seconds < 10) {
        GotoXY(14, 15);
        WriteInt(0);
        GotoXY(15, 15);
    }
    else {
        GotoXY(14, 15);
    }

    WriteInt(seconds);
}

// Функция таймера

void timer_on() {
    while (1) {
        char buttons = detect_buttons();
        if (buttons & BUTTON_TIMER_OFF) {
            break;
        }
        erase_prev_time();
        draw_time();
        seconds++;
        if (seconds == 60) {
            seconds = 0;
            minutes++;
        }

        _delay_ms(500);
    }
}

// Функция рисования фона

void DrawBackground() {
    FillRect(0, 0, XSIZE, YSIZE, background_color);
}


void main() {
    DDRB = 0x00;
    PORTB = 0xFF;
    DDRD = 0xFF;
    PORTD = 0xFF;

    InitTFT();
    DrawBackground();
    DrawClockFace();
    draw_time();

    while (1) {
        char buttons = detect_buttons();
        if (buttons & BUTTON_TIMER_ON) {
            timer_on();
        }
        else if (buttons & BUTTON_TIMER_NULL) {
            minutes = 0;
            seconds = 0;
            erase_prev_time();
            draw_time();
        }

        _delay_ms(10);
    }
}