#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

void InstallKeyboard(void);
bool GetKey(int ScanCode);
void ChangeKeyState(int ScanCode);
void ShutDownKeyboard(void);
#endif
