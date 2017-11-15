#include "III.h"

CPad CPad::Pads[2];

ushort CPad::oldKeystates[KEY_NUMKEYS];
ushort CPad::newKeystates[KEY_NUMKEYS];
ushort CPad::tempKeystates[KEY_NUMKEYS];

void
CPad::UpdatePads(void)
{
	GetPad(0)->Update(0);
	GetPad(1)->Update(1);

	// keyboard
	memcpy(CPad::oldKeystates, CPad::newKeystates, sizeof(CPad::oldKeystates));
	memcpy(CPad::newKeystates, CPad::tempKeystates, sizeof(CPad::newKeystates));
}

void
CPad::Update(short n)
{
	plCapturePad(n);
	CPad::GetPad(n)->OldState = CPad::GetPad(n)->NewState;
	plUpdatePad(&CPad::GetPad(n)->NewState);
}

bool
CPad::IsKeyJustDown(int key)
{
	return oldKeystates[key] == 0 && newKeystates[key] == 1;
}

bool
CPad::IsKeyDown(int key)
{
	return newKeystates[key] == 1;
}


void
CControllerState::Clear(void)
{
	leftX = 0;
	leftY = 0;
	rightX = 0;
	rightY = 0;
	l1 = 0;
	l2 = 0;
	r1 = 0;
	r2 = 0;
	up = 0;
	down = 0;
	left = 0;
	right = 0;
	start = 0;
	select = 0;
	square = 0;
	triangle = 0;
	cross = 0;
	circle = 0;
	leftshock = 0;
	rightshock = 0;
}
