#include "taskbar.h"

wxBEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
EVT_MENU(ID_MENU_RESTORE, MyTaskBarIcon::OnMenuRestore)
EVT_MENU(ID_MENU_EXIT, MyTaskBarIcon::OnMenuExit)
EVT_TASKBAR_LEFT_DCLICK(MyTaskBarIcon::OnLeftButtonDClick)
wxEND_EVENT_TABLE()
