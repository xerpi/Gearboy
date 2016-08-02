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

#define SCREEN_W 960
#define SCREEN_H 544

#define EXIT_MASK        (SCE_CTRL_LTRIGGER | SCE_CTRL_SQUARE)
#define CHANGE_GAME_MASK (SCE_CTRL_LTRIGGER | SCE_CTRL_TRIANGLE)
#define FULLSCREEN_MASK  (SCE_CTRL_SQUARE)
#define TURBO_MASK       (SCE_CTRL_RTRIGGER)
#define JOY_THRESHOLD    110

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

static SceCtrlData pad;
static SceCtrlData old_pad;

static const struct {
	unsigned int vita;
	Gameboy_Keys gb;
} key_map[] = {
	{SCE_CTRL_CROSS,  A_Key},
	{SCE_CTRL_CIRCLE, B_Key},
	{SCE_CTRL_SELECT, Select_Key},
	{SCE_CTRL_START,  Start_Key},
	{SCE_CTRL_UP,     Up_Key},
	{SCE_CTRL_DOWN,   Down_Key},
	{SCE_CTRL_LEFT,   Left_Key},
	{SCE_CTRL_RIGHT,  Right_Key},
};

#define KEY_MAP_SIZE (sizeof(key_map) / sizeof(*key_map))

static void set_scale(int new_scale)
{
	scale = new_scale;
	gb_draw_x = SCREEN_W / 2 - (GAMEBOY_WIDTH / 2) * scale;
	gb_draw_y = SCREEN_H / 2 - (GAMEBOY_HEIGHT / 2) * scale;
}

static void init(void)
{
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));

	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

	fullscreen_scale_x = SCREEN_W / (float)GAMEBOY_WIDTH;
	fullscreen_scale_y = SCREEN_H / (float)GAMEBOY_HEIGHT;

	gb_texture = vita2d_create_empty_texture(GAMEBOY_WIDTH, GAMEBOY_HEIGHT);
	gb_texture_pixels = vita2d_texture_get_datap(gb_texture);

	set_scale(3);

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

static void update_input(void)
{
	unsigned int i;
	unsigned int pressed;
	unsigned int released;

	sceCtrlPeekBufferPositive(0, &pad, 1);

	if (pad.lx < (128 - JOY_THRESHOLD))
		pad.buttons |= SCE_CTRL_LEFT;
	else if (pad.lx > (128 + JOY_THRESHOLD))
		pad.buttons |= SCE_CTRL_RIGHT;

	if (pad.ly < (128 - JOY_THRESHOLD))
		pad.buttons |= SCE_CTRL_UP;
	else if (pad.ly > (128 + JOY_THRESHOLD))
		pad.buttons |= SCE_CTRL_DOWN;

	pressed = pad.buttons & ~old_pad.buttons;
	released = ~pad.buttons & old_pad.buttons;

	if ((pad.buttons & EXIT_MASK) == EXIT_MASK) {
		running = false;
	} else if ((pressed & FULLSCREEN_MASK) == FULLSCREEN_MASK) {
		fullscreen = !fullscreen;
	} else if ((pressed & TURBO_MASK) == TURBO_MASK) {
		vita2d_set_vblank_wait(0);
	} else if ((released & TURBO_MASK) == TURBO_MASK) {
		vita2d_set_vblank_wait(1);
	}

	for (i = 0; i < KEY_MAP_SIZE; i++) {
		if (pressed & key_map[i].vita)
			theGearboyCore->KeyPressed(key_map[i].gb);
		else if (released & key_map[i].vita)
			theGearboyCore->KeyReleased(key_map[i].gb);
	}

	old_pad = pad;
}

static void update(void)
{
	vita2d_start_drawing();
	vita2d_clear_screen();

	update_input();

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
