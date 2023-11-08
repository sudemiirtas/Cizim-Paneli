#pragma once
#include "DrawObj.h"
#include "mylog.h"
#define MAX_LOADSTRING 100

// Globals sınıfıyla ilgili verileri içeren yorumlar
class Globals
{
public:
    HINSTANCE hInst;                                // Uygulama örneği
    WCHAR szTitle[MAX_LOADSTRING];                  // Başlık çubuğu metni
    WCHAR szWindowClass[MAX_LOADSTRING];            // Ana pencere sınıfı adı
    WCHAR szChildClass[MAX_LOADSTRING];             // Çocuk pencere sınıfı adı
    HWND hWndFather, myChildWindow;
    HWND myButton[5];
    int currentDrawMode;     // Çizim modu (0=line, 1=rect, 2=circle, 3=text, 4=select - Araç kutusu)
    int modifyState;  // Dosya değiştirme durumu (0=new file, 1=modified but not saved, 2=saved or opened)
    string fileName;  // Pencere başlığında görüntülenecek dosya adı
    DrawObj* pastebinObjectPtr;  // Kopyalama veya Kesme sonrası yapıştırma nesnesi
    DrawObj* selectedObjectPtr;  // Seçili şekil
    DrawObj* preSelectObject;    // Seçili şekli işleme alır
    wstring lastFilePath;  // Son açılan dosyanın yol bilgisi

    int mouseX, mouseY;
    TextObj newText;  // Yalnızca newText şu anda global olmalı
    bool mouseHasDown;
    bool hasSelected;

    RECT rect;
    bool autoScrolling;
    TCHAR szBuffer[50];
    int cxChar, cyChar;
    list<DrawObj*> drawObjList;

    mylog mlog;

    //----------------------------------------------------------------------------------------------
    bool debugMode; // Hata ayıklama modunda programı çalıştır
    //----------------------------------------------------------------------------------------------

    Globals();
    static Globals& var();
};
