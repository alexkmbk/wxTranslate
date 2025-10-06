#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/artprov.h>

enum {
    ID_MENU_RESTORE = wxID_HIGHEST + 2,
    ID_MENU_EXIT
};

class MyTaskBarIcon : public wxTaskBarIcon
{
public:
    MyTaskBarIcon(wxFrame* frame) : m_frame(frame) {}

    wxMenu* CreatePopupMenu() override {
        wxMenu* menu = new wxMenu;
        menu->Append(ID_MENU_RESTORE, "Показать окно");
        menu->AppendSeparator();
        menu->Append(ID_MENU_EXIT, "Выход");
        return menu;
    }

    void OnMenuRestore(wxCommandEvent&) {
        if (m_frame) {
            m_frame->Show();
            m_frame->Raise();
        }
    }

    void OnMenuExit(wxCommandEvent&) {
        if (m_frame) {
            m_frame->Destroy();
        }
    }

    void OnLeftButtonDClick(wxTaskBarIconEvent& event) {
        if (m_frame) {
            m_frame->Show();
            m_frame->Raise();
        }
    }
private:
    wxFrame* m_frame;
    wxDECLARE_EVENT_TABLE();
};

