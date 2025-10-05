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

    // Функция для EnumChildWindows
    struct EnumData {
        HWND result = nullptr;
    } data;

    EnumChildWindows(hwndParent, [](HWND hwnd, LPARAM lParam) -> BOOL {
        EnumData* d = reinterpret_cast<EnumData*>(lParam);

        // Проверяем, есть ли выделенный текст
        LRESULT sel = SendMessage(hwnd, EM_GETSEL, 0, 0);
        DWORD start = LOWORD(sel);
        DWORD end = HIWORD(sel);

        if (end > start) {
            d->result = hwnd;
            return FALSE; // нашли — прекращаем перечисление
        }

        // Продолжаем искать
        return TRUE;
        }, reinterpret_cast<LPARAM>(&data));

    return data.result;
}


// Рекурсивный поиск дочернего окна с выделенным текстом
std::wstring FindSelectedTextRecursive(HWND parent) {
    std::wstring selectedText;
    HWND child = GetWindow(parent, GW_CHILD);

    while (child != nullptr) {
        selectedText = GetSelectedTextFromWindow(child);
        if (!selectedText.empty()) {
            return selectedText; // Найдено и возвращаем
        }

        // Рекурсивный вызов для дочерних окон
        selectedText = FindSelectedTextRecursive(child);
        if (!selectedText.empty()) {
            return selectedText; // Найдено и возвращаем
        }

        child = GetWindow(child, GW_HWNDNEXT);
    }
    return L""; // Ничего не найдено
}

// Основная логика извлечения текста из конкретного HWND
std::wstring GetSelectedTextFromWindow(HWND hWnd) {
    if (!hWnd || !IsWindowVisible(hWnd)) {
        return L"";
    }

    // Проверяем наличие выделения через EM_GETSEL
    LRESULT sel = SendMessage(hWnd, EM_GETSEL, 0, 0);
    DWORD start = LOWORD(sel);
    DWORD end = HIWORD(sel);

    // Если нет выделения (start == end), возвращаем пустую строку
    if (start >= end) {
        return L"";
    }

    // Пробуем получить текст из окна.
    // Это будет работать для EDIT и RICHEDIT контролов,
    // и даже для других, если они правильно обрабатывают EM_GETSEL и WM_GETTEXT.
    int len = (int)SendMessageW(hWnd, WM_GETTEXTLENGTH, 0, 0);

    if (len > 0) {
        std::wstring buf(len + 1, L'\0');
        // Получаем весь текст. Система выполнит маршаллинг буфера.
        int charsCopied = (int)SendMessageW(hWnd, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)&buf[0]);

        if (charsCopied > 0) {
            // Убеждаемся, что границы в пределах текста
            end = min(charsCopied, end);
                // Извлекаем подстроку
            if (start < end && end <= buf.size()) {
                std::wstring selected = buf.substr(start, end - start);
                return selected;
            }
        }
    }

    //if (hWnd) {

    //    // 1. Узнаем длину текста (WM_GETTEXTLENGTH)
    //    // SendMessage для кросс-процессной передачи текста
    //    int len = (int)SendMessageW(hWnd, WM_GETTEXTLENGTH, 0, 0);

    //    // 2. Выделяем буфер
    //    std::wstring buf(len + 1, L'\0');

    //    // 3. Получаем текст (WM_GETTEXT). Система выполнит маршаллинг буфера.
    //    SendMessageW(hWnd, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)&buf[0]);

    //    // Получаем границы выделения.
    //    // Для EM_GETSEL, начало (start) в LOWORD, конец (end) в HIWORD LRESULT.
    //    LRESULT sel = SendMessage(hWnd, EM_GETSEL, 0, 0);

    //    // Извлекаем start и end из LRESULT
    //    DWORD start = LOWORD(sel);
    //    DWORD end = HIWORD(sel);

    //    // Проверяем корректность и извлекаем подстроку
    //    if (start < end && end <= buf.size()) {
    //        return buf.substr(start, end - start);
    //    }
    //}
    return L"";
}

// Главная функция для получения выделенного текста
std::wstring GetSelectedText() {
    DWORD fgThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
    DWORD myThread = GetCurrentThreadId();

    // 1. Присоединяем поток
    if (!AttachThreadInput(myThread, fgThread, TRUE)) {
        return L""; // Не удалось присоединиться
    }

    HWND fgWindow = GetForegroundWindow();
    HWND focused = GetFocus();
    std::wstring selectedText;

    // 2. Сначала проверяем окно, которое имеет фокус ввода
    if (focused != nullptr) {
        selectedText = GetSelectedTextFromWindow(focused);
        if (!selectedText.empty()) {
            AttachThreadInput(myThread, fgThread, FALSE);
            return selectedText;
        }
    }

    // 3. Если в окне с фокусом нет выделенного текста,
    // ищем выделение рекурсивно, начиная с главного окна.
    // (Поиск начинается с дочерних окон активного окна, включая фокус.)
    selectedText = FindSelectedTextRecursive(focused);

    // 4. Отсоединяем поток
    AttachThreadInput(myThread, fgThread, FALSE);

    return selectedText;
}