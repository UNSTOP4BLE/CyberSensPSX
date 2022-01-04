/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "sanz.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend skull fragments
static SkullFragment char_sanz_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend player types
enum
{
	sanz_ArcMain_Idle0,
	sanz_ArcMain_Idle1,
	sanz_ArcMain_Left0, //Up Right
	sanz_ArcMain_Down0,  //Left Down
	sanz_ArcMain_Up0,  //Up Right
	sanz_ArcMain_Right0,
	sanz_ArcMain_Right1,
	sanz_ArcMain_Eye,
	sanz_ArcMain_Dead0, //BREAK
	
	sanz_ArcMain_Max,
};

enum
{
	sanz_ArcDead_Dead1, //Mic Drop
	sanz_ArcDead_Dead2, //Twitch
	sanz_ArcDead_Retry, //Retry prompt
	
	sanz_ArcDead_Max,
};

#define sanz_Arc_Max sanz_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[sanz_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_sanz_skull)];
	u8 skull_scale;
} Char_sanz;

//Boyfriend player definitions
static const CharFrame char_sanz_frame[] = {
	{sanz_ArcMain_Idle0, {  0,   0, 103, 153}, { 99,  153}}, //0 idle 1
	{sanz_ArcMain_Idle0, {103,   0, 103, 153}, { 100, 153}}, //1 idle 2
	{sanz_ArcMain_Idle1, {  0,   2, 104, 151}, { 101,  151}}, //2 idle 3
	{sanz_ArcMain_Idle1, {104,   3, 104, 150}, { 101,  150}}, //3 idle 4

	{sanz_ArcMain_Left0, {  0,   0, 107, 149}, { 107, 149}}, //8 left
	{sanz_ArcMain_Left0, {107,   1, 109, 148}, { 109, 148}}, //9 left
	
	{sanz_ArcMain_Down0,  {  0,  0, 105, 140}, { 105,  140}}, //4 down 1
	{sanz_ArcMain_Down0,  {105,  1, 107, 139}, { 107,  139}}, //5 down 2

	{sanz_ArcMain_Up0,  {  0,   0, 102, 163}, { 97, 163}}, //6 up 1
	{sanz_ArcMain_Up0,  {102,   0, 99,  163}, { 95, 163}}, //7 up 2
	
	{sanz_ArcMain_Right0,  {  0,   0, 145, 153}, { 101,  153}}, //10 right 
	{sanz_ArcMain_Right1,  {  0,   0, 143, 154}, { 100,  154}}, //11 right 
	
	{sanz_ArcMain_Eye, {  0,   0, 104, 153}, { 101,  153}}, //12 eye
	
	{sanz_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //13 dead0 0
	{sanz_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //14 dead0 1
	{sanz_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //15 dead0 2
	{sanz_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //16 dead0 3
	
	{sanz_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //17 dead1 0
	{sanz_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //18 dead1 1
	{sanz_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //19 dead1 2
	{sanz_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //20 dead1 3
	
	{sanz_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //21 dead2 body twitch 0
	{sanz_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //22 dead2 body twitch 1
	{sanz_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //23 dead2 balls twitch 0
	{sanz_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //24 dead2 balls twitch 1
};

static const Animation char_sanz_anim[PlayerAnim_Max] = {
	{3, (const u8[]){ 3,  2,  1,  0, ASCR_BACK, 0}}, //CharAnim_Idle
	{3, (const u8[]){ 5,  4, ASCR_BACK, 0}},             //CharAnim_Down
	{3, (const u8[]){ 5,  4, ASCR_BACK, 0}},             //CharAnim_DownAlt
	{3, (const u8[]){ 7, 6, ASCR_BACK, 0}},             //CharAnim_Up
	{3, (const u8[]){ 7, 6, ASCR_BACK, 0}},     //CharAnim_UpAlt
	{3, (const u8[]){ 9,  8, ASCR_BACK, 0}},             //CharAnim_Left
	{3, (const u8[]){ 4,  6,  6,  6, ASCR_BACK, 0}},     //CharAnim_LeftAlt
	{3, (const u8[]){11, 10, ASCR_BACK, 0}},             //CharAnim_Right
	{3, (const u8[]){11, 10, ASCR_BACK, 0}},     //CharAnim_RightAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{3, (const u8[]){12, ASCR_BACK, 0}},         //PlayerAnim_Peace
	{3, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //PlayerAnim_Sweat
	
	{5, (const u8[]){13, 14, 15, 16, 16, 16, 16, 16, 16, 16, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){16, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){17, 18, 19, 20, 20, 20, 20, 20, 20, 20, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){20, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){21, 22, 20, 20, 20, 20, 20, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){23, 24, 20, 20, 20, 20, 20, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){20, 20, 20, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){23, 24, 20, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_sanz_SetFrame(void *user, u8 frame)
{
	Char_sanz *this = (Char_sanz*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_sanz_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_sanz_Tick(Character *character)
{
	Char_sanz *this = (Char_sanz*)character;
	if (stage.stage_id == StageId_1_4 && stage.song_step == 948) {
		this->character.health_i = 4;
	}
	if (stage.stage_id == StageId_1_4 && stage.song_step == 951) {
		this->character.health_i = 3;
	}
	
	if (stage.stage_id == StageId_1_4 && stage.song_step == 948) {
		character->set_anim(character, PlayerAnim_Peace);
	}

	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_Right))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
		

	}
	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_sanz, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(56,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_sanz_SetFrame);
	Character_Draw(character, &this->tex, &char_sanz_frame[this->frame]);
}

void Char_sanz_SetAnim(Character *character, u8 anim)
{
	Char_sanz *this = (Char_sanz*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;
			
			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //sanz_ArcDead_Dead1
				"dead2.tim", //sanz_ArcDead_Dead2
				"retry.tim", //sanz_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[sanz_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_sanz_Free(Character *character)
{
	Char_sanz *this = (Char_sanz*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_sanz_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_sanz *this = Mem_Alloc(sizeof(Char_sanz));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_sanz_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_sanz_Tick;
	this->character.set_anim = Char_sanz_SetAnim;
	this->character.free = Char_sanz_Free;
	
	Animatable_Init(&this->character.animatable, char_sanz_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;

	this->character.health_i = 3;

	this->character.focus_x = FIXED_DEC(-90,1);
	this->character.focus_y = FIXED_DEC(-90,1);
	this->character.focus_zoom = FIXED_DEC(8,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SANZ.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\SANZDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",  //sanz_ArcMain_Idle
		"idle1.tim",  //sanz_ArcMain_Idle
		"left.tim",  //sanz_ArcMain_Hit0
		"down.tim",  //sanz_ArcMain_Hit0
		"up.tim",  //sanz_ArcMain_Hit0
		"right0.tim",  //sanz_ArcMain_Hit0
		"right1.tim",  //sanz_ArcMain_Hit0
		"eye.tim", //sanz_ArcMain_Peace
		"dead0.tim", //sanz_ArcMain_Dead0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_sanz_skull, sizeof(char_sanz_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_sanz, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
