#include "device.h"
#include "common.h"


static char key_state[128],key_pressed[128];
void InstallKeyboard(void)
{
	int i;
	for(i = 0;i < 128;i ++)
	{
		key_state[i] = 0;
		key_pressed[i] = 0;
	}
}

void ChangeKeyState(int ScanCode)
{
	if(ScanCode & 0x80)
	{
		ScanCode &= 0x7f; 
		key_state[ScanCode]=0; 
	}
	
	else 
	{ 
		key_state[ScanCode]=1; 
		key_pressed[ScanCode]=1; 
	}
	
}

bool GetKey(int ScanCode) 
{ 
	bool res; 
	res = key_state[ScanCode] | key_pressed[ScanCode]; 
	key_pressed[ScanCode]=0; 
	return res; 
} 

void ShutDownKeyboard(void) 
{

}
