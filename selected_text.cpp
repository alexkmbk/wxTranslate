#include <windows.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

// ��������� ������� ���������� ������ ������
HANDLE CopyClipboardData(UINT format)
{
    HANDLE hData = GetClipboardData(format);
    if (!hData) return nullptr;

    SIZE_T size = GlobalSize(hData);
    if (size == 0) return nullptr;

    HGLOBAL hCopy = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hCopy) return nullptr;

    void* pSrc = GlobalLock(hData);
    void* pDst = GlobalLock(hCopy);
    if (pSrc && pDst)
        memcpy(pDst, pSrc, size);
    GlobalUnlock(hData);
    GlobalUnlock(hCopy);

    return hCopy;
}

// �������� �������: �������� ���������� ����� �� ��������� ����
std::wstring GetSelectedText()
{
    std::wstring result;

    // ��������� ������� ���������� ������ ������
    if (!OpenClipboard(nullptr))
        return L"";
    HANDLE oldTextData = CopyClipboardData(CF_UNICODETEXT);
    EmptyClipboard();
    CloseClipboard();

    // ����, ���� ������������ �������� ��� ������������
    while (GetAsyncKeyState(VK_LWIN) & 0x8000 ||
        GetAsyncKeyState(VK_RWIN) & 0x8000 ||
        GetAsyncKeyState(VK_SHIFT) & 0x8000 ||
        GetAsyncKeyState(VK_MENU) & 0x8000 ||
        GetAsyncKeyState(VK_CONTROL) & 0x8000)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // ��������� Ctrl+C
    INPUT inputs[4] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'C';

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'C';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));

    // ���� ���������� ������ ������ (�� 1 �������)
    std::wstring newText;
    auto start = std::chrono::steady_clock::now();

    for (;;)
    {
        if (OpenClipboard(nullptr))
        {
            HANDLE hData = GetClipboardData(CF_UNICODETEXT);
            if (hData)
            {
                LPCWSTR pText = static_cast<LPCWSTR>(GlobalLock(hData));
                if (pText)
                {
                    newText = pText;
                    GlobalUnlock(hData);
                }
            }
            CloseClipboard();
        }

        if (!newText.empty())
            break;

        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1))
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // ��������������� ������ ����� ������
    if (OpenClipboard(nullptr))
    {
        EmptyClipboard();
        if (oldTextData)
            SetClipboardData(CF_UNICODETEXT, oldTextData);
        CloseClipboard();
    }

    result = newText;
    return result;
}


