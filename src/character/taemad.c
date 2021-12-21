/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "taemad.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//taemad character structure
enum
{
	taemad_ArcMain_Idle0,
	taemad_ArcMain_Idle1,
	taemad_ArcMain_Idle2,
	taemad_ArcMain_Idle3,
	taemad_ArcMain_Left0,
	taemad_ArcMain_Left1,
	taemad_ArcMain_Left2,
	taemad_ArcMain_Down0,
	taemad_ArcMain_Down1,
	taemad_ArcMain_Down2,
	taemad_ArcMain_Up0,
	taemad_ArcMain_Up1,
	taemad_ArcMain_Up2,
	taemad_ArcMain_Right0,
	taemad_ArcMain_Right1,
	taemad_ArcMain_Right2,
	taemad_ArcMain_UpAlt0,
	taemad_ArcMain_UpAlt1,
	taemad_ArcMain_UpAlt2,
	taemad_ArcMain_UpAlt3,
	taemad_ArcMain_UpAlt4,
	taemad_ArcMain_UpAlt5,
	taemad_ArcMain_UpAlt6,
	taemad_ArcMain_UpAlt7,
	taemad_ArcMain_UpAlt8,
	taemad_ArcMain_UpAlt9,
	
	taemad_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[taemad_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_taemad;

//taemad character definitions
static const CharFrame char_taemad_frame[] = {
	{taemad_ArcMain_Idle0, {  0,   0, 174, 200}, { 115, 200}}, //0 idle 1
	{taemad_ArcMain_Idle1, {  0,   0, 174, 202}, { 116, 202}}, //1 idle 2
	{taemad_ArcMain_Idle2, {  0,   0, 173, 203}, { 115, 203}}, //2 idle 3
	{taemad_ArcMain_Idle3, {  0,   0, 173, 203}, { 115, 203}}, //3 idle 4
	
	{taemad_ArcMain_Left0, {  0,   0, 173, 202}, { 128, 202}}, //4 left 1
	{taemad_ArcMain_Left1, {  0,   0, 174, 202}, { 126, 202}}, //5 left 2
	{taemad_ArcMain_Left2, {  0,   0, 174, 201}, { 126, 201}}, //6 left 3

	{taemad_ArcMain_Down0, {  0,   0, 174, 187}, { 114, 187}}, //7 down 1
	{taemad_ArcMain_Down1, {  0,   0, 174, 186}, { 114, 186}}, //8 down 2
	{taemad_ArcMain_Down2, {  0,   0, 173, 187}, { 113, 187}}, //9 down 3
	
	{taemad_ArcMain_Up0, {  0,   0, 171, 208}, { 113, 208}}, //10 up 1
	{taemad_ArcMain_Up1, {  0,   0, 174, 205}, { 119, 205}}, //11 up 2
	{taemad_ArcMain_Up2, {  0,   0, 173, 206}, { 119, 206}}, //12 up 3
	
	{taemad_ArcMain_Right0, {  0,   0, 175, 191}, { 104, 191}}, //13 right 1
	{taemad_ArcMain_Right1, {  0,   0, 175, 190}, { 99, 190}}, //14 right 2
	{taemad_ArcMain_Right2, {  0,   0, 176, 189}, { 96, 189}}, //15 right 3

	{taemad_ArcMain_UpAlt0, {  0,   0, 174, 203}, { 115, 203}}, //16 0
	{taemad_ArcMain_UpAlt1, {  0,   0, 173, 203}, { 115, 203}}, //17 1
	{taemad_ArcMain_UpAlt2, {  0,   0, 173, 203}, { 115, 203}}, //18 2
	{taemad_ArcMain_UpAlt3, {  0,   0, 173, 207}, { 117, 207}}, //19 3
	{taemad_ArcMain_UpAlt4, {  0,   0, 173, 187}, { 115, 187}}, //20 4
	{taemad_ArcMain_UpAlt5, {  0,   0, 174, 187}, { 116, 187}}, //21 5
	{taemad_ArcMain_UpAlt6, {  0,   0, 173, 187}, { 115, 187}}, //22 6
	{taemad_ArcMain_UpAlt7, {  0,   0, 173, 187}, { 115, 187}}, //23 7
	{taemad_ArcMain_UpAlt8, {  0,   0, 176, 200}, { 107, 200}}, //24 8
	{taemad_ArcMain_UpAlt9, {  0,   0, 174, 198}, { 110, 191}}, //25 9

};

static const Animation char_taemad_anim[CharAnim_Max] = {
	{3, (const u8[]){ 0, 1, 2, 3, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 4, 5, 6, ASCR_BACK, 0}},         //CharAnim_Left
	{3, (const u8[]){ 20,  20, 23,  24, 24,  25,  25, ASCR_BACK, 0}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 7, 8, 9, ASCR_BACK, 0}},         //CharAnim_Down
	{3, (const u8[]){ 21,  22,  22,  22,  23, 23, ASCR_BACK, 0}},   //CharAnim_DownAlt
	{2, (const u8[]){ 10, 11, 12, ASCR_BACK, 0}},         //CharAnim_Up
	{3, (const u8[]){ 16,  16,  17, 17,  17,  18,  18, ASCR_BACK, 0}},   //CharAnim_UpAlt
	{2, (const u8[]){ 13, 14, 15, ASCR_BACK, 0}},         //CharAnim_Right
	{3, (const u8[]){ 18,  19,  19, 19,  19,  20,   ASCR_BACK, 0}},   //CharAnim_RightAlt
};

//taemad character functions
void Char_taemad_SetFrame(void *user, u8 frame)
{
	Char_taemad *this = (Char_taemad*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_taemad_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_taemad_Tick(Character *character)
{
	Char_taemad *this = (Char_taemad*)character;
	
	if (stage.stage_id == StageId_1_3 && stage.song_step > 1761) {
		this->character.focus_zoom = FIXED_DEC(15,10);
	}
	if (stage.stage_id == StageId_1_3 && stage.song_step > 1780) {
		this->character.focus_zoom = FIXED_DEC(8,10);
	}

	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_taemad_SetFrame);
	Character_Draw(character, &this->tex, &char_taemad_frame[this->frame]);

	if (stage.stage_id == StageId_1_3 && stage.song_step == 1761) {
		character->set_anim(character, CharAnim_UpAlt);
	}

	if (stage.stage_id == StageId_1_3 && stage.song_step == 1765) {
		character->set_anim(character, CharAnim_RightAlt);
	}

	if (stage.stage_id == StageId_1_3 && stage.song_step == 1772) {
		character->set_anim(character, CharAnim_DownAlt);
	}

	if (stage.stage_id == StageId_1_3 && stage.song_step == 1780) {
		character->set_anim(character, CharAnim_LeftAlt);
	}
	
}

void Char_taemad_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_taemad_Free(Character *character)
{
	Char_taemad *this = (Char_taemad*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_taemad_New(fixed_t x, fixed_t y)
{
	//Allocate taemad object
	Char_taemad *this = Mem_Alloc(sizeof(Char_taemad));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_taemad_New] Failed to allocate taemad object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_taemad_Tick;
	this->character.set_anim = Char_taemad_SetAnim;
	this->character.free = Char_taemad_Free;
	
	Animatable_Init(&this->character.animatable, char_taemad_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 2;
	
	this->character.focus_x = FIXED_DEC(0,1);
	this->character.focus_y = FIXED_DEC(-125,1);
	this->character.focus_zoom = FIXED_DEC(8,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\TAEMAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //taemad_ArcMain_Idle0
		"idle1.tim", //taemad_ArcMain_Idle1
		"idle2.tim", //taemad_ArcMain_Idle1
		"idle3.tim", //taemad_ArcMain_Idle
		"left0.tim",  //taemad_ArcMain_Left
		"left1.tim",  //taemad_ArcMain_Left
		"left2.tim",  //taemad_ArcMain_Left
		"down0.tim",  //taemad_ArcMain_Down
		"down1.tim",  //taemad_ArcMain_Down
		"down2.tim",  //taemad_ArcMain_Down
		"up0.tim",    //taemad_ArcMain_Up
		"up1.tim",    //taemad_ArcMain_Up
		"up2.tim",    //taemad_ArcMain_Up
		"right0.tim", //taemad_ArcMain_Right
		"right1.tim", //taemad_ArcMain_Right
		"right2.tim", //taemad_ArcMain_Right
		"speak0.tim", //taemad_ArcMain_Right	
		"speak1.tim", //taemad_ArcMain_Right
		"speak2.tim", //taemad_ArcMain_Right
		"speak3.tim", //taemad_ArcMain_Right
		"speak4.tim", //taemad_ArcMain_Right
		"speak5.tim", //taemad_ArcMain_Right
		"speak6.tim", //taemad_ArcMain_Right
		"speak7.tim", //taemad_ArcMain_Right
		"speak8.tim", //taemad_ArcMain_Right
		"speak9.tim", //taemad_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
