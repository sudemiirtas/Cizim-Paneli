#pragma once

#include "Resource.h"  // Kaynakları içe aktar
#include <string>  // C++ dizesi kullanmak için <string> başlık dosyasını içe aktar
#include "DrawObj.h"  // Özel çizim nesnelerini içe aktar

// Paint_Programing kullanarak kullanılan işlevleri içe aktar

// Uygulama sınıfının kayıt işlemi
ATOM MyRegisterClass(HINSTANCE hInstance);

// Uygulama örneğini başlat
BOOL InitInstance(HINSTANCE, int);

// Ana pencere için pencere işleme işlevi
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Çocuk pencere için pencere işleme işlevi
LRESULT CALLBACK ChildWndProc(HWND, UINT, WPARAM, LPARAM);

// Belirtilen koordinatlarda otomatik kaydırma
void AutoScroll(HWND, int, int, int, int, RECT);

// Yerel koordinatları almak için kullanılan işlev
RECT GetLocalCoordinates(HWND hWnd);

// Nesneleri temizlemek için kullanılan işlev
void CleanObjects(HWND);

// Başlığı ayarlamak için kullanılan işlev
void SetTitle(std::string, HWND);

// Araç seçim durumunu değiştirmek için kullanılan işlev
void ChangeToolsSelectionState(int, HMENU);

// Renk seçim durumunu değiştirmek için kullanılan işlev
void ChangeColorsSelectionState(int, HMENU);

// Arkaplan rengi seçim durumunu değiştirmek için kullanılan işlev
void ChangeBGSelectionState(int, HMENU);

// Çizgi kalınlığı seçim durumunu değiştirmek için kullanılan işlev
void ChangeLineSelectionState(int, HMENU);

// Nesneyi otomatik kaydırma işlemi için kullanılan işlev
void AutoScrollObject(HWND, const DrawObj*, int, int, RECT);

// Nesneyi yeniden boyutlandırma işlemi için kullanılan işlev
void AutoScrollObjectResize(HWND, const DrawObj*, int, int, RECT);

// Çizim sırasında nesneyi otomatik kaydırma işlemi için kullanılan işlev
void AutoScrollObjectWhenDrawing(HWND, const DrawObj*, int, int, RECT);
