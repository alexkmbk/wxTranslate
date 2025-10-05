#include <windows.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <cwchar>

#include "selected_text.h"

std::wstring GetSelectedTextFromWindow(HWND hWnd);

HWND GetFocusedOrSelectedChild(HWND hwndParent)
{
    HWND found = nullptr;

    // ������� ��� EnumChildWindows
    struct EnumData {
        HWND result = nullptr;
    } data;

    EnumChildWindows(hwndParent, [](HWND hwnd, LPARAM lParam) -> BOOL {
        EnumData* d = reinterpret_cast<EnumData*>(lParam);

        // ���������, ���� �� ���������� �����
        LRESULT sel = SendMessage(hwnd, EM_GETSEL, 0, 0);
        DWORD start = LOWORD(sel);
        DWORD end = HIWORD(sel);

        if (end > start) {
            d->result = hwnd;
            return FALSE; // ����� � ���������� ������������
        }

        // ���������� ������
        return TRUE;
        }, reinterpret_cast<LPARAM>(&data));

    return data.result;
}


// ����������� ����� ��������� ���� � ���������� �������
std::wstring FindSelectedTextRecursive(HWND parent) {
    std::wstring selectedText;
    HWND child = GetWindow(parent, GW_CHILD);

    while (child != nullptr) {
        selectedText = GetSelectedTextFromWindow(child);
        if (!selectedText.empty()) {
            return selectedText; // ������� � ����������
        }

        // ����������� ����� ��� �������� ����
        selectedText = FindSelectedTextRecursive(child);
        if (!selectedText.empty()) {
            return selectedText; // ������� � ����������
        }

        child = GetWindow(child, GW_HWNDNEXT);
    }
    return L""; // ������ �� �������
}

// �������� ������ ���������� ������ �� ����������� HWND
std::wstring GetSelectedTextFromWindow(HWND hWnd) {
    if (!hWnd || !IsWindowVisible(hWnd)) {
        return L"";
    }

    // ��������� ������� ��������� ����� EM_GETSEL
    LRESULT sel = SendMessage(hWnd, EM_GETSEL, 0, 0);
    DWORD start = LOWORD(sel);
    DWORD end = HIWORD(sel);

    // ���� ��� ��������� (start == end), ���������� ������ ������
    if (start >= end) {
        return L"";
    }

    // ������� �������� ����� �� ����.
    // ��� ����� �������� ��� EDIT � RICHEDIT ���������,
    // � ���� ��� ������, ���� ��� ��������� ������������ EM_GETSEL � WM_GETTEXT.
    int len = (int)SendMessageW(hWnd, WM_GETTEXTLENGTH, 0, 0);

    if (len > 0) {
        std::wstring buf(len + 1, L'\0');
        // �������� ���� �����. ������� �������� ���������� ������.
        int charsCopied = (int)SendMessageW(hWnd, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)&buf[0]);

        if (charsCopied > 0) {
            // ����������, ��� ������� � �������� ������
            end = min(charsCopied, end);
                // ��������� ���������
            if (start < end && end <= buf.size()) {
                std::wstring selected = buf.substr(start, end - start);
                return selected;
            }
        }
    }

    //if (hWnd) {

    //    // 1. ������ ����� ������ (WM_GETTEXTLENGTH)
    //    // SendMessage ��� �����-���������� �������� ������
    //    int len = (int)SendMessageW(hWnd, WM_GETTEXTLENGTH, 0, 0);

    //    // 2. �������� �����
    //    std::wstring buf(len + 1, L'\0');

    //    // 3. �������� ����� (WM_GETTEXT). ������� �������� ���������� ������.
    //    SendMessageW(hWnd, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)&buf[0]);

    //    // �������� ������� ���������.
    //    // ��� EM_GETSEL, ������ (start) � LOWORD, ����� (end) � HIWORD LRESULT.
    //    LRESULT sel = SendMessage(hWnd, EM_GETSEL, 0, 0);

    //    // ��������� start � end �� LRESULT
    //    DWORD start = LOWORD(sel);
    //    DWORD end = HIWORD(sel);

    //    // ��������� ������������ � ��������� ���������
    //    if (start < end && end <= buf.size()) {
    //        return buf.substr(start, end - start);
    //    }
    //}
    return L"";
}

// ������� ������� ��� ��������� ����������� ������
std::wstring GetSelectedText() {
    DWORD fgThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
    DWORD myThread = GetCurrentThreadId();

    // 1. ������������ �����
    if (!AttachThreadInput(myThread, fgThread, TRUE)) {
        return L""; // �� ������� ��������������
    }

    HWND fgWindow = GetForegroundWindow();
    HWND focused = GetFocus();
    std::wstring selectedText;

    // 2. ������� ��������� ����, ������� ����� ����� �����
    if (focused != nullptr) {
        selectedText = GetSelectedTextFromWindow(focused);
        if (!selectedText.empty()) {
            AttachThreadInput(myThread, fgThread, FALSE);
            return selectedText;
        }
    }

    // 3. ���� � ���� � ������� ��� ����������� ������,
    // ���� ��������� ����������, ������� � �������� ����.
    // (����� ���������� � �������� ���� ��������� ����, ������� �����.)
    selectedText = FindSelectedTextRecursive(focused);

    // 4. ����������� �����
    AttachThreadInput(myThread, fgThread, FALSE);

    return selectedText;
}