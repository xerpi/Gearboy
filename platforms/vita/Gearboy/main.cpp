/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

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

#define SCREEN_WIDTH 940
#define SCREEN_HEIGHT 544

struct palette_color {
	int red;
	int green;
	int blue;
	int alpha;
};

static bool running = true;
static bool paused = false;

static const char *output_file = "gearboy.cfg";

static const float kGB_Width = 160.0f;
static const float kGB_Height = 144.0f;
static const float kGB_TexWidth = kGB_Width / 256.0f;
static const float kGB_TexHeight = kGB_Height / 256.0f;

static GearboyCore *theGearboyCore;
static GB_Color *theFrameBuffer;
static vita2d_texture *gb_texture;
static void *gb_texture_pixels;

static palette_color dmg_palette[4];
static uint32_t screen_width, screen_height;

static void init_sdl(void)
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
	init_sdl();

	gb_texture = vita2d_create_empty_texture(GAMEBOY_WIDTH, GAMEBOY_HEIGHT);
	gb_texture_pixels = vita2d_texture_get_datap(gb_texture);

	theGearboyCore = new GearboyCore();
	theGearboyCore->Init();
}

static void end(void)
{
	/*Config cfg;

	Setting &root = cfg.getRoot();
	Setting &address = root.add("Gearboy", Setting::TypeGroup);

	address.add("keypad_left", Setting::TypeString) = SDL_GetKeyName(kc_keypad_left);
	address.add("keypad_right", Setting::TypeString) = SDL_GetKeyName(kc_keypad_right);
	address.add("keypad_up", Setting::TypeString) = SDL_GetKeyName(kc_keypad_up);
	address.add("keypad_down", Setting::TypeString) = SDL_GetKeyName(kc_keypad_down);
	address.add("keypad_a", Setting::TypeString) = SDL_GetKeyName(kc_keypad_a);
	address.add("keypad_b", Setting::TypeString) = SDL_GetKeyName(kc_keypad_b);
	address.add("keypad_start", Setting::TypeString) = SDL_GetKeyName(kc_keypad_start);
	address.add("keypad_select", Setting::TypeString) = SDL_GetKeyName(kc_keypad_select);

	address.add("joystick_gamepad_a", Setting::TypeInt) = jg_a;
	address.add("joystick_gamepad_b", Setting::TypeInt) = jg_b;
	address.add("joystick_gamepad_start", Setting::TypeInt) = jg_start;
	address.add("joystick_gamepad_select", Setting::TypeInt) = jg_select;
	address.add("joystick_gamepad_x_axis", Setting::TypeInt) = jg_x_axis;
	address.add("joystick_gamepad_y_axis", Setting::TypeInt) = jg_y_axis;
	address.add("joystick_gamepad_x_axis_invert", Setting::TypeBoolean) = jg_x_axis_invert;
	address.add("joystick_gamepad_y_axis_invert", Setting::TypeBoolean) = jg_y_axis_invert;

	address.add("emulator_pause", Setting::TypeString) = SDL_GetKeyName(kc_emulator_pause);
	address.add("emulator_quit", Setting::TypeString) = SDL_GetKeyName(kc_emulator_quit);

	address.add("emulator_pal0_red", Setting::TypeInt) = dmg_palette[0].red;
	address.add("emulator_pal0_green", Setting::TypeInt) = dmg_palette[0].green;
	address.add("emulator_pal0_blue", Setting::TypeInt) = dmg_palette[0].blue;
	address.add("emulator_pal1_red", Setting::TypeInt) = dmg_palette[1].red;
	address.add("emulator_pal1_green", Setting::TypeInt) = dmg_palette[1].green;
	address.add("emulator_pal1_blue", Setting::TypeInt) = dmg_palette[1].blue;
	address.add("emulator_pal2_red", Setting::TypeInt) = dmg_palette[2].red;
	address.add("emulator_pal2_green", Setting::TypeInt) = dmg_palette[2].green;
	address.add("emulator_pal2_blue", Setting::TypeInt) = dmg_palette[2].blue;
	address.add("emulator_pal3_red", Setting::TypeInt) = dmg_palette[3].red;
	address.add("emulator_pal3_green", Setting::TypeInt) = dmg_palette[3].green;
	address.add("emulator_pal3_blue", Setting::TypeInt) = dmg_palette[3].blue;

	cfg.writeFile(output_file);*/

	//SDL_JoystickClose(game_pad);

	SafeDeleteArray(theFrameBuffer);
	SafeDelete(theGearboyCore);
	vita2d_fini();
	vita2d_free_texture(gb_texture);

	/*SDL_DestroyWindow(theWindow);
	SDL_Quit();
	bcm_host_deinit();*/
}

static void update(void)
{
	vita2d_start_drawing();
	vita2d_clear_screen();

	theGearboyCore->RunToVBlank(gb_texture_pixels);

	vita2d_draw_texture(gb_texture, 0, 0);

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
