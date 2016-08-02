/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez
 * 		 2016  Sergi Granell

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <cstdlib>
#include "gearboy.h"

#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>

#include <vita2d.h>

#define SCREEN_W 940
#define SCREEN_H 544

struct palette_color {
	int red;
	int green;
	int blue;
	int alpha;
};

static bool running = true;
static bool paused = false;
static bool fullscreen = false;
static int scale = 4;
static int gb_draw_x = 0;
static int gb_draw_y = 0;
static float fullscreen_scale_x = 0.0f;
static float fullscreen_scale_y = 0.0f;

static const char *output_file = "gearboy.cfg";

static GearboyCore *theGearboyCore;
static GB_Color *theFrameBuffer;
static vita2d_texture *gb_texture;
static void *gb_texture_pixels;

static palette_color dmg_palette[4];

static void set_scale(int new_scale)
{
	scale = new_scale;
	gb_draw_x = SCREEN_W / 2 - (GAMEBOY_WIDTH / 2) * scale;
	gb_draw_y = SCREEN_H / 2 - (GAMEBOY_HEIGHT / 2) * scale;
}

static void init_palette(void)
{
	dmg_palette[0].red = 0xEF;
	dmg_palette[0].green = 0xF3;
	dmg_palette[0].blue = 0xD5;
	dmg_palette[0].alpha = 0xFF;

	dmg_palette[1].red = 0xA3;
	dmg_palette[1].green = 0xB6;
	dmg_palette[1].blue = 0x7A;
	dmg_palette[1].alpha = 0xFF;

	dmg_palette[2].red = 0x37;
	dmg_palette[2].green = 0x61;
	dmg_palette[2].blue = 0x3B;
	dmg_palette[2].alpha = 0xFF;

	dmg_palette[3].red = 0x04;
	dmg_palette[3].green = 0x1C;
	dmg_palette[3].blue = 0x16;
	dmg_palette[3].alpha = 0xFF;
}

static void init(void)
{
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));
	init_palette();

	fullscreen_scale_x = SCREEN_W / (float)GAMEBOY_WIDTH;
	fullscreen_scale_y = SCREEN_H / (float)GAMEBOY_HEIGHT;

	set_scale(3);

	gb_texture = vita2d_create_empty_texture(GAMEBOY_WIDTH, GAMEBOY_HEIGHT);
	gb_texture_pixels = vita2d_texture_get_datap(gb_texture);

	theGearboyCore = new GearboyCore();
	theGearboyCore->Init();
}

static void end(void)
{
	SafeDeleteArray(theFrameBuffer);
	SafeDelete(theGearboyCore);
	vita2d_fini();
	vita2d_free_texture(gb_texture);
}

static void update(void)
{
	vita2d_start_drawing();
	vita2d_clear_screen();

	theGearboyCore->RunToVBlank((GB_Color *)gb_texture_pixels);

	if (fullscreen) {
		vita2d_draw_texture_scale(gb_texture, 0, 0,
			fullscreen_scale_x, fullscreen_scale_y);

	} else {
		vita2d_draw_texture_scale(gb_texture,
			gb_draw_x, gb_draw_y, scale, scale);
	}

	vita2d_end_drawing();
	vita2d_swap_buffers();
}

#define ROM_PATH "ux0:ROMS/Pokemon - Silver Version.gbc"

int main(int argc, char** argv)
{
	init();

	bool forcedmg = false;
	bool nosound = false;

	if (theGearboyCore->LoadROM(ROM_PATH, forcedmg)) {
		GB_Color color1;
		GB_Color color2;
		GB_Color color3;
		GB_Color color4;

		color1.red = dmg_palette[0].red;
		color1.green = dmg_palette[0].green;
		color1.blue = dmg_palette[0].blue;
		color2.red = dmg_palette[1].red;
		color2.green = dmg_palette[1].green;
		color2.blue = dmg_palette[1].blue;
		color3.red = dmg_palette[2].red;
		color3.green = dmg_palette[2].green;
		color3.blue = dmg_palette[2].blue;
		color4.red = dmg_palette[3].red;
		color4.green = dmg_palette[3].green;
		color4.blue = dmg_palette[3].blue;

		theGearboyCore->SetDMGPalette(color1, color2, color3, color4);
		theGearboyCore->EnableSound(!nosound);
		theGearboyCore->LoadRam();

		while (running) {
			update();
		}

		theGearboyCore->SaveRam();
	}

	end();

	sceKernelExitProcess(0);
	return 0;
}
