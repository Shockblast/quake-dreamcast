/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// in_null.c -- for systems without a mouse

#include "quakedef.h"

#include <dc/maple.h>
#include <dc/maple/keyboard.h>
#include <dc/maple/controller.h>
#include <dc/maple/mouse.h>

cvar_t	m_filter = {"m_filter","0"};

cvar_t	in_joystick = {"joystick","1", true};
cvar_t	joy_name = {"joyname", "joystick"};
cvar_t	joy_advanced = {"joyadvanced", "0"};
cvar_t	joy_advaxisx = {"joyadvaxisx", "0"};
cvar_t	joy_advaxisy = {"joyadvaxisy", "0"};
cvar_t	joy_advaxisz = {"joyadvaxisz", "0"};
cvar_t	joy_advaxisr = {"joyadvaxisr", "0"};
cvar_t	joy_advaxisu = {"joyadvaxisu", "0"};
cvar_t	joy_advaxisv = {"joyadvaxisv", "0"};
cvar_t	joy_forwardthreshold = {"joyforwardthreshold", "0.15"};
cvar_t	joy_sidethreshold = {"joysidethreshold", "0.15"};
cvar_t	joy_pitchthreshold = {"joypitchthreshold", "0.15"};
cvar_t	joy_yawthreshold = {"joyyawthreshold", "0.15"};
cvar_t	joy_forwardsensitivity = {"joyforwardsensitivity", "-1.0"};
cvar_t	joy_sidesensitivity = {"joysidesensitivity", "-1.0"};
cvar_t	joy_pitchsensitivity = {"joypitchsensitivity", "1.0"};
cvar_t	joy_yawsensitivity = {"joyyawsensitivity", "-1.0"};

void IN_Init (void)
{
	// mouse variables
	Cvar_RegisterVariable (&m_filter);

	// joystick variables
	Cvar_RegisterVariable (&in_joystick);
	Cvar_RegisterVariable (&joy_name);
	Cvar_RegisterVariable (&joy_advanced);
	Cvar_RegisterVariable (&joy_advaxisx);
	Cvar_RegisterVariable (&joy_advaxisy);
	Cvar_RegisterVariable (&joy_advaxisz);
	Cvar_RegisterVariable (&joy_advaxisr);
	Cvar_RegisterVariable (&joy_advaxisu);
	Cvar_RegisterVariable (&joy_advaxisv);
	Cvar_RegisterVariable (&joy_forwardthreshold);
	Cvar_RegisterVariable (&joy_sidethreshold);
	Cvar_RegisterVariable (&joy_pitchthreshold);
	Cvar_RegisterVariable (&joy_yawthreshold);
	Cvar_RegisterVariable (&joy_forwardsensitivity);
	Cvar_RegisterVariable (&joy_sidesensitivity);
	Cvar_RegisterVariable (&joy_pitchsensitivity);
	Cvar_RegisterVariable (&joy_yawsensitivity);
}

void IN_Shutdown (void)
{
}

static void convert_event(int buttons,int prev_buttons,const unsigned char *cvtbl,int n)
{
	int changed = buttons^prev_buttons;
	int i;

	if (!changed) return;

	for(i=0;i<n;i++) {
		if (cvtbl[i] && changed&(1<<i)) {
			Key_Event(cvtbl[i],(buttons>>i)&1);
		}
	}
}

static void IN_Joystick(void)
{
	const static unsigned	char	cvtbl[] = {
#if 1
	/* same code of keyboard */
	0, /* C */
	K_SPACE, /* B */
	K_ENTER, /* A */
	K_ESCAPE, /* START */
	K_UPARROW, /* UP */
	K_DOWNARROW, /* DOWN */
	K_LEFTARROW, /* LEFT */
	K_RIGHTARROW, /* RIGHT */
	0, /* Z */
	'y', /* Y */
	K_JOY3, /* X */
	0, /* D */
	0, /* UP2 */
	0, /* DOWN2 */
	0, /* LEFT2 */
	0, /* RIGHT2 */
	K_ALT,/* ltrig */
	K_CTRL,/* rtrig */
#else
	0, /* C */
	K_JOY2, /* B */
	K_JOY1, /* A */
	0, /* START */
	K_AUX29, /* UP */
	K_AUX31, /* DOWN */
	K_AUX32, /* LEFT */
	K_AUX30, /* RIGHT */
	0, /* Z */
	K_JOY4, /* Y */
	K_JOY3, /* X */
	0, /* D */
	0, /* UP2 */
	0, /* DOWN2 */
	0, /* LEFT2 */
	0, /* RIGHT2 */
	K_JOY5,/* ltrig */
	K_JOY6,/* rtrig */
#endif
	};

	uint8	caddr;
	cont_cond_t	cond;
	static int prev_buttons;
	int buttons;

	if ((caddr=maple_first_controller())==0 || cont_get_cond(caddr, &cond)<0) return;
	
	buttons = cond.buttons^0xffff;
	if (cond.ltrig) buttons|=(1<<16);
	if (cond.rtrig) buttons|=(1<<17);
	convert_event(buttons,prev_buttons,cvtbl,sizeof(cvtbl));
	prev_buttons = buttons;

}

static void IN_Mouse(void)
{
	const static unsigned char cvtbl[] = {
	0,
	K_MOUSE2,	/* rbutton  */
	K_MOUSE1,	/* lbutton */
	0 /* K_MOUSE3 */,	/* wheel press? */
	K_MWHEELUP,	/* wheel up*/
	K_MWHEELDOWN,	/* wheel down */
	};

	uint8	maddr;
	mouse_cond_t	cond;

	static int prev_buttons;
	int buttons;

	if ((maddr=maple_first_mouse())==0 || mouse_get_cond(maddr, &cond)<0) return;

	buttons = cond.buttons^0xff;
	if (cond.dz<0) buttons|=1<<4;
	if (cond.dz>0) buttons|=1<<5;
	convert_event(buttons,prev_buttons,cvtbl,sizeof(cvtbl));
	prev_buttons = buttons;
}


static void analog_move(usercmd_t *cmd,int mx,int my)
{
	float	mouse_x,mouse_y;
	static	float	old_mouse_x, old_mouse_y;

	if (m_filter.value)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}
	old_mouse_x = mx;
	old_mouse_y = my;

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;

	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;
	
	if (in_mlook.state & 1)
		V_StopPitchDrift ();
		
	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}
}

static void IN_MouseMove(usercmd_t *cmd)
{
	uint8	maddr;
	mouse_cond_t	cond;
	int mouse_x,mouse_y;

	if ((maddr=maple_first_mouse())==0 || mouse_get_cond(maddr, &cond)<0) return;

	analog_move(cmd, cond.dx*4,cond.dy*4);
}

#define JOY_ABSOLUTE_AXIS	0x00000000		// control like a joystick
#define JOY_RELATIVE_AXIS	0x00000010		// control like a mouse, spinner, tr
#define	JOY_MAX_AXES	6
enum _ControlList
{
	AxisNada = 0, AxisForward, AxisLook, AxisSide, AxisTurn
};
const static int	dwAxisMap[JOY_MAX_AXES] = {
	AxisNada,	/* rtrig */
	AxisNada,	/* ltrig */
	AxisTurn,	/* joyx */
	AxisForward,	/* joyy */
	AxisNada,	/* joy2x */
	AxisNada,	/* joy2y */
};
const static int	dwControlMap[JOY_MAX_AXES] = {
	JOY_ABSOLUTE_AXIS,
	JOY_ABSOLUTE_AXIS,
	JOY_ABSOLUTE_AXIS,
	JOY_ABSOLUTE_AXIS,
	JOY_ABSOLUTE_AXIS,
	JOY_ABSOLUTE_AXIS,
};

static void IN_JoyMove(usercmd_t *cmd)
{
	uint8	caddr;
	cont_cond_t	cond;
	float	speed, aspeed;
	float	fAxisValue, fTemp;
	int		i;

	if ((caddr=maple_first_controller())==0 || cont_get_cond(caddr, &cond)<0) return;
	
#if 1 /* WIN routine */
	if (in_speed.state & 1)
		speed = cl_movespeedkey.value;
	else
		speed = 1;
	aspeed = speed * host_frametime;

	// loop through the axes
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		// get the floating point zero-centered, potentially-inverted data for the current axis
		fAxisValue = (float) ((&cond.rtrig)[i]-128)/128.0;
		// move centerpoint to zero
		// convert range from -32768..32767 to -1..1 

		switch (dwAxisMap[i])
		{
		case AxisForward:
			if ((joy_advanced.value == 0.0) && (in_mlook.state & 1))
			{
				// user wants forward control to become look control
				if (fabs(fAxisValue) > joy_pitchthreshold.value)
				{		
					// if mouse invert is on, invert the joystick pitch value
					// only absolute control support here (joy_advanced is false)
					if (m_pitch.value < 0.0)
					{
						cl.viewangles[PITCH] -= (fAxisValue * joy_pitchsensitivity.value) * aspeed * cl_pitchspeed.value;
					}
					else
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * aspeed * cl_pitchspeed.value;
					}
					V_StopPitchDrift();
				}
				else
				{
					// no pitch movement
					// disable pitch return-to-center unless requested by user
					// *** this code can be removed when the lookspring bug is fixed
					// *** the bug always has the lookspring feature on
					if(lookspring.value == 0.0)
						V_StopPitchDrift();
				}
			}
			else
			{
				// user wants forward control to be forward control
				if (fabs(fAxisValue) > joy_forwardthreshold.value)
				{
					cmd->forwardmove += (fAxisValue * joy_forwardsensitivity.value) * speed * cl_forwardspeed.value;
				}
			}
			break;

		case AxisSide:
			if (fabs(fAxisValue) > joy_sidethreshold.value)
			{
				cmd->sidemove += (fAxisValue * joy_sidesensitivity.value) * speed * cl_sidespeed.value;
			}
			break;

		case AxisTurn:
			if ((in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1)))
			{
				// user wants turn control to become side control
				if (fabs(fAxisValue) > joy_sidethreshold.value)
				{
					cmd->sidemove -= (fAxisValue * joy_sidesensitivity.value) * speed * cl_sidespeed.value;
				}
			}
			else
			{
				// user wants turn control to be turn control
				if (fabs(fAxisValue) > joy_yawthreshold.value)
				{
					if(dwControlMap[i] == JOY_ABSOLUTE_AXIS)
					{
						cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity.value) * aspeed * cl_yawspeed.value;
					}
					else
					{
						cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity.value) * speed * 180.0;
					}

				}
			}
			break;

		case AxisLook:
			if (in_mlook.state & 1)
			{
				if (fabs(fAxisValue) > joy_pitchthreshold.value)
				{
					// pitch movement detected and pitch movement desired by user
					if(dwControlMap[i] == JOY_ABSOLUTE_AXIS)
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * aspeed * cl_pitchspeed.value;
					}
					else
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * speed * 180.0;
					}
					V_StopPitchDrift();
				}
				else
				{
					// no pitch movement
					// disable pitch return-to-center unless requested by user
					// *** this code can be removed when the lookspring bug is fixed
					// *** the bug always has the lookspring feature on
					if(lookspring.value == 0.0)
						V_StopPitchDrift();
				}
			}
			break;

		default:
			break;
		}
	}

	// bounds check pitch
	if (cl.viewangles[PITCH] > 80.0)
		cl.viewangles[PITCH] = 80.0;
	if (cl.viewangles[PITCH] < -70.0)
		cl.viewangles[PITCH] = -70.0;
#else /* DOS routine */
	if (in_speed.state & 1)
		speed = cl_movespeedkey.value;
	else
		speed = 1;
	aspeed = speed*host_frametime;

	if (in_strafe.state & 1)
	{
		if (joystickx < joyxl)
			cmd->sidemove -= speed*cl_sidespeed.value; 
		else if (joystickx > joyxh) 
			cmd->sidemove += speed*cl_sidespeed.value; 
	}
	else
	{
		if (joystickx < joyxl)
			cl.viewangles[YAW] += aspeed*cl_yawspeed.value;
		else if (joystickx > joyxh) 
			cl.viewangles[YAW] -= aspeed*cl_yawspeed.value;
		cl.viewangles[YAW] = anglemod(cl.viewangles[YAW]);
	}

	if (in_mlook.state & 1)
	{
		if (m_pitch.value < 0)
			speed *= -1;
		
		if (joysticky < joyyl) 
			cl.viewangles[PITCH] += aspeed*cl_pitchspeed.value;
		else if (joysticky > joyyh) 
			cl.viewangles[PITCH] -= aspeed*cl_pitchspeed.value;
	}
	else
	{
		if (joysticky < joyyl) 
			cmd->forwardmove += speed*cl_forwardspeed.value; 
		else if (joysticky > joyyh) 
			cmd->forwardmove -= speed*cl_backspeed.value;  
	}
#endif
}


void IN_Commands (void)
{
	IN_Joystick();
	IN_Mouse();
}

void IN_Move (usercmd_t *cmd)
{
	IN_JoyMove(cmd);
	IN_MouseMove(cmd);
}

const static unsigned char q_key[]= {
	/*0*/	0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
		'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
		'u', 'v', 'w', 'x', 'y', 'z',
	/*1e*/	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	/*28*/	K_ENTER, K_ESCAPE, K_BACKSPACE, K_TAB, K_SPACE, '-', '=', '[', ']', '\\', 0, ';', '\'',
	/*35*/	'`', ',', '.', '/', 0, K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12,
	/*46*/	0, 0, K_PAUSE, K_INS, K_HOME, K_PGUP, K_DEL, K_END, K_PGDN, K_RIGHTARROW, K_LEFTARROW, K_DOWNARROW, K_UPARROW,
	/*53*/	0, '/', '*', '-', '+', 13, '1', '2', '3', '4', '5', '6',
	/*5f*/	'7', '8', '9', '0', '.', 0
};

const static unsigned char q_shift[] = {
	K_CTRL,K_SHIFT,K_ALT
};

void Sys_SendKeyEvents (void)
{
	static kbd_state_t	old_state;
	static uint8 old_addr;

	kbd_state_t *state;
	uint8	addr;
	int	port,unit;

	int shiftkeys;

	int i;

	addr = maple_first_kb();
	if (addr!=old_addr) {
		old_addr = addr;
		memset(&old_state,0,sizeof(old_state));
	}

	if (addr==0) return;

	maple_raddr(addr,&port,&unit);

	state = kbd_get_state(port,unit);

	shiftkeys = state->shift_keys ^ old_state.shift_keys;
	for(i=0;i<3;i++) {
		if ((shiftkeys>>i)&1) {
			Key_Event(q_shift[i],(state->shift_keys>>i)&1);
		}
	}

	for(i=0;i<sizeof(q_key);i++) {
		if (state->matrix[i]!=old_state.matrix[i]) {
			int key = q_key[i];
			if (key) Key_Event(key,state->matrix[i]);
		}
	}

	old_state = *state;
}

