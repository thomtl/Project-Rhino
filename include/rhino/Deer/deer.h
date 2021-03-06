#pragma once
#include <rhino/common.h>
#include <rhino/Deer/deertext.h>
#include <rhino/Deer/deergui.h>

#include <rhino/udi/udi.h>

#define deer_clear_screen() (deer_clear_screen_col(0, 0, 0))

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

void deer_clear_screen_col(uint8_t r, uint8_t g, uint8_t b);
void deer_start();
