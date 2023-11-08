#include "stdafx.h"
#include "Listener.h"

// Belirli bir mesajı ilgili dinleyici işlevine eşleştirmek için kullanılan fonksiyon.
LRESULT Listener::Trig(UINT msg, Parameter& param)
{
    if (evnetHandlerMap_[msg] != nullptr)
        return evnetHandlerMap_[msg](param);
    return DefWindowProc(param.hWnd_, msg, param.wParam_, param.lParam_);
}

// Listener sınıfının tek bir örneğini oluşturup döndüren ve bu nesnenin üzerinden olayları dinlemeye izin veren işlev.
Listener& Listener::WinProcMsgListener() {
    static Listener l;
    return l;
}
