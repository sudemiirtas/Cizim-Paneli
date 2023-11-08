// Paint_Programing.cpp : Uygulamanın giriş noktasını tanımlar.
//

#include "stdafx.h"
#include "Paint_Programing.h"
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "Windowsx.h"
#include "Windows.h"
#include "DrawObj.h"
#include "Listener.h"
#include "WM_Command.h"
#include "Globals.h"

using namespace std;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Global Veriler
	Globals::var().hInst = hInstance;
	Listener::WinProcMsgListener().hInst = hInstance;

	// Icon program
	LoadStringW(hInstance, IDS_APP_TITLE, Globals::var().szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_PAINT_PROGRAMING, Globals::var().szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Her Bir Mesaj Türünü İlgili Fonksiyonlarla Eşleştirme
	Listener::WinProcMsgListener().AddEvent(WM_COMMAND, WM_CommandEvent);
	Listener::WinProcMsgListener().AddEvent(WM_CREATE, WM_CreateEvent);
	Listener::WinProcMsgListener().AddEvent(WM_MOUSEMOVE, WM_MouseMoveEvent);
	Listener::WinProcMsgListener().AddEvent(WM_LBUTTONDOWN, WM_LButtonDownEvent);
	Listener::WinProcMsgListener().AddEvent(WM_LBUTTONUP, WM_LButtonUpEvent);
	Listener::WinProcMsgListener().AddEvent(WM_KEYDOWN, WM_KeyDownEvent);
	Listener::WinProcMsgListener().AddEvent(WM_PAINT, WM_PaintEvent);
	Listener::WinProcMsgListener().AddEvent(WM_SIZE, WM_SizeEvent);
	Listener::WinProcMsgListener().AddEvent(WM_HSCROLL, WM_HScrollEvent);
	Listener::WinProcMsgListener().AddEvent(WM_VSCROLL, WM_VScrollEvent);
	Listener::WinProcMsgListener().AddEvent(WM_CLOSE, WM_CloseEvent);
	Listener::WinProcMsgListener().AddEvent(WM_DESTROY, WM_DestroyEvent);
	Listener::WinProcMsgListener().AddEvent(WM_GETMINMAXINFO, WM_GetMinMaxInfo);
	Listener::WinProcMsgListener().AddEvent(WM_SETCURSOR, WM_SetCursorEvent);

	// Uygulama Penceresini Başlatma ve Maksimize Etme
	if (!InitInstance(hInstance, SW_MAXIMIZE))
	{
		return FALSE;
	}

	// Çocuk Pencereyi Oluşturma
	WNDCLASSEXW childClass;
	childClass.cbSize = sizeof(WNDCLASSEX);
	childClass.style = CS_HREDRAW | CS_VREDRAW;
	childClass.lpfnWndProc = ChildWndProc;
	childClass.cbClsExtra = 0;
	childClass.cbWndExtra = 0;
	childClass.hInstance = hInstance;
	childClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINT_PROGRAMING));
	childClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	childClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	childClass.lpszMenuName = L"Child";
	childClass.lpszClassName = Globals::var().szChildClass;
	childClass.hIconSm = LoadIcon(childClass.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassEx(&childClass);
	// Sekil kutusu
	Globals::var().myChildWindow = CreateWindow(Globals::var().szChildClass, L"Tool Box", WS_CAPTION | WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 77, 320, Globals::var().hWndFather, (HMENU)103, Globals::var().hInst, NULL);

	// Araç Kutusu Oluşturma
	Globals::var().myButton[0] = CreateWindow(L"BUTTON", L"B1", WS_VISIBLE | WS_CHILD | BS_BITMAP, 5, 5, 50, 50, Globals::var().myChildWindow, (HMENU)120, Globals::var().hInst, NULL);
	Globals::var().myButton[1] = CreateWindow(L"BUTTON", L"B2", WS_VISIBLE | WS_CHILD | BS_BITMAP, 5, 60, 50, 50, Globals::var().myChildWindow, (HMENU)121, Globals::var().hInst, NULL);
	Globals::var().myButton[2] = CreateWindow(L"BUTTON", L"B3", WS_VISIBLE | WS_CHILD | BS_BITMAP, 5, 115, 50, 50, Globals::var().myChildWindow, (HMENU)122, Globals::var().hInst, NULL);
	Globals::var().myButton[3] = CreateWindow(L"BUTTON", L"B4", WS_VISIBLE | WS_CHILD | BS_BITMAP, 5, 170, 50, 50, Globals::var().myChildWindow, (HMENU)123, Globals::var().hInst, NULL);
	Globals::var().myButton[4] = CreateWindow(L"BUTTON", L"B5", WS_VISIBLE | WS_CHILD | BS_BITMAP, 5, 225, 50, 50, Globals::var().myChildWindow, (HMENU)124, Globals::var().hInst, NULL);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PAINT_PROGRAMING));

	MSG msg;

	// 
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//   MyRegisterClass()
//
//  
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	// WNDCLASSEXW yapısını oluştur
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); // Yapı boyutunu ayarla

	// Pencere stilini belirle
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	// Pencere işleme fonksiyonunu belirle
	wcex.lpfnWndProc = WndProc;
	// Ekstra sınıf verisi için bellek ayırma (bu örnekte kullanılmıyor)
	wcex.cbClsExtra = 0;
	// Ekstra pencere verisi için bellek ayırma (bu örnekte kullanılmıyor)
	wcex.cbWndExtra = 0;
	// Uygulama örneğini kaydet
	wcex.hInstance = hInstance;
	// Pencere ikonunu yükle
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINT_PROGRAMING));
	// Fare imgesini yükle
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	// Pencere arka planını belirle (beyaz renkli pencere)
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	// Menüyü belirle (bu örnekte kaynak dosyadan yükleniyor)
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PAINT_PROGRAMING);
	// Pencere sınıf adını belirle (Globals::var().szWindowClass)
	wcex.lpszClassName = Globals::var().szWindowClass;
	// Küçük ikonu yükle
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	// WNDCLASSEXW yapısını kaydet ve ATOM değerini döndür
	return RegisterClassExW(&wcex);
}

//
//   InitInstance()
//
//  
//

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// Uygulama örneğini kaydet
	Globals::var().hInst = hInstance;

	// Ana pencereyi oluştur
	Globals::var().hWndFather = CreateWindowW(Globals::var().szWindowClass, // Pencere sınıf adı
		Globals::var().szTitle, // Pencere başlığı
		WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, // Pencere stil ve özellikleri
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, // Pencere pozisyonu ve boyutu (varsayılan değerler kullanılır)
		nullptr, // Ebeveyn pencere yok
		nullptr, // Menü yok
		hInstance, // Uygulama örneği
		nullptr); // Ekstra veri yok

	// Pencere oluşturulamazsa FALSE değerini döndür
	if (!Globals::var().hWndFather)
	{
		return FALSE;
	}

	// Pencereyi göster
	ShowWindow(Globals::var().hWndFather, nCmdShow);
	// Pencereyi güncelle
	UpdateWindow(Globals::var().hWndFather);

	// Pencere oluşturulduğu için TRUE değerini döndür
	return TRUE;
}


//
//  WinMain: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  
//
//  WM_COMMAND 
//  WM_PAINT   
//  WM_DESTROY 
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Listener sınıfından WinProcMsgListener nesnesini kullanarak olayı tetikle.
	// Trig() fonksiyonu, belirli bir olay türünü işlemek için uygun olay işleyicisini çağırır.
	return Listener::WinProcMsgListener().Trig(message, Parameter(hWnd, message, wParam, lParam));
}


// child window geri cagırma
LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	int wmId;
	static HBITMAP bmpIcon1, bmpIcon2, bmpIcon3, bmpIcon4, bmpIcon5, bmpIcon6;   // bir düğme için bir bitmap ikon

	switch (message)
	{
	case WM_CREATE:
		// Düğme ikonlarını yükleyin
		bmpIcon1 = (HBITMAP)LoadImage(NULL, L"line.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bmpIcon2 = (HBITMAP)LoadImage(NULL, L"rect.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bmpIcon3 = (HBITMAP)LoadImage(NULL, L"circle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bmpIcon6 = (HBITMAP)LoadImage(NULL, L"ellipse.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bmpIcon4 = (HBITMAP)LoadImage(NULL, L"text.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bmpIcon5 = (HBITMAP)LoadImage(NULL, L"select.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		break;
	case WM_SIZE:
		// Pencere boyutu değiştiğinde gerekirse burada işlemler yapılabilir
		// InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_COMMAND:
		// WM_COMMAND olayını işle
		wmId = LOWORD(wParam);
		// Event
		SendMessage(Globals::var().hWndFather, WM_COMMAND, wmId, 0);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// Çocuk pencere düğmeleri için ikonları ayarlayın
		SendMessage(Globals::var().myButton[0], BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmpIcon1);
		SendMessage(Globals::var().myButton[1], BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmpIcon2);
		SendMessage(Globals::var().myButton[2], BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmpIcon3);
		SendMessage(Globals::var().myButton[3], BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmpIcon4);
		SendMessage(Globals::var().myButton[4], BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmpIcon5);
		SendMessage(Globals::var().myButton[Globals::var().currentDrawMode], BM_SETSTATE, BN_PUSHED, 0);

		// Eğer bir ikon yüklenemediyse hata mesajı göster
		if (!bmpIcon1)
			MessageBox(hWnd, 0, TEXT("NO IMAGE"), MB_OK); // Bilgi mesajı

		EndPaint(hWnd, &ps);
		ReleaseDC(hWnd, hdc);
		break;
	case WM_DESTROY:
		// Kullanılan bitmap nesnelerini sil
		DeleteObject(bmpIcon1);
		DeleteObject(bmpIcon2);
		DeleteObject(bmpIcon3);
		DeleteObject(bmpIcon4);
		DeleteObject(bmpIcon5);
		DeleteObject(bmpIcon6);
		PostQuitMessage(0); // Uygulamayı kapat
		break;
	case WM_WINDOWPOSCHANGING:  // Sürükleme yapıldığında geçerli
	{
		WINDOWPOS* pos = (WINDOWPOS*)lParam;
		RECT rectFather;
		GetWindowRect(Globals::var().hWndFather, &rectFather);

		// Pencere sınırlamaları uygula
		if (pos->x < 0)
			pos->x = 0;

		if (pos->y < 0)
			pos->y = 0;

		if (pos->y > rectFather.bottom - rectFather.top - 397)
			pos->y = rectFather.bottom - rectFather.top - 397;

		if (pos->x > rectFather.right - rectFather.left - 110)
			pos->x = rectFather.right - rectFather.left - 110;
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//x ve y odağı sınırın dışındaysa pencereyi kaydırın. X, Y odağı farenin ekrandaki konumudur
void AutoScroll(HWND hwnd, int Xfocus, int Yfocus, int xCurrentScroll, int yCurrentScroll, RECT windowRect)
{
	// Pencere dikdörtgenini al
	RECT rect;
	GetWindowRect(hwnd, &rect);

	// X Ekseninde Otomatik Kaydırma
	if (Xfocus > (rect.right - rect.left - 29) && xCurrentScroll < 2000)
	{
		WPARAM wParam;
		// Eğer çizim modu DrawTexts ise, yatay kaydırma değerine (Xfocus - windowRect.right) + 1 eklenir.
		// Eğer çizim modu DrawTexts değilse, yatay kaydırma değerine (Xfocus - windowRect.right) eklenir.
		wParam = MAKEWPARAM(SB_THUMBTRACK, xCurrentScroll + (Xfocus - windowRect.right) + (Globals::var().currentDrawMode == DrawTexts ? 1 : 0));
		SendMessage(hwnd, WM_HSCROLL, wParam, NULL);
	}
	else if (xCurrentScroll > 0 && Xfocus <= 0)
	{
		WPARAM wParam;
		// Eğer çizim modu DrawTexts ise, yatay kaydırma değerine (xCurrentScroll + Xfocus - 8) eklenir (yeni bir metin girişi için bir boşluk bırakılır).
		// Eğer çizim modu DrawTexts değilse, yatay kaydırma değerine (xCurrentScroll + Xfocus - 5) eklenir.
		wParam = MAKEWPARAM(SB_THUMBTRACK, (xCurrentScroll + Xfocus - (Globals::var().currentDrawMode == DrawTexts ? 8 : 5)) < 0 ? 0 : xCurrentScroll + Xfocus - (Globals::var().currentDrawMode == DrawTexts ? 8 : 5));
		SendMessage(hwnd, WM_HSCROLL, wParam, NULL);
	}

	// Y Ekseninde Otomatik Kaydırma
	if (Yfocus > windowRect.bottom && yCurrentScroll < 2000)
	{
		// Dikey kaydırma değerine (Yfocus - windowRect.bottom) eklenir.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, yCurrentScroll + (Yfocus - windowRect.bottom));
		SendMessage(hwnd, WM_VSCROLL, wParam, NULL);
	}
	else if (yCurrentScroll > 0 && Yfocus < 0)
	{
		WPARAM wParam;
		// Eğer çizim modu DrawTexts ise, dikey kaydırma değerine (yCurrentScroll + Yfocus - 10) eklenir (yeni bir metin girişi için bir boşluk bırakılır).
		// Eğer çizim modu DrawTexts değilse, dikey kaydırma değerine (yCurrentScroll + Yfocus) eklenir.
		wParam = MAKEWPARAM(SB_THUMBTRACK, (yCurrentScroll + Yfocus - (Globals::var().currentDrawMode == DrawTexts ? 10 : 0)) < 0 ? 0 : yCurrentScroll + Yfocus - (Globals::var().currentDrawMode == DrawTexts ? 10 : 0));
		SendMessage(hwnd, WM_VSCROLL, wParam, NULL);
	}
}


//nesneyi taşırken kaydırma yapmak için.
//x ve y odağı sınırın dışındaysa pencereyi kaydırın. X, Y odağı farenin ekrandaki konumudur
void AutoScrollObject(HWND hwnd, const DrawObj* obj, int xCurrentScroll, int yCurrentScroll, RECT windowRect)
{
	//RECT rect;
	//GetWindowRect(hwnd, &rect);

	int top = (obj->ptBeg.y < obj->ptEnd.y ? obj->ptBeg.y : obj->ptEnd.y);
	int left = (obj->ptBeg.x < obj->ptEnd.x ? obj->ptBeg.x : obj->ptEnd.x);
	int bottom = (obj->ptBeg.y > obj->ptEnd.y ? obj->ptBeg.y : obj->ptEnd.y);
	int right = (obj->ptBeg.x > obj->ptEnd.x ? obj->ptBeg.x : obj->ptEnd.x);
	Globals::var().autoScrolling = true;

	POINT p;  //p fare konumudur
	GetCursorPos(&p);
	ScreenToClient(hwnd, &p);

	// x konumu
	if (right - xCurrentScroll > windowRect.right && xCurrentScroll < 2000 && p.x > windowRect.right / 2)
	{
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, right - windowRect.right);
		SendMessage(hwnd, WM_HSCROLL, wParam, NULL);
	}
	else if (xCurrentScroll > 0 && left - xCurrentScroll < 0 && p.x < windowRect.right / 2)
	{
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, left < 0 ? 0 : left);
		SendMessage(hwnd, WM_HSCROLL, wParam, NULL);
	}

	// y konumu
	if (bottom - yCurrentScroll > windowRect.bottom && yCurrentScroll < 2000 && p.y > windowRect.bottom / 2) 
	{
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, bottom - windowRect.bottom);
		SendMessage(hwnd, WM_VSCROLL, wParam, NULL);
	}
	else if (yCurrentScroll > 0 && top - yCurrentScroll < 0 && p.y < windowRect.bottom / 2)
	{
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, top < 0 ? 0 : top);
		SendMessage(hwnd, WM_VSCROLL, wParam, NULL);
	}

	Globals::var().autoScrolling = false;
}

//nesneyi yeniden boyutlandırırken kaydırma yapmak için.
//fare ve nesne sınırı aynı seçenekteyken kaydırma etkilidir.
void AutoScrollObjectResize(HWND hwnd, const DrawObj* obj, int xCurrentScroll, int yCurrentScroll, RECT windowRect)
{
	// Objenin başlangıç ve bitiş noktalarını kontrol ederek, 
	// objenin çizim alanını belirle.
	int top = (obj->ptBeg.y < obj->ptEnd.y ? obj->ptBeg.y : obj->ptEnd.y);
	int left = (obj->ptBeg.x < obj->ptEnd.x ? obj->ptBeg.x : obj->ptEnd.x);
	int bottom = (obj->ptBeg.y > obj->ptEnd.y ? obj->ptBeg.y : obj->ptEnd.y);
	int right = (obj->ptBeg.x > obj->ptEnd.x ? obj->ptBeg.x : obj->ptEnd.x);

	// Otomatik kaydırma işlemi başlıyor, bu nedenle autoScrolling değişkeni true yapılır.
	Globals::var().autoScrolling = true;

	// Farenin ekran üzerindeki pozisyonunu al.
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(hwnd, &p);

	// X ekseninde otomatik kaydırma işlemi
	if (right - xCurrentScroll > windowRect.right && xCurrentScroll < 2000 && abs(p.x - right + xCurrentScroll) < 30)
	{
		// Sağa doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, right - windowRect.right);
		SendMessage(hwnd, WM_HSCROLL, wParam, NULL);
	}
	else if (xCurrentScroll > 0 && left - xCurrentScroll < 0 && abs(p.x - left + xCurrentScroll) < 30)
	{
		// Sola doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, left - 1 < 0 ? 0 : left - 1);
		SendMessage(hwnd, WM_HSCROLL, wParam, NULL);
	}
	else if (right - xCurrentScroll < 0 && xCurrentScroll > 0 && abs(p.x - right + xCurrentScroll) < 30)
	{
		// Sağa doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, right - windowRect.right);
		SendMessage(hwnd, WM_HSCROLL, wParam, NULL);
	}
	else if (xCurrentScroll < 2000 && left - xCurrentScroll > windowRect.right && abs(p.x - left + xCurrentScroll) < 30)
	{
		// Sola doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, left - 1 < 0 ? 0 : left - 1);
		SendMessage(hwnd, WM_HSCROLL, wParam, NULL);
	}

	// Y ekseninde otomatik kaydırma işlemi
	if (bottom - yCurrentScroll > windowRect.bottom && yCurrentScroll < 2000 && abs(p.y - bottom + yCurrentScroll) < 30)
	{
		// Aşağı doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, bottom - windowRect.bottom);
		SendMessage(hwnd, WM_VSCROLL, wParam, NULL);
	}
	else if (yCurrentScroll > 0 && top - yCurrentScroll < 0 && abs(p.y - top + yCurrentScroll) < 30)
	{
		// Yukarı doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, top < 0 ? 0 : top);
		SendMessage(hwnd, WM_VSCROLL, wParam, NULL);
	}
	else if (bottom - yCurrentScroll < 0 && yCurrentScroll > 0 && abs(p.y - bottom + yCurrentScroll) < 30)
	{
		// Aşağı doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, bottom - 1 < 0 ? 0 : bottom - 1);
		SendMessage(hwnd, WM_VSCROLL, wParam, NULL);
	}
	else if (top - yCurrentScroll > windowRect.bottom && yCurrentScroll < 2000 && abs(p.y - top + yCurrentScroll) < 30)
	{
		// Yukarı doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, top - windowRect.bottom);
		SendMessage(hwnd, WM_VSCROLL, wParam, NULL);
	}

	// Otomatik kaydırma işlemi tamamlandı, autoScrolling değişkeni false yapılır.
	Globals::var().autoScrolling = false;
}


//nesne çizerken kaydırma yapmak için.
// ptEnd'i yalnızca nesne ptEnd üzerinde çizilirken karşılaştırırız
void AutoScrollObjectWhenDrawing(HWND hwnd, const DrawObj* obj, int xCurrentScroll, int yCurrentScroll, RECT windowRect)
{
	// Otomatik kaydırma işlemi başlıyor, bu nedenle autoScrolling değişkeni true yapılır.
	Globals::var().autoScrolling = true;

	// X ekseninde otomatik kaydırma işlemi
	if (obj->ptEnd.x - xCurrentScroll > windowRect.right && xCurrentScroll < 2000)
	{
		// Sağa doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, obj->ptEnd.x - windowRect.right);
		SendMessage(hwnd, WM_HSCROLL, wParam, NULL);
	}
	else if (xCurrentScroll > 0 && obj->ptEnd.x - xCurrentScroll < 0)
	{
		// Sola doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, obj->ptEnd.x < 0 ? 0 : obj->ptEnd.x);
		SendMessage(hwnd, WM_HSCROLL, wParam, NULL);
	}

	// Y ekseninde otomatik kaydırma işlemi
	if (obj->ptEnd.y - yCurrentScroll > windowRect.bottom && yCurrentScroll < 2000)
	{
		// Aşağı doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, obj->ptEnd.y - windowRect.bottom);
		SendMessage(hwnd, WM_VSCROLL, wParam, NULL);
	}
	else if (yCurrentScroll > 0 && obj->ptEnd.y - yCurrentScroll < 0)
	{
		// Yukarı doğru kaydırma işlemi yapılır.
		WPARAM wParam = MAKEWPARAM(SB_THUMBTRACK, obj->ptEnd.y < 0 ? 0 : obj->ptEnd.y);
		SendMessage(hwnd, WM_VSCROLL, wParam, NULL);
	}

	// Otomatik kaydırma işlemi tamamlandı, autoScrolling değişkeni false yapılır.
	Globals::var().autoScrolling = false;
}

// Belirtilen pencerenin yerel koordinatlarını hesaplayan fonksiyon.
RECT GetLocalCoordinates(HWND hWnd)
{
	RECT Rect;

	// Pencerenin ekran koordinatlarını alır.
	GetWindowRect(hWnd, &Rect);

	// Pencerenin yerel koordinatlarına dönüştürülür.
	MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), (LPPOINT)&Rect, 2);

	// Yerel koordinatları içeren RECT yapısını döndürür.
	return Rect;
}


// Çizim nesnelerini temizleyen fonksiyon.
void CleanObjects(HWND hWnd)
{
	// DrawObjList içindeki her öğe için bellekten alınan alanı serbest bırak.
	for (auto& it : Globals::var().drawObjList)
		delete(it);

	// DrawObjList'i temizle. clear() fonksiyonu bellekten alanı temizlemez, sadece listeyi boşaltır.
	Globals::var().drawObjList.clear();

	// Seçili nesne işaretçisini ve seçim durumunu sıfırla.
	Globals::var().selectedObjectPtr = nullptr;
	Globals::var().hasSelected = false;

	// Kopyala/Yapıştır nesne işaretçisini sıfırla.
	Globals::var().pastebinObjectPtr = nullptr;

	// Pencereyi geçersiz kıl, böylece içeriği yeniden çizilir.
	InvalidateRect(hWnd, NULL, TRUE);
}


// Başlık çubuğunu belirtilen metinle güncelleyen fonksiyon.
void SetTitle(string name, HWND hWnd)
{
	// Yeni başlık metnini oluştur.
	string s = "Paint Program - [" + name + "]";

	// string'i wstring'e dönüştür.
	wstring stemp = wstring(s.begin(), s.end());

	// wstring'i LPCWSTR türüne dönüştür.
	LPCWSTR sw = stemp.c_str();

	// Pencerenin başlık çubuğunu güncelle.
	SetWindowText(hWnd, sw);
}


// Araç seçim durumunu değiştiren fonksiyon.
void ChangeToolsSelectionState(int position, HMENU hMenu)
{
	// Eğer bir nesne seçiliyse ve yeni seçilen pozisyon seçili nesnenin pozisyonuyla aynı değilse, seçili nesneyi bırak.
	if (Globals::var().selectedObjectPtr != nullptr && position != 4)
		Globals::var().selectedObjectPtr = nullptr;

	// 'Edit' menüsünü al
	HMENU hMenu2 = GetSubMenu(hMenu, 2); // hMenu2 = Edit

	// Araç çubuğundaki her bir düğme için durumu ayarla
	for (int i = 0; i < 5; i++)
	{
		if (i == position)
		{
			// Yeni seçilen pozisyon seçili yapılır, ilgili menü öğesi de işaretlenir.
			SendMessage(Globals::var().myButton[i], BM_SETSTATE, BN_PUSHED, 0);
			CheckMenuItem(hMenu2, i, MF_CHECKED | MF_BYPOSITION);
		}
		else
		{
			// Diğer pozisyonlar seçili değil yapılır, ilgili menü öğesi işaretsiz yapılır.
			SendMessage(Globals::var().myButton[i], BM_SETSTATE, 0, 0);
			CheckMenuItem(hMenu2, i, MF_UNCHECKED | MF_BYPOSITION);
		}
	}
}


// Renk seçim durumunu değiştiren fonksiyon.
void ChangeColorsSelectionState(int position, HMENU hMenu)
{
	// Eğer metin çizme modundayken ve yeni metin nesnesi tamamlanmışsa, yeni metin nesnesinin rengini değiştir.
	if (Globals::var().currentDrawMode == DrawTexts && Globals::var().newText.startFinished)
	{
		Globals::var().mlog.OP_modifyStart(&Globals::var().newText, -1);
		Globals::var().newText.color = position;
		Globals::var().modifyState = 1;
		Globals::var().mlog.OP_modifyEnd(&Globals::var().newText);
		InvalidateRect(Globals::var().hWndFather, NULL, FALSE);
	}
	// Eğer seçili bir nesne varsa, seçili nesnenin rengini değiştir.
	else if (Globals::var().selectedObjectPtr != nullptr)
	{
		// Seçili nesnenin listedeki konumunu bul.
		auto it = find(Globals::var().drawObjList.begin(), Globals::var().drawObjList.end(), Globals::var().selectedObjectPtr);
		if (it != Globals::var().drawObjList.end())
		{
			int pos = distance(Globals::var().drawObjList.begin(), it);
			Globals::var().mlog.OP_modifyStart(Globals::var().selectedObjectPtr, pos);
			Globals::var().selectedObjectPtr->color = position;
			Globals::var().modifyState = 1;
			Globals::var().mlog.OP_modifyEnd(Globals::var().selectedObjectPtr);
			InvalidateRect(Globals::var().hWndFather, NULL, FALSE);
		}
	}

	// 'Edit' menüsünü al
	HMENU hMenu2 = GetSubMenu(hMenu, 2); // hMenu2 = Edit
	// 'Edit' menüsündeki 'Delete' alt menüsünü al
	HMENU hMenu3 = GetSubMenu(hMenu2, 6); // hMenu3 = Edit - Delete

	// Renk seçeneklerinde dolaşarak ilgili menü öğelerini işaretle veya işaretsiz yap.
	for (int i = 0; i < 8; i++)
	{
		if (i == position)
		{
			CheckMenuItem(hMenu3, i, MF_CHECKED | MF_BYPOSITION);
		}
		else
		{
			CheckMenuItem(hMenu3, i, MF_UNCHECKED | MF_BYPOSITION);
		}
	}
}


// Arka plan rengi seçim durumunu değiştiren fonksiyon.
void ChangeBGSelectionState(int position, HMENU hMenu)
{
	// Eğer metin çizme modundayken ve yeni metin nesnesi tamamlanmışsa, yeni metin nesnesinin arka plan rengini değiştir.
	if (Globals::var().currentDrawMode == DrawTexts && Globals::var().newText.startFinished)
	{
		Globals::var().mlog.OP_modifyStart(&Globals::var().newText, -1);
		Globals::var().newText.backgroundColor = position;
		Globals::var().modifyState = 1;
		Globals::var().mlog.OP_modifyEnd(&Globals::var().newText);
		InvalidateRect(Globals::var().hWndFather, NULL, FALSE);
	}
	// Eğer seçili bir nesne varsa, seçili nesnenin arka plan rengini değiştir.
	else if (Globals::var().selectedObjectPtr != nullptr)
	{
		// Seçili nesnenin listedeki konumunu bul.
		auto it = find(Globals::var().drawObjList.begin(), Globals::var().drawObjList.end(), Globals::var().selectedObjectPtr);
		if (it != Globals::var().drawObjList.end())
		{
			int pos = distance(Globals::var().drawObjList.begin(), it);
			Globals::var().mlog.OP_modifyStart(Globals::var().selectedObjectPtr, pos);
			Globals::var().selectedObjectPtr->backgroundColor = position;
			Globals::var().modifyState = 1;
			Globals::var().mlog.OP_modifyEnd(Globals::var().selectedObjectPtr);
			InvalidateRect(Globals::var().hWndFather, NULL, FALSE);
		}
	}

	// 'Edit' menüsünü al
	HMENU hMenu2 = GetSubMenu(hMenu, 2); // hMenu2 = Edit
	// 'Edit' menüsündeki 'Undo' alt menüsünü al
	HMENU hMenu5 = GetSubMenu(hMenu2, 8); // hMenu5 = Edit - Undo

	// Arka plan rengi seçeneklerinde dolaşarak ilgili menü öğelerini işaretle veya işaretsiz yap.
	for (int i = 0; i < 8; i++)
	{
		if (i == position)
		{
			CheckMenuItem(hMenu5, i, MF_CHECKED | MF_BYPOSITION);
		}
		else
		{
			CheckMenuItem(hMenu5, i, MF_UNCHECKED | MF_BYPOSITION);
		}
	}
}


// Çizgi kalınlığı seçim durumunu değiştiren fonksiyon.
void ChangeLineSelectionState(int position, HMENU hMenu)
{
	// Eğer seçili bir nesne varsa ve seçili nesne metin değilse, seçili nesnenin çizgi kalınlığını değiştir.
	if (Globals::var().selectedObjectPtr != nullptr && Globals::var().selectedObjectPtr->objectType != 4)
	{
		// Seçili nesnenin listedeki konumunu bul.
		auto it = find(Globals::var().drawObjList.begin(), Globals::var().drawObjList.end(), Globals::var().selectedObjectPtr);
		if (it != Globals::var().drawObjList.end())
		{
			int pos = distance(Globals::var().drawObjList.begin(), it);
			Globals::var().mlog.OP_modifyStart(Globals::var().selectedObjectPtr, pos);
			// Seçili nesnenin çizgi kalınlığını değiştir.
			Globals::var().selectedObjectPtr->lineWidth = position;
			Globals::var().mlog.OP_modifyEnd(Globals::var().selectedObjectPtr);
			InvalidateRect(Globals::var().hWndFather, NULL, FALSE);
		}
	}

	// 'Edit' menüsünü al
	HMENU hMenu2 = GetSubMenu(hMenu, 2); // hMenu2 = Edit
	// 'Edit' menüsündeki 'Redo' alt menüsünü al
	HMENU hMenu4 = GetSubMenu(hMenu2, 7); // hMenu4 = Edit - Redo

	// Çizgi kalınlığı seçeneklerinde dolaşarak ilgili menü öğelerini işaretle veya işaretsiz yap.
	for (int i = 0; i < 5; i++)
	{
		if (i == position - 1)
		{
			CheckMenuItem(hMenu4, i, MF_CHECKED | MF_BYPOSITION);
		}
		else
		{
			CheckMenuItem(hMenu4, i, MF_UNCHECKED | MF_BYPOSITION);
		}
	}
}

