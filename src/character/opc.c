/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "opc.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

///Option Credits & FreePlay character structure
enum
{
	OPC_ArcMain_OPC0,
	
	OPC_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[OPC_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
} Char_OPC;

///Option Credits & FreePlay character definitions
static const CharFrame char_opc_frame[] = {
	{OPC_ArcMain_OPC0, {  0,   0, 100,  51}, { 39,  73}}, //0 bop left 1
	{OPC_ArcMain_OPC0, { 99,   0, 100,  51}, { 39,  73}}, //1 bop left 2
	{OPC_ArcMain_OPC0, {  0,  52, 100,  50}, { 39,  73}}, //1 bop left 2

};

static const Animation char_opc_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  ASCR_BACK, 1}},                         //CharAnim_Idle
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  4,  4,  5, ASCR_BACK, 1}}, //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_LeftAlt
	{2, (const u8[]){12, 13, ASCR_REPEAT}},                                  //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_UpAlt
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9, 10, 10, 11, ASCR_BACK, 1}}, //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_RightAlt
};

///Option Credits & FreePlay character functions
void Char_OPC_SetFrame(void *user, u8 frame)
{
	Char_OPC *this = (Char_OPC*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_opc_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_OPC_Tick(Character *character)
{
	Char_OPC *this = (Char_OPC*)character;
	
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
	{
		Character_CheckEndSing(character);
		
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			if (Animatable_Ended(&character->animatable) &&
				(character->animatable.anim != CharAnim_Left &&
				 character->animatable.anim != CharAnim_Down &&
				 character->animatable.anim != CharAnim_Up &&
				 character->animatable.anim != CharAnim_Right) &&
				(stage.song_step & 0x3) == 0)
				character->set_anim(character, CharAnim_Idle);
		}
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_OPC_SetFrame);
	Character_Draw(character, &this->tex, &char_opc_frame[this->frame]);
}

void Char_OPC_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_OPC_Free(Character *character)
{
	Char_OPC *this = (Char_OPC*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_OPC_New(fixed_t x, fixed_t y)
{
	//Allocate opc object
	Char_OPC *this = Mem_Alloc(sizeof(Char_OPC));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_OPC_New] Failed to allocate opc object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_OPC_Tick;
	this->character.set_anim = Char_OPC_SetAnim;
	this->character.free = Char_OPC_Free;
	
	Animatable_Init(&this->character.animatable, char_opc_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(16,1);
	this->character.focus_y = FIXED_DEC(-50,1);
	this->character.focus_zoom = FIXED_DEC(13,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\OPTION.ARC;1");
	
	const char **pathp = (const char *[]){
		"opc.tim",  //OPC_ArcMain_OPC
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
