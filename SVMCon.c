#include "SVMTypes.h"
#include "SVMCon.h"

#ifdef _WIN32
#include <windows.h>

HANDLE conIn;
HANDLE conOut;

CHAR_INFO buffer[80*25];

static void SVMUpdateConsole()
{
  COORD sz = { 80, 25 };
  COORD coord = { 0, 0 };
  SMALL_RECT region = { 0, 0, 79, 24 };
  WriteConsoleOutput(conOut, &buffer[0], sz, coord, &region);
}

SVM_WORD SVMSetSymbol(SVM_WORD idx, SVM_WORD sym)
{
  ((SVM_WORD*)&buffer[0])[idx] = sym;
  SVMUpdateConsole();
  return 0;
}

SVM_WORD SVMGetSymbol(SVM_WORD idx)
{
  return ((SVM_WORD*)&buffer[0])[idx];
}

SVM_WORD SVMSetCursorPos(SVM_WORD pos)
{
  if(LOWORD(pos) < 80 && HIWORD(pos) < 25)
    return SetConsoleCursorPosition(conOut, *(COORD*)&pos);
  else
    return 0;
}

SVM_WORD SVMGetCursorPos()
{
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(conOut, &info);
  return *(SVM_WORD*)&info.dwCursorPosition;
}

SVM_WORD SVMSetCursorVisibility(SVM_WORD visible)
{
  CONSOLE_CURSOR_INFO info;
  GetConsoleCursorInfo(conOut, &info);
  info.bVisible = visible;
  return SetConsoleCursorInfo(conOut, &info);
}

SVM_WORD SVMGetCursorVisibility()
{
  CONSOLE_CURSOR_INFO info;
  GetConsoleCursorInfo(conOut, &info);
  return info.bVisible;
}

SVM_WORD SVMInitConsole()
{
  SMALL_RECT r;
  COORD bs;
  DWORD n;
  AllocConsole();
  conIn = CreateFile(
    L"CONIN$",
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    0,
    NULL);
  if(INVALID_HANDLE_VALUE == conIn)
    return 0;
  conOut = CreateFile(
    L"CONOUT$",
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    0,
    NULL);
  if(INVALID_HANDLE_VALUE == conOut)
    return 0;
  bs.X = 80;
  bs.Y = 25;  
  r.Left = 0;
  r.Top = 0;
  r.Right = 79;
  r.Bottom = 24;
  SetConsoleWindowInfo(conOut, TRUE, &r);
  if(!SetConsoleScreenBufferSize(conOut, bs))
    return 0;
  if(!SetConsoleWindowInfo(conOut, TRUE, &r))
    return 0;
  bs.X = 0;
  bs.Y = 0;
  if(!FillConsoleOutputCharacter(conOut, ' ', 80*25, bs, &n))
    return 0;
  if(!FillConsoleOutputAttribute(conOut, 0x0F, 80*25, bs, &n))
    return 0;
  memset(buffer, 0, sizeof(buffer));
  SetConsoleOutputCP(1252);
  return 1;
}

SVM_WORD SVMCloseConsole()
{
  return 1;
}

SVM_INT SVMPeekConsoleInput()
{
  INPUT_RECORD ir;
  DWORD n = 0;
  do
  {
    PeekConsoleInput(conIn, &ir, 1, &n);
    if(n > 0)
    {
      ReadConsoleInput(conIn, &ir, 1, &n);
      if(KEY_EVENT == ir.EventType && ir.Event.KeyEvent.bKeyDown)
        return ir.Event.KeyEvent.wVirtualKeyCode;
    }
  }while(n > 0);
  return -1;
}
#else
#error Please implement SVM console for your platform
#endif // _WIN32