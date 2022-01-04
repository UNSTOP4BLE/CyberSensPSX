/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "opo.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Option Options character structure
enum
{
	OPO_ArcMain_OPO,
	
	OPO_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[OPO_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
} Char_OPO;

//Option Options character definitions
static const CharFrame char_opo_frame[] = {
	{OPO_ArcMain_OPO, {  0,  70,  94,  70}, { 39,  73}}, //1 bop left 2
	{OPO_ArcMain_OPO, { 96,  70,  94,  70}, { 39,  73}}, //1 bop left 3

};

static const Animation char_opo_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,   ASCR_CHGANI, 0}},                         //CharAnim_Idle
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  4,  4,  5, ASCR_BACK, 1}}, //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_LeftAlt
	{2, (const u8[]){12, 13, ASCR_REPEAT}},                                  //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_UpAlt
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9, 10, 10, 11, ASCR_BACK, 1}}, //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_RightAlt
};

//Option Options character functions
void Char_OPO_SetFrame(void *user, u8 frame)
{
	Char_OPO *this = (Char_OPO*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_opo_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_OPO_Tick(Character *character)
{
	Char_OPO *this = (Char_OPO*)character;
	
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
	Animatable_Animate(&character->animatable, (void*)this, Char_OPO_SetFrame);
	Character_Draw(character, &this->tex, &char_opo_frame[this->frame]);
}

void Char_OPO_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_OPO_Free(Character *character)
{
	Char_OPO *this = (Char_OPO*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_OPO_New(fixed_t x, fixed_t y)
{
	//Allocate opo object
	Char_OPO *this = Mem_Alloc(sizeof(Char_OPO));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_OPO_New] Failed to allocate opo object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_OPO_Tick;
	this->character.set_anim = Char_OPO_SetAnim;
	this->character.free = Char_OPO_Free;
	
	Animatable_Init(&this->character.animatable, char_opo_anim);
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
		"opo.tim",  //OPO_ArcMain_OPO
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
