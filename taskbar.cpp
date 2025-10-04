#include "taskbar.h"

wxBEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
EVT_MENU(ID_MENU_RESTORE, MyTaskBarIcon::OnMenuRestore)
EVT_MENU(ID_MENU_EXIT, MyTaskBarIcon::OnMenuExit)
wxEND_EVENT_TABLE()
