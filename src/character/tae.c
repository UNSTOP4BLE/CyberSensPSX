/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "tae.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//tae character structure
enum
{
	tae_ArcMain_Idle0,
	tae_ArcMain_Idle1,
	tae_ArcMain_Idle2,
	tae_ArcMain_Left0,
	tae_ArcMain_Left1,
	tae_ArcMain_Down0,
	tae_ArcMain_Down1,
	tae_ArcMain_Down2,
	tae_ArcMain_Down3,
	tae_ArcMain_Up0,
	tae_ArcMain_Up1,
	tae_ArcMain_Right0,
	tae_ArcMain_Right1,

	
	tae_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[tae_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_tae;

//tae character definitions
static const CharFrame char_tae_frame[] = {
	{tae_ArcMain_Idle0, {  0,   0, 122, 199}, { 115, 199}}, //0 idle 1
	{tae_ArcMain_Idle0, {  133,   0, 123, 199}, { 115, 199}}, //1 idle 2
	{tae_ArcMain_Idle1, {  0,   0, 122, 201}, { 116, 201}}, //2 idle 3
	{tae_ArcMain_Idle1, {  132,   0, 124, 201}, { 116, 201}}, //3 idle 4
	{tae_ArcMain_Idle2, {  0,   0, 125, 201}, { 116, 201}}, //4 idle 5
	{tae_ArcMain_Idle2, {  130,   0, 125, 201}, { 116, 201}}, //5 idle 6
	
	{tae_ArcMain_Left0, {  0,   0, 124, 200}, { 122, 200}}, //6 left 1
	{tae_ArcMain_Left0, {  132,   0, 124, 200}, { 122, 200}}, //7 left 2
	{tae_ArcMain_Left1, {  0,   0, 124, 201}, { 123, 201}}, //8 left 3
	{tae_ArcMain_Left1, {  131,   1, 125, 200}, { 122, 200}}, //9 left 4

	{tae_ArcMain_Down0, {  0,   0, 143, 170}, { 131, 170}}, //10 down 1
	{tae_ArcMain_Down1, {  0,   0, 142, 174}, { 129, 174}}, //11 down 2
	{tae_ArcMain_Down2, {  0,   0, 142, 176}, { 129, 176}}, //12 down 3
	{tae_ArcMain_Down3, {  0,   0, 142, 176}, { 129, 176}}, //13 down 4
	
	{tae_ArcMain_Up0, {  0,   0, 127, 211}, { 112, 211}}, //14 up 1
	{tae_ArcMain_Up0, {  130,   0, 126, 211}, { 112, 211}}, //15 up 2
	{tae_ArcMain_Up1, {  0,   0, 127, 210}, { 112, 210}}, //16 up 3
	{tae_ArcMain_Up1, {  130,   1, 126, 209}, { 112, 209}}, //17 up 4
	
	{tae_ArcMain_Right0, {  0,   0, 107, 195}, { 86, 195}}, //18 right 1
	{tae_ArcMain_Right0, {  107,   2, 107, 193}, { 86, 193}}, //19 right 2
	{tae_ArcMain_Right1, {  0,   1, 107, 193}, { 85, 193}}, //20 right 3
	{tae_ArcMain_Right1, {  107,   0, 106, 194}, { 85, 194}}, //21 right 4
};

static const Animation char_tae_anim[CharAnim_Max] = {
	{3, (const u8[]){ 0, 1, 2, 3, 4, 5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 6, 7, 8, 9, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 10, 11, 12, 13, ASCR_BACK, 0}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 14, 15, 16, 17, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 18, 19, 20, 21, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//tae character functions
void Char_tae_SetFrame(void *user, u8 frame)
{
	Char_tae *this = (Char_tae*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_tae_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_tae_Tick(Character *character)
{
	Char_tae *this = (Char_tae*)character;

	if (stage.stage_id == StageId_1_2 && stage.song_step > -20) {
		this->character.focus_zoom = FIXED_DEC(15,10);
	}
	if (stage.stage_id == StageId_1_2 && stage.song_step > 120) {
		this->character.focus_zoom = FIXED_DEC(8,10);
	}
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_tae_SetFrame);
	Character_Draw(character, &this->tex, &char_tae_frame[this->frame]);
}

void Char_tae_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_tae_Free(Character *character)
{
	Char_tae *this = (Char_tae*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_tae_New(fixed_t x, fixed_t y)
{
	//Allocate tae object
	Char_tae *this = Mem_Alloc(sizeof(Char_tae));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_tae_New] Failed to allocate tae object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_tae_Tick;
	this->character.set_anim = Char_tae_SetAnim;
	this->character.free = Char_tae_Free;
	
	Animatable_Init(&this->character.animatable, char_tae_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(0,1);
	this->character.focus_y = FIXED_DEC(-125,1);
	this->character.focus_zoom = FIXED_DEC(8,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\TAE.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //tae_ArcMain_Idle0
		"idle1.tim", //tae_ArcMain_Idle1
		"idle2.tim", //tae_ArcMain_Idle1
		"left0.tim",  //tae_ArcMain_Left
		"left1.tim",  //tae_ArcMain_Left
		"down0.tim",  //tae_ArcMain_Down
		"down1.tim",  //tae_ArcMain_Down
		"down2.tim",  //tae_ArcMain_Down
		"down3.tim",  //tae_ArcMain_Down
		"up0.tim",    //tae_ArcMain_Up
		"up1.tim",    //tae_ArcMain_Up
		"right0.tim", //tae_ArcMain_Right
		"right1.tim", //tae_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
