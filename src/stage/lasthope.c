/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "lasthope.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

//lasthope background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	IO_Data arc_peko, arc_peko_ptr[1];
	Gfx_Tex tex_back0; //bg
	Gfx_Tex tex_back1; //bg

	//peko state
	Gfx_Tex tex_peko;
	u8 peko_frame, peko_tex_id;

	Animatable peko0_animatable;
	Animatable peko1_animatable;
	
} Back_lasthope;

//peko animation and rects
static const CharFrame peko_frame[1] = {
	{0, {0, 67, 128, 67}, { 128,  67}}, //right 3 (turned off)

};

static const Animation peko_anim[] = {
	{2, (const u8[]){0, 0, 0, 0, 0,  ASCR_BACK, 1}}, //Left

};
//peko functions
void lasthope_peko_SetFrame(void *user, u8 frame)
{
	Back_lasthope *this = (Back_lasthope*)user;
	
	//Check if this is a new frame
	if (frame != this->peko_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &peko_frame[this->peko_frame = frame];
		if (cframe->tex != this->peko_tex_id)
			Gfx_LoadTex(&this->tex_peko, this->arc_peko_ptr[this->peko_tex_id = cframe->tex], 0);
	}
}

void lasthope_peko_Draw(Back_lasthope *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &peko_frame[this->peko_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_peko, &src, &dst, stage.camera.bzoom);
}

void Back_lasthope_DrawFG(StageBack *back)
{
	Back_lasthope *this = (Back_lasthope*)back;

	fixed_t fx, fy;

	//peeko
	

	if (RandomRange(0, 1000) == 70) {
	stage.cooldown = 60;
	}

	if (stage.cooldown != 0)
	   lasthope_peko_Draw(this, FIXED_DEC(0,1), FIXED_DEC(43,1));

	Animatable_Animate(&this->peko0_animatable, (void*)this, lasthope_peko_SetFrame);
	}


void Back_lasthope_DrawMD(StageBack *back)
{
	Back_lasthope *this = (Back_lasthope*)back;
	
	fixed_t fx, fy;
}

void Back_lasthope_DrawBG(StageBack *back)
{
	Back_lasthope *this = (Back_lasthope*)back;
	
	fixed_t fx, fy;
	
	//Draw sunset
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT back_src = {0, 0, 256, 256};
	RECT_FIXED back_dst = {
		FIXED_DEC(132,1) - fx,
		FIXED_DEC(-125,1) - fy,
		FIXED_DEC(320,1),
		FIXED_DEC(426,1)
	};
	
	Stage_DrawTex(&this->tex_back1, &back_src, &back_dst, stage.camera.bzoom);

	//Draw background
	fx = stage.camera.x;
	fy = stage.camera.y;

	RECT back2_src = {0, 0, 256, 256};
	RECT_FIXED back2_dst = {
		FIXED_DEC(-185,1) - fx,
		FIXED_DEC(-125,1) - fy,
		FIXED_DEC(320,1),
		FIXED_DEC(426,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &back2_src, &back2_dst, stage.camera.bzoom);
}

void Back_lasthope_Free(StageBack *back)
{
	Back_lasthope *this = (Back_lasthope*)back;
	
	//Free smoke archive
	Mem_Free(this->arc_peko);

	//Free structure
	Mem_Free(this);
}

StageBack *Back_lasthope_New(void)
{
	//Allocate background structure
	Back_lasthope *this = (Back_lasthope*)Mem_Alloc(sizeof(Back_lasthope));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_lasthope_DrawFG;
	this->back.draw_md = Back_lasthope_DrawMD;
	this->back.draw_bg = Back_lasthope_DrawBG;
	this->back.free = Back_lasthope_Free;
	
	//Load background textures
	//Load tv0 textures
	this->arc_peko = IO_Read("\\LASTHOPE\\BACK.ARC;1");
	this->arc_peko_ptr[0] = Archive_Find(this->arc_peko, "peko.tim");

	IO_Data arc_back = IO_Read("\\LASTHOPE\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Mem_Free(arc_back);

	Animatable_Init(&this->peko0_animatable, peko_anim);
	Animatable_SetAnim(&this->peko0_animatable, 0);
	this->peko_frame = this->peko_tex_id = 0xFF; //Force art load
	
	return (StageBack*)this;
}
