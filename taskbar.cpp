#include "taskbar.h"
//#include <wx/icon.h>

//void SetTaskBarIcon(MyTaskBarIcon* taskBarIcon)
//{
//    wxIcon icon;
//    icon.LoadFile("resources/icon.png", wxBITMAP_TYPE_PNG);
//    taskBarIcon->SetIcon(icon, "wxTranslate");
//}

wxBEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
EVT_MENU(ID_MENU_RESTORE, MyTaskBarIcon::OnMenuRestore)
EVT_MENU(ID_MENU_EXIT, MyTaskBarIcon::OnMenuExit)
EVT_TASKBAR_LEFT_DCLICK(MyTaskBarIcon::OnLeftButtonDClick)
wxEND_EVENT_TABLE()
