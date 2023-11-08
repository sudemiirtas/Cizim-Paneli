#pragma once

#include "stdafx.h"  // Önişlemci komutu, Visual Studio projelerinde yaygın olarak kullanılır.

#include <unordered_map>  // std::unordered_map kullanmak için gerekli başlık dosyası.
#include <functional>     // std::function kullanmak için gerekli başlık dosyası.

struct Parameter
{
    HWND hWnd_;       // Pencere tutucu (handle) nesnesi
    WPARAM wParam_;   // Durum parametresi
    LPARAM lParam_;   // Ek bilgi parametresi
    UINT message_;    // Olay kodu

    Parameter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        : hWnd_(hWnd)
        , message_(message)
        , wParam_(wParam)
        , lParam_(lParam)
    {}
};

using Event = std::function<LRESULT(Parameter& param)>;  // Event türü, bir işlevi temsil eder.

class Listener
{
private:
    std::unordered_map<UINT, Event> evnetHandlerMap_;  // Olay kodları ve ilgili işleyicileri saklayan bir map.
    Event defaultEventHandler_;  // Varsayılan olay işleyici.

public:
    HINSTANCE hInst;  // HINSTANCE türünden bir değişken.

    // Windows işleme mesajlarını dinlemek için kullanılacak olan Listener sınıfı için bir tek örnek döndürme işlevi.
    static Listener& WinProcMsgListener();

    // Varsayılan olay işleyiciyi ayarlamak için kullanılır.
    inline void AddDefaultEvent(Event e) { defaultEventHandler_ = e; }

    // Belirli bir olay kodu için bir olay işleyici eklemek için kullanılır.
    inline void AddEvent(UINT msg, Event e) { evnetHandlerMap_[msg] = e; }

    // Belirli bir olay kodunu tetiklemek için kullanılır ve ilgili işleyiciyi çağırır.
    LRESULT Trig(UINT msg, Parameter& param);
};
