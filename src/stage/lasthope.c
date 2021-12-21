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

//Week 4 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //bg
	Gfx_Tex tex_back1; //bg
	
} Back_lasthope;

void Back_lasthope_DrawFG(StageBack *back)
{
	Back_lasthope *this = (Back_lasthope*)back;
	
	fixed_t fx, fy;
	
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
	IO_Data arc_back = IO_Read("\\LASTHOPE\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
