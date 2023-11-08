#include "stdafx.h"
#include "Windowsx.h "
#include "WM_Command.h"
#include "Globals.h"
#include "Save.h"
#include "Paint_Programing.h"
#include "mylog.h"

static int mouseX, mouseY;
static LineObj  newLine;
static RectangularObj newRect;
static CircleObj newCircle;
static bool mouseHasDown = false;

static RECT rect;
static TCHAR szBuffer[50];
static int cxChar, cyChar;
static int xPasteDir, yPasteDir;  // yapıştır = 1; yapıştırma = 0

SCROLLINFO si;
// Bu değişkenler BitBlt tarafından gereklidir.
//static HDC hdcWin;           // window DC
static HDC hdcScreen;       // Tüm ekran için DC
							 ///static HDC hdcScreenCompat; // ekran için bellek DC
							 ///static HBITMAP hbmpCompat;   //eski DC'ye bitmap tanıtıcısı
static BITMAP bmp;           // bitmap veri yapısı
static BOOL fBlt;            // BitBlt meydana gelirse DOĞRU
static BOOL fScroll;         // Kaydırma gerçekleştiyse DOĞRU
static BOOL fSize;           // fBlt ve WM_SIZE ise DOĞRU

							 // Bu değişkenler yatay kaydırma için gereklidir.
static int xMinScroll;       // minimum yatay kaydırma değeri
static int xCurrentScroll;   // geçerli yatay kaydırma değeri
static int xMaxScroll;       // maximum yatay kaydırma değeri

							 // Bu değişkenler dikey kaydırma için gereklidir.
static int yMinScroll;       // minimum dikey kaydırma değeri
static int yCurrentScroll;   // geçerli dikey kaydırma değeri
static int yMaxScroll;       // maximum dikey kaydırma değeri
static HMENU hMenu = NULL;     //sistem menüsünü almayı deneyin
static HBITMAP hBmp;         //bellek DC için bitmap

static int currentColor;        //0=siyah, 0-7 renk çeşitleri
static int currentBgColor;      //0=saydam, 0~7 renk çeşitleri
static int currentLineWidth;    //1-5 kalınlık derecesi
static int currentCursorMode;   //0=orijinal 1=siyah 2=gri 3=kırmızı 4=yeşil 5=mavi 6=koyu yeşil 7=sarı 8=pembe
static HCURSOR cursors[6];      //0=orijinal 1=siyah 2=gri 3=kırmızı 4=yeşil 5=mavi 
static bool isMoving, isResizing;
static int hScrollResize = 0;   //tam ekran kaydırma boyutu değişikliğini önlemek için sayaç. 2'ye kadar sayılırsa atlama yapacak.
static HCURSOR * currentCursor;

LRESULT DefaultEvnetHandler(Parameter& param)
{
	return DefWindowProc(param.hWnd_, param.message_, param.wParam_, param.lParam_);
}

LRESULT WM_CommandEvent(Parameter& param)
{
	int wmId = LOWORD(param.wParam_);
	// durum
	switch (wmId)
	{
	case IDM_ABOUT:  //program hakkında
		DialogBox(Globals::var().hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), param.hWnd_, About);
		break;
	case IDM_EXIT:  //program çıkış
		SendMessage(param.hWnd_, WM_CLOSE, NULL, NULL);
		break;
		DestroyWindow(param.hWnd_);
		break;
	case ID_COMMAND_1:  //Painter temizle
	{
		if (Globals::var().modifyState == 1) //kullanıcı ekranı kaydetme iletişim kutusu
		{
			if (DisplayConfirmClearMessageBox(Globals::var().fileName) == IDYES)
			{
				PushCurrentNewText(Globals::var().newText);
				if (Globals::var().lastFilePath.size() < 1)
				{
					SaveToFile(Globals::var().drawObjList, Globals::var().fileName);
					SetTitle(Globals::var().fileName, param.hWnd_);
					Globals::var().modifyState = 2;
				}
				else
				{
					SaveToLastFilePath(Globals::var().drawObjList);
					Globals::var().modifyState = 2;
				}
			}
		}
		int oldState = Globals::var().modifyState;
		Globals::var().mlog.ClearLogs();
		if (Globals::var().drawObjList.size() > 0)  //değişiklik durumunu ayarla
			Globals::var().modifyState = 1;
		else
			Globals::var().modifyState = oldState;
		newLine.Clean();
		newRect.Clean();
		Globals::var().newText.Clean();
		newCircle.Clean();
		Globals::var().selectedObjectPtr = NULL;
		currentCursorMode = 0;
		Globals::var().hasSelected = false;
		CleanObjects(param.hWnd_);
		break;
	}
	case 120:
		SetFocus(param.hWnd_);  //focus'u ana pencereye geri döndür
	case ID_LineTool:
		Globals::var().currentDrawMode = DrawLine;
		ChangeToolsSelectionState(Globals::var().currentDrawMode, hMenu);
		PushCurrentNewText(Globals::var().newText);
		Globals::var().hasSelected = false;
		InvalidateRect(param.hWnd_, NULL, FALSE);
		break;
	case 121:
		SetFocus(param.hWnd_);
	case ID_RectTool:
		Globals::var().currentDrawMode = DrawRect;
		ChangeToolsSelectionState(Globals::var().currentDrawMode, hMenu);
		PushCurrentNewText(Globals::var().newText);
		Globals::var().hasSelected = false;
		InvalidateRect(param.hWnd_, NULL, FALSE);
		break;
	case 122:
		SetFocus(param.hWnd_);
	case ID_CircleTool:
		Globals::var().currentDrawMode = DrawCircle;
		ChangeToolsSelectionState(Globals::var().currentDrawMode, hMenu);
		PushCurrentNewText(Globals::var().newText);
		Globals::var().hasSelected = false;
		InvalidateRect(param.hWnd_, NULL, FALSE);
		break;
	case 123:
		SetFocus(param.hWnd_);
	case ID_TextTool:
		Globals::var().currentDrawMode = DrawTexts;
		Globals::var().hasSelected = false;
		ChangeToolsSelectionState(Globals::var().currentDrawMode, hMenu);
		break;
	case 124:
		SetFocus(param.hWnd_);
	case ID_SelectTool:
		Globals::var().currentDrawMode = SelectTool;
		PushCurrentNewText(Globals::var().newText);
		ChangeToolsSelectionState(Globals::var().currentDrawMode, hMenu);
		InvalidateRect(param.hWnd_, NULL, FALSE);
		break;
	case ID_BLACK:
		currentColor = 0;
		ChangeColorsSelectionState(currentColor, hMenu);
		break;
	case ID_GRAY:
		currentColor = 1;
		ChangeColorsSelectionState(currentColor, hMenu);
		break;
	case ID_RED:
		currentColor = 2;
		ChangeColorsSelectionState(currentColor, hMenu);
		break;
	case ID_GREEN:
		currentColor = 3;
		ChangeColorsSelectionState(currentColor, hMenu);
		break;
	case ID_BLU:
		currentColor = 4;
		ChangeColorsSelectionState(currentColor, hMenu);
		break;
	case ID_CYAN:
		currentColor = 5;
		ChangeColorsSelectionState(currentColor, hMenu);
		break;
	case ID_YELLOW:
		currentColor = 6;
		ChangeColorsSelectionState(currentColor, hMenu);
		break;
	case ID_Magenta:
		currentColor = 7;
		ChangeColorsSelectionState(currentColor, hMenu);
		break;
	case ID_BG_Transparent:
		currentBgColor = 0;
		ChangeBGSelectionState(currentBgColor, hMenu);
		break;
	case  ID_BG_GRAY:
		currentBgColor = 1;
		ChangeBGSelectionState(currentBgColor, hMenu);
		break;
	case  ID_BG_RED:
		currentBgColor = 2;
		ChangeBGSelectionState(currentBgColor, hMenu);
		break;
	case  ID_BG_GREEN:
		currentBgColor = 3;
		ChangeBGSelectionState(currentBgColor, hMenu);
		break;
	case  ID_BG_BLU:
		currentBgColor = 4;
		ChangeBGSelectionState(currentBgColor, hMenu);
		break;
	case  ID_BG_CYAN:
		currentBgColor = 5;
		ChangeBGSelectionState(currentBgColor, hMenu);
		break;
	case  ID_BG_YELLOW:
		currentBgColor = 6;
		ChangeBGSelectionState(currentBgColor, hMenu);
		break;
	case  ID_BG_Magenta:
		currentBgColor = 7;
		ChangeBGSelectionState(currentBgColor, hMenu);
		break;
	case ID_Wide1:
		currentLineWidth = 1;
		ChangeLineSelectionState(currentLineWidth, hMenu);
		break;
	case ID_Wide2:
		currentLineWidth = 2;
		ChangeLineSelectionState(currentLineWidth, hMenu);
		break;
	case ID_Wide3:
		currentLineWidth = 3;
		ChangeLineSelectionState(currentLineWidth, hMenu);
		break;
	case ID_Wide4:
		currentLineWidth = 4;
		ChangeLineSelectionState(currentLineWidth, hMenu);
		break;
	case ID_Wide5:
		currentLineWidth = 5;
		ChangeLineSelectionState(currentLineWidth, hMenu);
		break;
	case ID_SAVE:
		if (Globals::var().modifyState == 0 || Globals::var().lastFilePath.size() <1)
			goto SAVE_AS_NEW_FILE;  //eski bir dosya yolu yok
		PushCurrentNewText(Globals::var().newText);
		SaveToLastFilePath(Globals::var().drawObjList);
		Globals::var().modifyState = 2;
		SetTitle(Globals::var().fileName, param.hWnd_);
		break;
	case ID_SAVE_AS:
	{
	SAVE_AS_NEW_FILE:
		PushCurrentNewText(Globals::var().newText);
		SaveToFile(Globals::var().drawObjList, Globals::var().fileName);
		Globals::var().modifyState = 2;
		SetTitle(Globals::var().fileName, param.hWnd_);
		break;
	}
	case ID_NEW_FILE:
		if (Globals::var().modifyState == 1) //kaydetme iletişim kutusunu görüntüle
		{
			if (DisplayConfirmNewFileMessageBox(Globals::var().fileName) == IDYES)
			{
				PushCurrentNewText(Globals::var().newText);
				if (Globals::var().modifyState == 0 || Globals::var().lastFilePath.size() < 1)
					SaveToFile(Globals::var().drawObjList, Globals::var().fileName);  //son açılan dosya yok					
				else
					SaveToLastFilePath(Globals::var().drawObjList);  //son açılan dosyaya kaydet
			}
		}
		Globals::var().fileName = "My Paint";
		Globals::var().lastFilePath.clear();
		SetTitle(Globals::var().fileName, param.hWnd_);
		newLine.Clean();
		newRect.Clean();
		Globals::var().newText.Clean();
		newCircle.Clean();
		Globals::var().selectedObjectPtr = NULL;
		currentCursorMode = 0;
		Globals::var().hasSelected = false;
		CleanObjects(param.hWnd_);
		Globals::var().mlog.ClearLogs();
		Globals::var().modifyState = 0;  //<-bunun ClearLogs()'tan sonra ayarlanması gerekir, aksi halde içeride değiştirilecektir.
		break;
	case ID_OPEN_FILE:
	{
		if (Globals::var().modifyState == 1) //kaydetme iletişim kutusunu görüntüle
		{
			if (DisplayConfirmNewFileMessageBox(Globals::var().fileName) == IDYES)
			{
				PushCurrentNewText(Globals::var().newText);
				if (Globals::var().modifyState == 0 || Globals::var().lastFilePath.size() < 1)
					SaveToFile(Globals::var().drawObjList, Globals::var().fileName);  //son açılan dosya yok				
				else
					SaveToLastFilePath(Globals::var().drawObjList);  //son açılan dosyaya kaydet
			}
		}
		Globals::var().selectedObjectPtr = NULL;
		Globals::var().hasSelected = false;
		currentCursorMode = 0;
		Globals::var().mlog.ClearLogs();
		CleanObjects(param.hWnd_);
		ReadFromFile(Globals::var().drawObjList, Globals::var().fileName);
		newLine.Clean();
		newRect.Clean();
		Globals::var().newText.Clean();
		newCircle.Clean();
		Globals::var().mlog.ClearLogs();
		Globals::var().modifyState = 2;
		SetTitle(Globals::var().fileName, param.hWnd_);
		InvalidateRect(param.hWnd_, NULL, TRUE);
		break;
	}
	case ID_Copy:
	{
		if (Globals::var().selectedObjectPtr == nullptr)
			break;

		if (Globals::var().pastebinObjectPtr != nullptr)
			delete Globals::var().pastebinObjectPtr;

		xPasteDir = 0;  //yapıştırma yönü sol üsttedir
		yPasteDir = 0;

		switch (Globals::var().selectedObjectPtr->objectType)
		{
		case Line:
		{
			LineObj *temp = (LineObj*)Globals::var().selectedObjectPtr;
			Globals::var().pastebinObjectPtr = new LineObj(*temp);
			break;
		}
		case Rect:
		{
			RectangularObj *temp = (RectangularObj*)Globals::var().selectedObjectPtr;
			Globals::var().pastebinObjectPtr = new RectangularObj(*temp);
			break;
		}
		case Circle:
		{
			CircleObj *temp = (CircleObj*)Globals::var().selectedObjectPtr;
			Globals::var().pastebinObjectPtr = new CircleObj(*temp);
			break;
		}
		case Text:
		{
			TextObj *temp = (TextObj*)Globals::var().selectedObjectPtr;
			Globals::var().pastebinObjectPtr = new TextObj(*temp);
			break;
		}
		default:
			break;
		}
		break;
	}
	case ID_Cut:
	{
		if (Globals::var().selectedObjectPtr == nullptr)
			break;

		if (Globals::var().pastebinObjectPtr != nullptr)
			delete Globals::var().pastebinObjectPtr;

		xPasteDir = 0;  //yapıştırma yönü sol üsttedir
		yPasteDir = 0;

		switch (Globals::var().selectedObjectPtr->objectType)
		{
		case Line:
		{
			LineObj *temp = (LineObj*)Globals::var().selectedObjectPtr;
			Globals::var().pastebinObjectPtr = new LineObj(*temp);
			break;
		}
		case Rect:
		{
			RectangularObj *temp = (RectangularObj*)Globals::var().selectedObjectPtr;
			Globals::var().pastebinObjectPtr = new RectangularObj(*temp);
			break;
		}
		case Circle:
		{
			CircleObj *temp = (CircleObj*)Globals::var().selectedObjectPtr;
			Globals::var().pastebinObjectPtr = new CircleObj(*temp);
			break;
		}
		case Text:
		{
			TextObj *temp = (TextObj*)Globals::var().selectedObjectPtr;
			Globals::var().pastebinObjectPtr = new TextObj(*temp);
			break;
		}
		default:
			break;
		}

		//newText'te kesilirse temizleyin.
		if (Globals::var().selectedObjectPtr == &Globals::var().newText)
		{
			Globals::var().newText.Clean();
			Globals::var().selectedObjectPtr = NULL;
			Globals::var().hasSelected = false;
			InvalidateRect(param.hWnd_, NULL, FALSE);
			break;
		}

		//seçilen nesnenin listedeki konumunu bulma
		auto it = std::find(Globals::var().drawObjList.begin(), Globals::var().drawObjList.end(), Globals::var().selectedObjectPtr);
		if (it != Globals::var().drawObjList.end())
		{
			int pos = distance(Globals::var().drawObjList.begin(), it);
			Globals::var().mlog.OP_del(Globals::var().selectedObjectPtr, pos);  //kesilen nesneyi kaydetme
			delete Globals::var().selectedObjectPtr;
			Globals::var().hasSelected = false;
			Globals::var().selectedObjectPtr = nullptr;
			currentCursorMode = 0;
			Globals::var().drawObjList.erase(it);
			InvalidateRect(param.hWnd_, NULL, FALSE);
		}

		break;
	}
	case ID_Paste:
	{
		if (Globals::var().pastebinObjectPtr == nullptr)
			break;

		//yeni yapıştırma konumunu ayarla
		POINT p = MovePastedObj();
		AutoScroll(param.hWnd_, p.x - xCurrentScroll + 1, p.y - yCurrentScroll + 1, xCurrentScroll, yCurrentScroll, rect);

		//geçmiş nesneyi listeye ekle
		switch (Globals::var().pastebinObjectPtr->objectType)
		{
		case Line:
		{
			Globals::var().drawObjList.push_back(new LineObj(*(LineObj*)Globals::var().pastebinObjectPtr));
			break;
		}
		case Rect:
		{
			Globals::var().drawObjList.push_back(new RectangularObj(*(RectangularObj*)Globals::var().pastebinObjectPtr));
			break;
		}
		case Circle:
		{
			Globals::var().drawObjList.push_back(new CircleObj(*(CircleObj*)Globals::var().pastebinObjectPtr));
			break;
		}
		case Text:
		{
			Globals::var().drawObjList.push_back(new TextObj(*(TextObj*)Globals::var().pastebinObjectPtr));
			break;
		}
		default:
			break;
		}

		//yeni nesne seçimi
		Globals::var().selectedObjectPtr = Globals::var().drawObjList.back();
		Globals::var().hasSelected = true;

		//kayıt öncesi hazırlık
		Globals::var().mlog.OP_add(Globals::var().selectedObjectPtr);

		InvalidateRect(param.hWnd_, NULL, FALSE);
		break;
	}
	case ID_Undo:
	{
		Undo();
		InvalidateRect(param.hWnd_, NULL, FALSE);
		break;
	}
	case ID_Redo:
	{
		Redo();
		InvalidateRect(param.hWnd_, NULL, FALSE);
		break;
	}
	case ID_Delete:
	{

		if (Globals::var().selectedObjectPtr == nullptr)
			break;

		if (Globals::var().currentDrawMode == DrawTexts && Globals::var().newText.startFinished) //newText sil
		{
			//log.h'a kayıt etme
			Globals::var().mlog.OP_del(&Globals::var().newText, -1);

			Globals::var().newText.Clean();

			//listedeki son nesneyi seçme
			if (Globals::var().drawObjList.size() > 0)
			{
				Globals::var().selectedObjectPtr = Globals::var().drawObjList.back();
				FocusSelectedObject(Globals::var().selectedObjectPtr, param.hWnd_);
			}
			else
			{
				Globals::var().selectedObjectPtr = nullptr;
				Globals::var().hasSelected = false;
			}
			InvalidateRect(param.hWnd_, NULL, FALSE);
			break;
		}

		//seçilen nesnenin listedeki konumunu bulma
		auto it = std::find(Globals::var().drawObjList.begin(), Globals::var().drawObjList.end(), Globals::var().selectedObjectPtr);
		if (it != Globals::var().drawObjList.end())
		{
			//log.h'a kayıt etme
			int pos = distance(Globals::var().drawObjList.begin(), it);
			Globals::var().mlog.OP_del(*it, pos);

			//silme işlemi yapılıyor
			delete Globals::var().selectedObjectPtr;
			Globals::var().drawObjList.erase(it);

			//önceki nesneyi seç
			it = Globals::var().drawObjList.begin();
			if (Globals::var().drawObjList.size() > 0 && *it != NULL)
			{
				if (pos > 0)
					advance(it, pos - 1);
				Globals::var().selectedObjectPtr = *it;
				FocusSelectedObject(Globals::var().selectedObjectPtr, param.hWnd_);
			}
			else
			{
				Globals::var().selectedObjectPtr = NULL;
				Globals::var().hasSelected = false;
			}
			currentCursorMode = 0;

			InvalidateRect(param.hWnd_, NULL, FALSE);
		}
		break;
	}
	case ID_DebugMode:
	{
		Globals::var().debugMode = !Globals::var().debugMode;
		InvalidateRect(param.hWnd_, NULL, FALSE);
		break;
	}
	default:
	{
		if (Globals::var().debugMode)
		{
			wsprintf(szBuffer, TEXT("Button ID %d : %d"), param.wParam_, param.lParam_);
			MessageBox(param.hWnd_, szBuffer, TEXT("Pressed"), MB_OK);
		}
		return DefWindowProc(param.hWnd_, param.message_, param.wParam_, param.lParam_);
	}
	}
	return 0;
}

LRESULT WM_CreateEvent(Parameter& param)
{
	// DLL dosyasını yükle
	if (LoadLibrary(L"PaintDLL.dll") == NULL)
	{
		MessageBox(0, L"Can not load DLL file.", L"Error",MB_OK);
		return 0;
	}
	currentColor = 0;
	Globals::var().hasSelected = false;
	cxChar = LOWORD(GetDialogBaseUnits());
	cyChar = HIWORD(GetDialogBaseUnits());

	HBITMAP bmpIcon1, bmpIcon2, bmpIcon3, bmpIcon4, bmpIcon5, bmpIcon6, bmpIcon7, bmpIcon8, bmpIcon9;
	HBITMAP line1, line2, line3, line4, line5;
	HBITMAP checkedIcon;

	//alt pencere düğmeleri için resim ekleme
	bmpIcon1 = (HBITMAP)LoadImage(NULL, L"black.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	bmpIcon2 = (HBITMAP)LoadImage(NULL, L"grey.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	bmpIcon3 = (HBITMAP)LoadImage(NULL, L"red.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	bmpIcon4 = (HBITMAP)LoadImage(NULL, L"green.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	bmpIcon5 = (HBITMAP)LoadImage(NULL, L"blu.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	bmpIcon6 = (HBITMAP)LoadImage(NULL, L"cyan.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	bmpIcon7 = (HBITMAP)LoadImage(NULL, L"yellow.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	bmpIcon8 = (HBITMAP)LoadImage(NULL, L"magenta.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	bmpIcon9 = (HBITMAP)LoadImage(NULL, L"transparent.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	checkedIcon = (HBITMAP)LoadImage(NULL, L"checkedIcon.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	line1 = (HBITMAP)LoadImage(NULL, L"wide1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	line2 = (HBITMAP)LoadImage(NULL, L"wide2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	line3 = (HBITMAP)LoadImage(NULL, L"wide3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	line4 = (HBITMAP)LoadImage(NULL, L"wide4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	line5 = (HBITMAP)LoadImage(NULL, L"wide5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	//menüye resim ekleme
	hMenu = GetMenu(param.hWnd_);
	if (hMenu)
	{
		HMENU hMenu2 = GetSubMenu(hMenu, 2);   //hMenu2 = File
		HMENU hMenu3 = GetSubMenu(hMenu2, 6);  //hMenu3 = Edit
		HMENU hMenu4 = GetSubMenu(hMenu2, 7);  //hMenu4 = Tool
		HMENU hMenu5 = GetSubMenu(hMenu2, 8);  //hMenu5 = Help

		ModifyMenu(hMenu3, 0, MF_BYPOSITION | MF_BITMAP, ID_BLACK, (LPCTSTR)bmpIcon1);
		ModifyMenu(hMenu3, 1, MF_BYPOSITION | MF_BITMAP, ID_GRAY, (LPCTSTR)bmpIcon2);
		ModifyMenu(hMenu3, 2, MF_BYPOSITION | MF_BITMAP, ID_RED, (LPCTSTR)bmpIcon3);
		ModifyMenu(hMenu3, 3, MF_BYPOSITION | MF_BITMAP, ID_GREEN, (LPCTSTR)bmpIcon4);
		ModifyMenu(hMenu3, 4, MF_BYPOSITION | MF_BITMAP, ID_BLU, (LPCTSTR)bmpIcon5);
		ModifyMenu(hMenu3, 5, MF_BYPOSITION | MF_BITMAP, ID_CYAN, (LPCTSTR)bmpIcon6);
		ModifyMenu(hMenu3, 6, MF_BYPOSITION | MF_BITMAP, ID_YELLOW, (LPCTSTR)bmpIcon7);
		ModifyMenu(hMenu3, 7, MF_BYPOSITION | MF_BITMAP, ID_Magenta, (LPCTSTR)bmpIcon8);
		ModifyMenu(hMenu5, 0, MF_BYPOSITION | MF_BITMAP, ID_BG_Transparent, (LPCTSTR)bmpIcon9);
		ModifyMenu(hMenu5, 1, MF_BYPOSITION | MF_BITMAP, ID_BG_GRAY, (LPCTSTR)bmpIcon2);
		ModifyMenu(hMenu5, 2, MF_BYPOSITION | MF_BITMAP, ID_BG_RED, (LPCTSTR)bmpIcon3);
		ModifyMenu(hMenu5, 3, MF_BYPOSITION | MF_BITMAP, ID_BG_GREEN, (LPCTSTR)bmpIcon4);
		ModifyMenu(hMenu5, 4, MF_BYPOSITION | MF_BITMAP, ID_BG_BLU, (LPCTSTR)bmpIcon5);
		ModifyMenu(hMenu5, 5, MF_BYPOSITION | MF_BITMAP, ID_BG_CYAN, (LPCTSTR)bmpIcon6);
		ModifyMenu(hMenu5, 6, MF_BYPOSITION | MF_BITMAP, ID_BG_YELLOW, (LPCTSTR)bmpIcon7);
		ModifyMenu(hMenu5, 7, MF_BYPOSITION | MF_BITMAP, ID_BG_Magenta, (LPCTSTR)bmpIcon8);
		ModifyMenu(hMenu4, 0, MF_BYPOSITION | MF_BITMAP, ID_Wide1, (LPCTSTR)line1);
		ModifyMenu(hMenu4, 1, MF_BYPOSITION | MF_BITMAP, ID_Wide2, (LPCTSTR)line2);
		ModifyMenu(hMenu4, 2, MF_BYPOSITION | MF_BITMAP, ID_Wide3, (LPCTSTR)line3);
		ModifyMenu(hMenu4, 3, MF_BYPOSITION | MF_BITMAP, ID_Wide4, (LPCTSTR)line4);
		ModifyMenu(hMenu4, 4, MF_BYPOSITION | MF_BITMAP, ID_Wide5, (LPCTSTR)line5);

		CheckMenuItem(hMenu3, ID_BLACK, MF_CHECKED);
		CheckMenuItem(hMenu4, ID_Wide1, MF_CHECKED);
		CheckMenuItem(hMenu5, ID_BG_Transparent, MF_CHECKED);
		for (int i = 0; i < 8; i++)
		{
			SetMenuItemBitmaps(hMenu3, i, MF_BYPOSITION, NULL, checkedIcon);
			SetMenuItemBitmaps(hMenu5, i, MF_BYPOSITION, NULL, checkedIcon);
			if (i < 5)
				SetMenuItemBitmaps(hMenu4, i, MF_BYPOSITION, NULL, checkedIcon);
		}
	}
	else
		MessageBox(param.hWnd_, 0, TEXT("NO MENU"), MB_OK);

	cursors[0] = LoadCursor(NULL, IDC_ARROW);    // 0 = varsayılan fare
	cursors[1] = LoadCursor(NULL, IDC_SIZENWSE); // 1 = boyut noktası başlangıcı
	cursors[2] = LoadCursor(NULL, IDC_SIZENESW); // 2 = boyut noktası sonu
	cursors[3] = LoadCursor(NULL, IDC_SIZEWE);   // 3 = nesnenin boyutu
	cursors[4] = LoadCursor(NULL, IDC_SIZENS);   // 4 = boyut sonu
	cursors[5] = LoadCursor(NULL, IDC_SIZEALL);  // 5 = tüm boyutlar
	currentCursor = &cursors[0];

	// Ekranın tamamı için normal bir DC ve bir bellek DC oluşturun.
    // Normal DC, ekranın içeriğinin anlık görüntüsünü sağlar.
    // Bellek DC ise ekranın görüntüsünü saklayan bir bit eşlemi (bitmap) sağlar.
    // İlişkilendirilmiş bit eşlemi içindeki anlık görüntü sağlar.

	hdcScreen = CreateDC(L"DISPLAY", (PCTSTR)NULL, (PCTSTR)NULL, (CONST DEVMODE *) NULL);

    //Normal cihaz bağlamı ile ilişkilendirilmiş 
	//bit eşlemi için ölçüler alındı. Bitmap boyutu 1920x1080.
	
	bmp.bmBitsPixel = (BYTE)GetDeviceCaps(hdcScreen, BITSPIXEL);
	bmp.bmPlanes = (BYTE)GetDeviceCaps(hdcScreen, PLANES);
	bmp.bmWidth = 2000; //GetDeviceCaps(hdcScreen, HORZRES);
	bmp.bmHeight = 2000; //GetDeviceCaps(hdcScreen, VERTRES);

						// Genişlik baytla hizalanmalıdır.
	bmp.bmWidthBytes = ((bmp.bmWidth + 15) &~15) / 8;

	// Etiketleri başlatın. 
	fBlt = FALSE;
	fScroll = FALSE;
	fSize = FALSE;

	//Yatay kaydırma değişkenlerini başlatın.
	xMinScroll = 0;
	xCurrentScroll = 0;
	xMaxScroll = 0;

	//Dikey kaydırma değişkenlerini başlatın.
	yMinScroll = 0;
	yCurrentScroll = 0;
	yMaxScroll = 0;

	isMoving = false;
	isResizing = false;

	DeleteDC(hdcScreen);
	return 0;
}

LRESULT WM_MouseMoveEvent(Parameter& param)
{
	mouseX = GET_X_LPARAM(param.lParam_) + xCurrentScroll;
	mouseY = GET_Y_LPARAM(param.lParam_) + yCurrentScroll;

	if (Globals::var().currentDrawMode == DrawLine && currentCursorMode == 0 && mouseHasDown) //çizgiyi çiz
	{
		if (!newLine.endFinished && newLine.startFinished)
		{
			newLine.ptEnd.x = mouseX;
			newLine.ptEnd.y = mouseY;
			if (newLine.ptEnd.x < 0)
				newLine.ptEnd.x = 0;
			else if (newLine.ptEnd.x > 2000)
				newLine.ptEnd.x = 2000;
			if (newLine.ptEnd.y < 0)
				newLine.ptEnd.y = 0;
			else if (newLine.ptEnd.y > 2000)
				newLine.ptEnd.y = 2000;

			AutoScrollObjectWhenDrawing(param.hWnd_, &newLine, xCurrentScroll, yCurrentScroll, rect);
			InvalidateRect(param.hWnd_, NULL, FALSE);
		}
	}
	else if (Globals::var().currentDrawMode == DrawRect && currentCursorMode == 0 && mouseHasDown)
	{
		if (!newRect.endFinished && newRect.startFinished)
		{
			newRect.ptEnd.x = mouseX;
			newRect.ptEnd.y = mouseY;
			if (newRect.ptEnd.x < 0)
				newRect.ptEnd.x = 0;
			else if (newRect.ptEnd.x > 2000)
				newRect.ptEnd.x = 2000;
			if (newRect.ptEnd.y < 0)
				newRect.ptEnd.y = 0;
			else if (newRect.ptEnd.y > 2000)
				newRect.ptEnd.y = 2000;

			AutoScrollObjectWhenDrawing(param.hWnd_, &newRect, xCurrentScroll, yCurrentScroll, rect);
			InvalidateRect(param.hWnd_, NULL, FALSE);
		}
	}
	else if (Globals::var().currentDrawMode == DrawCircle && currentCursorMode == 0 && mouseHasDown)
	{
		if (!newCircle.endFinished && newCircle.startFinished)
		{
			newCircle.ptEnd.x = mouseX;
			newCircle.ptEnd.y = mouseY;
			if (newCircle.ptEnd.x < 0)
				newCircle.ptEnd.x = 0;
			else if (newCircle.ptEnd.x > 2000)
				newCircle.ptEnd.x = 2000;
			if (newCircle.ptEnd.y < 0)
				newCircle.ptEnd.y = 0;
			else if (newCircle.ptEnd.y > 2000)
				newCircle.ptEnd.y = 2000;

			AutoScrollObjectWhenDrawing(param.hWnd_, &newCircle, xCurrentScroll, yCurrentScroll, rect);
			InvalidateRect(param.hWnd_, NULL, FALSE);
		}
	}
	else if (Globals::var().currentDrawMode == DrawTexts)  //text araçları
	{
	        //fare nesnenin üzerinde değilse yalnızca fare simgesini değiştirin.
		if (Globals::var().hasSelected && !mouseHasDown)
		{
			//fare 8 noktanın üzerindeyse çift oklu fare çizin.
			currentCursorMode = Globals::var().selectedObjectPtr->CheckMouseIsOnSizingOpint(mouseX, mouseY);

			//fare nesnenin üzerindeyse hareketli bir ok çizin.
			if (currentCursorMode == 0 && Globals::var().selectedObjectPtr->CheckObjectCollision(mouseX, mouseY))
			{
				currentCursor = &cursors[5];
				currentCursorMode = 9;
			}
			else
				currentCursor = &cursors[(currentCursorMode + 1) / 2];
		}
		else if (Globals::var().hasSelected && mouseHasDown)  //eğer fare nesnenin üzerindeyse, taşıma/yeniden boyutlandırma işlemini gerçekleştirin.
		{
			if (currentCursorMode == 9)  //hareket gerçekleştir.
			{
				if (!isMoving)
				{
					//ptBeg'in eski konumunu sakla
					isMoving = true;
					Globals::var().mlog.OP_moveStart(&Globals::var().newText, -1);  //newText'in hareketini log.h'a kaydet.
				}
				Globals::var().selectedObjectPtr->Moving(mouseX, mouseY);
				AutoScrollObject(param.hWnd_, Globals::var().selectedObjectPtr, xCurrentScroll, yCurrentScroll, rect);
				InvalidateRect(param.hWnd_, NULL, FALSE);
			}
			else if (currentCursorMode > 0 && currentCursorMode < 9)  //yeniden boyutlandırma işlemini gerçekleştir.
			{
				if (!isResizing)  //eski boyut değerlerini log.h'a kaydet
				{
					isResizing = true;
					currentCursor = &cursors[5];
					Globals::var().mlog.OP_sizeStart(&Globals::var().newText, -1);  //newText'in yeniden boyutlandırılmasını log.h'a kaydet.
				}

				if (Globals::var().selectedObjectPtr->objectType == Text)
				{
					TextObj * temp;
					temp = (TextObj*)Globals::var().selectedObjectPtr;
					temp->ResizingText(mouseX, mouseY, currentCursorMode);
					AutoScrollObject(param.hWnd_, Globals::var().selectedObjectPtr, xCurrentScroll, yCurrentScroll, rect);
				}
				else
				{
					Globals::var().selectedObjectPtr->Resizing(mouseX, mouseY, currentCursorMode);
					AutoScrollObject(param.hWnd_, Globals::var().selectedObjectPtr, xCurrentScroll, yCurrentScroll, rect);
				}
				InvalidateRect(param.hWnd_, NULL, FALSE);
			}
		}
	}
	else  //currentDrawMode == 4
	{
		if (!mouseHasDown && Globals::var().currentDrawMode == SelectTool)
		{
			//fare ve DrawObjList'in çarpışması olup olmadığını kontrol edin (kuyruktan)
			Globals::var().preSelectObject = NULL;
			for (auto it = Globals::var().drawObjList.crbegin(); it != Globals::var().drawObjList.crend(); ++it)
			{
				if ((*it)->CheckObjectCollision(mouseX, mouseY))
				{
					Globals::var().preSelectObject = (*it);
					break;
				}
				Globals::var().preSelectObject = NULL;
			}
			InvalidateRect(param.hWnd_, NULL, FALSE);
		}

		    //fare nesnenin üzerinde değilse sadece fare simgesini değiştirin.
		if (Globals::var().hasSelected && !mouseHasDown)
		{
			//fare 8 noktanın üzerindeyse çift oklu fare çiz.
			currentCursorMode = Globals::var().selectedObjectPtr->CheckMouseIsOnSizingOpint(mouseX, mouseY);

			//fare nesnenin üzerindeyse hareketli bir ok çiz.
			if (currentCursorMode == 0 && Globals::var().selectedObjectPtr->CheckObjectCollision(mouseX, mouseY))
			{
				currentCursor = &cursors[5];
				currentCursorMode = 9;
			}
			else
				currentCursor = &cursors[(currentCursorMode + 1) / 2];
		}
		else if (Globals::var().hasSelected && mouseHasDown)  //eğer fare nesnenin üzerindeyse, taşıma/yeniden boyutlandırma işlemini gerçekleştirin
		{
			if (currentCursorMode == 9)  //hareket gerçekleştir
			{
				if (!isMoving)
				{
					//ptBeg'in eski konumunu sakla
					isMoving = true;
					currentCursor = &cursors[5];

					//seçilen nesnenin listedeki birimlerinin bul
					auto it = find(Globals::var().drawObjList.begin(), Globals::var().drawObjList.end(), Globals::var().selectedObjectPtr);
					if (it != Globals::var().drawObjList.end())
					{
						int pos = distance(Globals::var().drawObjList.begin(), it);
						Globals::var().mlog.OP_moveStart(Globals::var().selectedObjectPtr, pos);
					}
				}
				Globals::var().selectedObjectPtr->Moving(mouseX, mouseY);
				AutoScrollObject(param.hWnd_, Globals::var().selectedObjectPtr, xCurrentScroll, yCurrentScroll, rect);
				InvalidateRect(param.hWnd_, NULL, FALSE);
			}
			else if (currentCursorMode > 0 && currentCursorMode < 9)  //yeniden boyutlandırma işlemini gerçekleştir.
			{
				if (!isResizing)  //eski boyut değerlerini log.h'a kaydet.
				{
					isResizing = true;
					auto it = find(Globals::var().drawObjList.begin(), Globals::var().drawObjList.end(), Globals::var().selectedObjectPtr);
					if (it != Globals::var().drawObjList.end())
					{
						int pos = distance(Globals::var().drawObjList.begin(), it);
						Globals::var().mlog.OP_sizeStart(Globals::var().selectedObjectPtr, pos);
					}

				if (Globals::var().selectedObjectPtr->objectType == Text)
				{
					TextObj * temp;
					temp = (TextObj*)Globals::var().selectedObjectPtr;
					temp->ResizingText(mouseX, mouseY, currentCursorMode);
					AutoScrollObjectResize(param.hWnd_, Globals::var().selectedObjectPtr, xCurrentScroll, yCurrentScroll, rect);
				}
				else
				{
					Globals::var().selectedObjectPtr->Resizing(mouseX, mouseY, currentCursorMode);
					AutoScrollObjectResize(param.hWnd_, Globals::var().selectedObjectPtr, xCurrentScroll, yCurrentScroll, rect);
				}
				InvalidateRect(param.hWnd_, NULL, FALSE);
			}
		}
		else
		{	
		}
	}

	SetCursor(*currentCursor);
	return 0;
}

LRESULT WM_LButtonDownEvent(Parameter& param)
{
	if (param.wParam_ && !mouseHasDown)  //eğer fare L aşağıdaysa
	{
		SetCapture(param.hWnd_);  //fareyi pencerenin dışında bile yakala
		mouseX = GET_X_LPARAM(param.lParam_) + xCurrentScroll;
		mouseY = GET_Y_LPARAM(param.lParam_) + yCurrentScroll;
		if (Globals::var().currentDrawMode == DrawLine && currentCursorMode == 0)  //cizmeye basla 
		{
			newLine.MakeStart(mouseX, mouseY, currentColor, currentBgColor, currentLineWidth);
			Globals::var().modifyState = 1;
		}
		else if (Globals::var().currentDrawMode == DrawRect && currentCursorMode == 0)
		{
			newRect.MakeStart(mouseX, mouseY, currentColor, currentBgColor, currentLineWidth);
			Globals::var().modifyState = 1;
		}
		else if (Globals::var().currentDrawMode == DrawCircle && currentCursorMode == 0)
		{
			newCircle.MakeStart(mouseX, mouseY, currentColor, currentBgColor, currentLineWidth);
			Globals::var().modifyState = 1;
		}
		else if (Globals::var().currentDrawMode == DrawTexts && currentCursorMode == 0)  //yeni bir text yap eski tecti haraket ettir
		{
			//eğer fare 8. boyutlandırma noktasındaysa, yeniden boyutlandırmaya başla
			if (currentCursorMode != 0)
			{
				mouseHasDown = true;
				Globals::var().selectedObjectPtr->StartToMove(mouseX, mouseY);
				return 0;
			}

			if (!Globals::var().newText.startFinished) //önceki başlangıcı olmayan yeni bir metin konumuna tıklayın
			{
				Globals::var().newText.MakeStart(mouseX, mouseY, currentColor, currentBgColor, currentLineWidth);
				Globals::var().newText.ptEnd.x = Globals::var().newText.ptBeg.x + 8 * 5 + 1;
				Globals::var().newText.ptEnd.y = Globals::var().newText.ptBeg.y + 1 * 13 + 1;
			}
			else if (Globals::var().newText.startFinished && !Globals::var().newText.endFinished)  //eski metni DrawObj listesine aktar
			{
				PushCurrentNewText(Globals::var().newText);
				Globals::var().newText.MakeStart(mouseX, mouseY, currentColor, currentBgColor, currentLineWidth);
				Globals::var().newText.ptEnd.x = Globals::var().newText.ptBeg.x + 8 * 5 + 1;
				Globals::var().newText.ptEnd.y = Globals::var().newText.ptBeg.y + 1 * 13 + 1;
			}
			if (Globals::var().newText.ptBeg.y > 1982)  //y'nin çok düşük olup olmadığını kontrol eder
			{
				Globals::var().newText.ptBeg.y = 1982;
				Globals::var().newText.ptEnd.y = Globals::var().newText.ptBeg.y + 1 * 13 + 1;
			}
			Globals::var().modifyState = 1;
			Globals::var().selectedObjectPtr = &Globals::var().newText;
			Globals::var().hasSelected = true;
		}
		else  //mode = 4, secenek aracı
		{
			bool b = false;

			//eğer fare 8. boyutlandırma noktasındaysa, yeniden boyutlandırmaya başla
			if (currentCursorMode != 0 && Globals::var().hasSelected)
			{
				mouseHasDown = true;
				Globals::var().selectedObjectPtr->StartToMove(mouseX, mouseY);
				return 0;
			}

			//boyutlandırma veya hareket yoksa çarpışmayı algıla ve bir nesne seç
			//fare ve DrawObjList'in çarpışması olup olmadığını kontrol edin (kuyruktan)	
			mouseHasDown = false;
			for (auto it = Globals::var().drawObjList.crbegin(); it != Globals::var().drawObjList.crend(); ++it)
			{
				if ((*it)->CheckObjectCollision(mouseX, mouseY))
				{
					Globals::var().selectedObjectPtr = (*it);
					Globals::var().selectedObjectPtr->StartToMove(mouseX, mouseY);  //kullanıcı nesneye tıklandığında hareket etmek isteyebilir
					Globals::var().hasSelected = true;
					mouseHasDown = true;
					currentCursorMode = 9;
					b = true;
					break;
				}
			}

			//işlem yoksa her şeyin seçimini kaldırın
			if (!b)
			{
				Globals::var().selectedObjectPtr = nullptr;
				Globals::var().hasSelected = false;
			}

			//draw a selected rectangle			
			InvalidateRect(param.hWnd_, NULL, FALSE);
			return 0;
		}
		InvalidateRect(param.hWnd_, NULL, FALSE);
		mouseHasDown = true;
	}
	return 0;
}

LRESULT WM_LButtonUpEvent(Parameter& param)
{
	if (!param.wParam_)
	{
		ReleaseCapture(); // fareyi yakalamayı durdur
		if (Globals::var().currentDrawMode == DrawLine && currentCursorMode == 0 && mouseHasDown)  //draw line bitir
		{
			newLine.MakeEnd(GET_X_LPARAM(param.lParam_), GET_Y_LPARAM(param.lParam_), xCurrentScroll, yCurrentScroll);

			//if line too short...
			int distancex = (newLine.ptEnd.x - newLine.ptBeg.x) * (newLine.ptEnd.x - newLine.ptBeg.x);
			int distancey = (newLine.ptEnd.y - newLine.ptBeg.y) * (newLine.ptEnd.y - newLine.ptBeg.y);
			int distance = sqrt(abs(distancex - distancey));
			if (distance < 3)
			{
				newLine.Clean();
				mouseHasDown = false;
				InvalidateRect(param.hWnd_, NULL, FALSE);
				return 0;
			}

			Globals::var().drawObjList.push_back(new LineObj(newLine));
			Globals::var().mlog.OP_add(&newLine);
			Globals::var().selectedObjectPtr = Globals::var().drawObjList.back();
			Globals::var().hasSelected = true;

			newLine.Clean();
		}
		else if (Globals::var().currentDrawMode == DrawRect && currentCursorMode == 0 && mouseHasDown)  //dikdörtgen çiz
		{
			newRect.MakeEnd(GET_X_LPARAM(param.lParam_), GET_Y_LPARAM(param.lParam_), xCurrentScroll, yCurrentScroll);

			//eğer rect çok küçükse...
			if (abs(newRect.ptEnd.x - newRect.ptBeg.x) < 4 && abs(newRect.ptEnd.y - newRect.ptBeg.y) < 4)
			{
				newRect.Clean();
				mouseHasDown = false;
				InvalidateRect(param.hWnd_, NULL, FALSE);
				return 0;
			}

			Globals::var().drawObjList.push_back(new RectangularObj(newRect));
			Globals::var().mlog.OP_add(&newRect);
			Globals::var().selectedObjectPtr = Globals::var().drawObjList.back();
			Globals::var().hasSelected = true;

			newRect.Clean();
		}
		else if (Globals::var().currentDrawMode == DrawCircle && currentCursorMode == 0 && mouseHasDown)  //yuvarlak çiz
		{
			newCircle.MakeEnd(GET_X_LPARAM(param.lParam_), GET_Y_LPARAM(param.lParam_), xCurrentScroll, yCurrentScroll);

			//eğer rect çok küçükse...
			if (abs(newCircle.ptEnd.x - newCircle.ptBeg.x) < 4 && abs(newCircle.ptEnd.y - newCircle.ptBeg.y) < 4)
			{
				newCircle.Clean();
				mouseHasDown = false;
				InvalidateRect(param.hWnd_, NULL, FALSE);
				return 0;
			}
			Globals::var().drawObjList.push_back(new CircleObj(newCircle));
			Globals::var().mlog.OP_add(&newCircle);
			Globals::var().selectedObjectPtr = Globals::var().drawObjList.back();
			Globals::var().hasSelected = true;

			newCircle.Clean();
		}
		else if (Globals::var().currentDrawMode == DrawTexts)  //sadece metnin taşınmasını durdur
		{
			currentCursorMode = 0;

			//hareketli bayrağı devre dışı bırak
			if (isMoving)
			{
				isMoving = false;
				//ptBeg'in yeni konumunu kaydet
				Globals::var().mlog.OP_moveEnd(&Globals::var().newText);
			}

			if (isResizing)
			{
				isResizing = false;
				Globals::var().mlog.OP_sizeEnd(&Globals::var().newText);
			}
		}
		else  //hareket etmeyi/yeniden boyutlandırmayı durdur
		{
			//yeniden boyutlandırmayı durdur
			currentCursorMode = 0;

			//hareketli bayrağı devre dışı bırak
			if (isMoving)
			{
				isMoving = false;
				//ptBeg'in yeni konumunu kaydet
				Globals::var().mlog.OP_moveEnd(Globals::var().selectedObjectPtr);
			}

			if (isResizing)
			{
				isResizing = false;
				Globals::var().mlog.OP_sizeEnd(Globals::var().selectedObjectPtr);
			}
		}
		InvalidateRect(param.hWnd_, NULL, FALSE);
		mouseHasDown = false;
	}
	return 0;
}

LRESULT WM_KeyDownEvent(Parameter& param)
{
	if (param.wParam_)
	{
		//izin verilen tuşları filtreleyin: a~z, 0~9, space/arrors/home/end/backspace/enter
		if ((param.wParam_ >= 65 && param.wParam_ <= 90) ||
			(param.wParam_ >= 48 && param.wParam_ <= 57) ||
			(param.wParam_ >= 0x60 && param.wParam_ <= 0x69) ||
			(param.wParam_ == 0x20) ||
			param.wParam_ == 0x0D ||
			param.wParam_ == 0x08 ||
			param.wParam_ == VK_HOME ||
			param.wParam_ == VK_END ||
			(param.wParam_ >= VK_LEFT && param.wParam_ <= VK_DOWN))
		{
			//devam
		}
		else
			return 0;

		bool modified = false;
		if ((Globals::var().currentDrawMode == DrawTexts && Globals::var().newText.startFinished))  //key into newText
		{
			Globals::var().mlog.OP_textStart(&Globals::var().newText, -1); //make a log
			modified = Globals::var().newText.KeyIn(param.wParam_);

			int x, y;  //x ve y penceredeki mevcut düzeltme işareti konumudur
			Globals::var().newText.CalculateCaretPosition();
			x = Globals::var().newText.caretPos.x - xCurrentScroll;
			y = Globals::var().newText.caretPos.y - yCurrentScroll;
			mouseX = Globals::var().newText.caretPos.x;
			mouseY = Globals::var().newText.caretPos.y;
			AutoScroll(param.hWnd_, x, y + 14, xCurrentScroll, yCurrentScroll, rect);
			InvalidateRect(param.hWnd_, NULL, FALSE);

			if (modified)  //günlükte değişiklik yap
				Globals::var().mlog.OP_textEnd(&Globals::var().newText);
		}
		else if (Globals::var().currentDrawMode == SelectTool && Globals::var().selectedObjectPtr != nullptr && Globals::var().selectedObjectPtr->objectType == 4)
		{
			TextObj * temp;
			temp = (TextObj*)Globals::var().selectedObjectPtr;

			auto it = find(Globals::var().drawObjList.begin(), Globals::var().drawObjList.end(), Globals::var().selectedObjectPtr);
			if (it != Globals::var().drawObjList.end())
			{
				int pos = distance(Globals::var().drawObjList.begin(), it);
				Globals::var().mlog.OP_textStart(Globals::var().selectedObjectPtr, pos);  //make a log

				modified = temp->KeyIn(param.wParam_);

				int x, y;  //x ve y penceredeki mevcut düzeltme işareti konumudur
				temp->CalculateCaretPosition();
				x = temp->caretPos.x - xCurrentScroll;
				y = temp->caretPos.y - yCurrentScroll;
				mouseX = temp->caretPos.x;
				mouseY = temp->caretPos.y;
				AutoScroll(param.hWnd_, x, y + 14, xCurrentScroll, yCurrentScroll, rect);
				InvalidateRect(param.hWnd_, NULL, FALSE);
			}

			if (modified)  //make change to log
				Globals::var().mlog.OP_textEnd(Globals::var().selectedObjectPtr);
		}
	}
	return 0;
}

LRESULT WM_PaintEvent(Parameter& param)
{
	// WM_PAINT olayını işlemek için bu fonksiyonu kullanıyoruz.

	PAINTSTRUCT ps; // PAINTSTRUCT yapısı, paint olayının parametrelerini içerir.
	RECT clientRec = rect; // Müşteri alanının koordinatlarını içeren bir dikdörtgen yapısı oluşturuyoruz.
	HDC hdc = BeginPaint(param.hWnd_, &ps); // Ekranda çizim yapmak için bir DC (Device Context) başlatıyoruz.

	// Müşteri alanını alıyoruz.
	GetClientRect(param.hWnd_, &clientRec);

	// Hafıza DC'si oluşturuyoruz ve müşteri alanı boyutunda uyumlu bir bitmap oluşturuyoruz.
	HDC memoryDC = CreateCompatibleDC(hdc);
	hBmp = CreateCompatibleBitmap(hdc, clientRec.right, clientRec.bottom);

	// Bitmap'i hafıza DC'sine seçiyoruz.
	SelectObject(memoryDC, hBmp);

	// Arka planı temizliyoruz.
	HBRUSH hbrBkGnd;
	hbrBkGnd = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	FillRect(memoryDC, &clientRec, hbrBkGnd);
	DeleteObject(hbrBkGnd);

	// Dosya adına * eklenmişse, başlık çubuğunu güncelliyoruz.
	if (Globals::var().modifyState == 1)
	{
		string title = Globals::var().fileName;
		title.append("*");
		SetTitle(title, param.hWnd_);
	}
	else
	{
		SetTitle(Globals::var().fileName, param.hWnd_);
	}

	// DrawObjList'teki her nesneyi çiziyoruz.
	string s2 = "";  // Hata ayıklama amaçlı kullanılan bir string.
	for (auto& it : Globals::var().drawObjList)
	{
		it->Paint(memoryDC, xCurrentScroll, yCurrentScroll);
	}

	// Yeni nesneleri çiziyoruz (varsa).
	newLine.Paint(memoryDC, xCurrentScroll, yCurrentScroll);
	newRect.Paint(memoryDC, xCurrentScroll, yCurrentScroll);
	newCircle.Paint(memoryDC, xCurrentScroll, yCurrentScroll);
	Globals::var().newText.Paint(memoryDC, xCurrentScroll, yCurrentScroll);

	// Fare tıklaması varsa, nesnenin türüne göre çizim yapılır.
	if (mouseHasDown)
	{
		switch (Globals::var().currentDrawMode)
		{
		case 0:
			newLine.PaintMouseOnRect(memoryDC, xCurrentScroll, yCurrentScroll);
			break;
		case 1:
			newRect.PaintMouseOnRect(memoryDC, xCurrentScroll, yCurrentScroll);
			break;
		case 2:
			newCircle.PaintMouseOnRect(memoryDC, xCurrentScroll, yCurrentScroll);
			break;
		default:
			break;
		}
	}

	// Önceden seçilmiş bir nesne varsa, onun etrafına bir dikdörtgen çizilir.
	if (Globals::var().preSelectObject != nullptr)
	{
		Globals::var().preSelectObject->PaintMouseOnRect(memoryDC, xCurrentScroll, yCurrentScroll);
		if (Globals::var().debugMode)
		{
			s2 = "Önceden seçilmiş nesneye çizim yapılıyor.";
			TextOutA(memoryDC, 700 - xCurrentScroll, 600 - yCurrentScroll, s2.c_str(), s2.length());
		}
	}

	// Seçilmiş bir nesne varsa, etrafına bir dikdörtgen çizilir.
	if (Globals::var().hasSelected)
		Globals::var().selectedObjectPtr->PaintSelectedRect(memoryDC, xCurrentScroll, yCurrentScroll);

	// Metin çizim modunda caret oluşturulur ve konumlandırılır.
	if (Globals::var().currentDrawMode == DrawTexts && Globals::var().newText.startFinished && !Globals::var().newText.endFinished)
	{
		CreateCaret(param.hWnd_, (HBITMAP)NULL, 3, 14);
		SetCaretPos(Globals::var().newText.caretPos.x - xCurrentScroll, Globals::var().newText.caretPos.y - yCurrentScroll);
		if (Globals::var().debugMode)
		{
			s2 = "caretPos=" + to_string(Globals::var().newText.caretPos.x) + ", " + to_string(Globals::var().newText.caretPos.y);
			TextOutA(memoryDC, 500 - xCurrentScroll, 500 - yCurrentScroll, s2.c_str(), s2.length());
		}
	}
	// Seçilmiş bir metin nesnesi varsa, caret konumlandırılır.
	else if (Globals::var().hasSelected && Globals::var().currentDrawMode == SelectTool && Globals::var().selectedObjectPtr->objectType == Text)
	{
		TextObj* t = (TextObj*)Globals::var().selectedObjectPtr;
		CreateCaret(param.hWnd_, (HBITMAP)NULL, 3, 14);
		SetCaretPos(t->caretPos.x - xCurrentScroll, t->caretPos.y - yCurrentScroll);
	}
	else
		DestroyCaret();

	// Hata ayıklama amaçlı bilgileri görüntüler.
	if (Globals::var().debugMode)
	{
		s2 = "xCurrentScroll=" + to_string(xCurrentScroll) + " yCurrentScroll=" + to_string(yCurrentScroll);
		TextOutA(memoryDC, 700 - xCurrentScroll, 640 - yCurrentScroll, s2.c_str(), s2.length());
		s2 = "mousex=" + to_string(mouseX) + " mousey=" + to_string(mouseY);
		TextOutA(memoryDC, 700 - xCurrentScroll, 620 - yCurrentScroll, s2.c_str(), s2.length());
		s2 = "currentDrawMode = " + to_string(Globals::var().currentDrawMode);
		TextOutA(memoryDC, 700 - xCurrentScroll, 660 - yCurrentScroll, s2.c_str(), s2.length());
		s2 = "nesne sayısı: " + to_string(Globals::var().drawObjList.size());
		TextOutA(memoryDC, 700 - xCurrentScroll, 680 - yCurrentScroll, s2.c_str(), s2.length());
	}

	// Değişiklikleri ekrana uygular.
	BitBlt(hdc,
		clientRec.left, clientRec.top,
		clientRec.right - clientRec.left, clientRec.bottom - clientRec.top,
		memoryDC,
		0, 0,
		SRCCOPY);

	DeleteDC(memoryDC);  // Bellek DC'sini serbest bırakır.
	DeleteObject(hBmp); // Bitmap nesnesini serbest bırakır.
	EndPaint(param.hWnd_, &ps); // Boyamayı sonlandırır.

	return 0;
}


LRESULT WM_SizeEvent(Parameter& param)
{
	// WM_SIZE olayını işlemek için bu fonksiyonu kullanıyoruz.

	// Müşteri alanını güncelliyoruz.
	rect.left = 0;
	rect.top = 0;
	rect.right = LOWORD(param.lParam_);
	rect.bottom = HIWORD(param.lParam_);

	// Yeni boyutları alıyoruz.
	int xNewSize;
	int yNewSize;
	xNewSize = LOWORD(param.lParam_);
	yNewSize = HIWORD(param.lParam_);

	// Eğer BitBlt kullanılıyorsa, boyut değişikliğini işlemek için fSize değişkenini TRUE yaparız.
	if (fBlt)
		fSize = TRUE;

	// Yatay kaydırma aralığını belirliyoruz.
	xMaxScroll = max(bmp.bmWidth - xNewSize, 0);
	xCurrentScroll = min(xCurrentScroll, xMaxScroll);
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = xMinScroll;
	si.nMax = bmp.bmWidth;
	si.nPage = xNewSize;
	si.nPos = xCurrentScroll;
	SetScrollInfo(param.hWnd_, SB_HORZ, &si, TRUE);

	// Dikey kaydırma aralığını belirliyoruz.
	if (hScrollResize < 5)
		hScrollResize++;
	if (hScrollResize != 2) // İkinci kez görmezden gel. Hâlâ neden olduğunu bilmiyoruz.
	{
		yMaxScroll = max(bmp.bmHeight - yNewSize, 0);
	}
	yCurrentScroll = min(yCurrentScroll, yMaxScroll);
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = yMinScroll;
	si.nMax = bmp.bmHeight;
	si.nPage = yNewSize;
	si.nPos = yCurrentScroll;
	SetScrollInfo(param.hWnd_, SB_VERT, &si, TRUE);

	// Eğer bir alt pencere varsa, onun boyutunu güncelliyoruz.
	if (Globals::var().myChildWindow)
		SetWindowPos(Globals::var().myChildWindow, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	// Pencereyi geçersiz kılıyoruz, böylece boyut değişiklikleri ekranda görüntülenir.
	InvalidateRect(param.hWnd_, NULL, false);

	return 0;
}


LRESULT WM_HScrollEvent(Parameter& param)
{
	int xDelta;     // xDelta = new_pos - current_pos  
	int xNewPos;    // Yeni konum 
	int yDelta = 0; // Dikey kaydırma değişkeni, yatay kaydırmada kullanılmaz.

	// WM_HSCROLL olayını işlemek için bu fonksiyonu kullanıyoruz.

	switch (LOWORD(param.wParam_))
	{
		// Kullanıcı, kaydırma çubuğu çubuğun sol tarafında tıkladı. 
	case SB_PAGEUP:
		xNewPos = xCurrentScroll - 50;
		break;

		// Kullanıcı, kaydırma çubuğu çubuğun sağ tarafında tıkladı. 
	case SB_PAGEDOWN:
		xNewPos = xCurrentScroll + 50;
		break;

		// Kullanıcı, sola tıkladı. 
	case SB_LINEUP:
		xNewPos = xCurrentScroll - 5;
		break;

		// Kullanıcı, sağa tıkladı. 
	case SB_LINEDOWN:
		xNewPos = xCurrentScroll + 5;
		break;

		// Kullanıcı, kaydırma kutucuğunu sürükledi. 
	case SB_THUMBTRACK:
		xNewPos = HIWORD(param.wParam_);
		break;

	default:
		xNewPos = xCurrentScroll;
	}

	// Yeni pozisyon 0 ile ekran genişliği arasında olmalıdır. 
	xNewPos = max(0, xNewPos);
	xNewPos = min(xMaxScroll, xNewPos);

	// Eğer mevcut konum değişmezse, kaydırma yapmayız.
	if (xNewPos == xCurrentScroll)
		return 0;

	// Kaydırma işleminin gerçekleştirildiğini belirten bayrağı TRUE yaparız. 
	fScroll = TRUE;

	// Kaydırmanın miktarını (piksel cinsinden) belirleriz.
	xDelta = xNewPos - xCurrentScroll;

	// Mevcut kaydırma konumunu sıfırlarız. 
	xCurrentScroll = xNewPos;

	// Pencereyi kaydırırız. (ScrollWindowEx çağrıldığında sistem çoğu 
	// istemci alanını tekrar çizer; ancak, geçersiz kılınan piksellerin 
	// dikdörtgenini tekrar çizmek için UpdateWindow'ı çağırmak gerekir.) 
	if (!Globals::var().autoScrolling)
	{
		ScrollWindowEx(param.hWnd_, -xDelta, -yDelta, (CONST RECT*) NULL,
			(CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL,
			SW_INVALIDATE);
		UpdateWindow(param.hWnd_);
	}

	// Kaydırma çubuğunu sıfırlarız. 
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	si.nPos = xCurrentScroll;
	SetScrollInfo(param.hWnd_, SB_HORZ, &si, TRUE);

	return 0;
}


LRESULT WM_VScrollEvent(Parameter& param)
{
	int xDelta = 0;
	int yDelta;     // yDelta = new_pos - current_pos 
	int yNewPos;    // new position 

	switch (LOWORD(param.wParam_))
	{
	
	case SB_PAGEUP:
		yNewPos = yCurrentScroll - 50;
		break;

		
	case SB_PAGEDOWN:
		yNewPos = yCurrentScroll + 50;
		break;

		// Kullanıcı alt oka tıkladı.
	case SB_LINEUP:
		yNewPos = yCurrentScroll - 5;
		break;

		// Kullanıcı alt oka tıkladı.
	case SB_LINEDOWN:
		yNewPos = yCurrentScroll + 5;
		break;

		// Kullanıcı kaydırma kutusunu sürükledi.
	case SB_THUMBTRACK:
		yNewPos = HIWORD(param.wParam_);
		if (yNewPos == 1004)
			yNewPos = yMaxScroll;  //Y'nin aşağıya doğru kaydırması için geçici çözüm
		break;

	default:
		yNewPos = yCurrentScroll;
	}

	//if (yNewPos > yCurrentScroll)
	//	yNewPos = yCurrentScroll + 10;
	//else if (yNewPos < yCurrentScroll)
	//	yNewPos = yCurrentScroll - 10;

	// Yeni konum 0 ile ekran yüksekliği arasında olmalıdır. 
	yNewPos = max(0, yNewPos);
	yNewPos = min(yMaxScroll, yNewPos);

	// Geçerli konum değişmezse kaydırma yapmayın.
	if (yNewPos == yCurrentScroll)
		return 0;

	// Kaydırma bayrağını TRUE olarak ayarlayın.
	fScroll = TRUE;

	// Kaydırılan miktarı (piksel cinsinden) belirleyin. 
	yDelta = yNewPos - yCurrentScroll;

	// Geçerli kaydırma konumunu sıfırlayın.
	yCurrentScroll = yNewPos;

	// Pencereyi kaydır. (Sistem çoğu şeyi yeniden boyar.)
	// ScrollWindowEx çağrıldığında istemci alanı; ancak öyle
	// yeniden boyamak için UpdateWindow'u çağırmak gerekiyor
	// geçersiz kılınan piksellerin dikdörtgeni.)
	if (!Globals::var().autoScrolling)
	{
		ScrollWindowEx(param.hWnd_, -xDelta, -yDelta, (CONST RECT *) NULL,  
			NULL, (HRGN)NULL, (PRECT)NULL,     // ve tüm alanı yeniden çizin.
			SW_INVALIDATE);
		UpdateWindow(param.hWnd_);
	}
	// Kaydırma çubuğunu sıfırlıyoruz.
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	si.nPos = yCurrentScroll;
	SetScrollInfo(param.hWnd_, SB_VERT, &si, TRUE);
	return 0;
}

LRESULT WM_CloseEvent(Parameter& param)
{
	if (Globals::var().modifyState == 1)  //dosya kaydetme iletişim kutusunu göster
	{
		if (DisplayConfirmNewFileMessageBox(Globals::var().fileName) == IDYES)
		{
			PushCurrentNewText(Globals::var().newText);
			if (Globals::var().lastFilePath.size() < 1)
			{
				SaveToFile(Globals::var().drawObjList, Globals::var().fileName);
				SetTitle(Globals::var().fileName, param.hWnd_);
				Globals::var().modifyState = 2;
			}
			else
			{
				SaveToLastFilePath(Globals::var().drawObjList);
				Globals::var().modifyState = 2;
			}
		}
	}
	DestroyWindow(param.hWnd_);
	return 0;
}

LRESULT WM_DestroyEvent(Parameter& param)
{
	PostQuitMessage(0);
	return 0;
}

LRESULT WM_SetCursorEvent(Parameter& param)
{
	return 0;  //fare simgesinin geri dönmesini önlemek için bu mesajı işleyin
}

LRESULT WM_GetMinMaxInfo(Parameter & param)  //pencerenin minimum boyutunu ayarladık
{
	LPMINMAXINFO lpMMI = (LPMINMAXINFO)param.lParam_;
	lpMMI->ptMinTrackSize.x = 77;
	lpMMI->ptMinTrackSize.y = 415;
	return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

POINT MovePastedObj()  //yapıştırılan nesneyi 30 birim hareket ettir
{
	int xdir, ydir;
	if (xPasteDir == 0)
	{
		xdir = -30;
	}
	else
	{
		xdir = 30;
	}

	if (yPasteDir == 0)
	{
		ydir = -30;
	}
	else
	{
		ydir = 30;
	}

	POINT newBeg = Globals::var().pastebinObjectPtr->ptBeg;
	POINT newEnd = Globals::var().pastebinObjectPtr->ptEnd;
	newBeg.x += xdir;
	newBeg.y += ydir;
	newEnd.x += xdir;
	newEnd.y += ydir;

	if (newBeg.x < 0)   //sınırlara çarpıldığında yönü değiştir
	{
		int delta = 0 - newBeg.x;
		newBeg.x = 0;
		newEnd.x += delta;
		xPasteDir = 1;
	}
	if (newBeg.y < 0)
	{
		int delta = 0 - newBeg.y;
		newBeg.y = 0;
		newEnd.y += delta;
		yPasteDir = 1;
	}

	if (newBeg.x > 1990)
	{
		int delta = 1990 - newBeg.x;
		newBeg.x = 1990;
		newEnd.x += delta;
		xPasteDir = 0;
	}
	if (newBeg.y > 1989)
	{
		int delta = 1990 - newBeg.y;
		newBeg.y = 1990;
		newEnd.y += delta;
		yPasteDir = 0;
	}
	//newEnd ile ilgilenmek
	if (newEnd.x < 0)
	{
		int delta = 0 - newEnd.x;
		newEnd.x = 0;
		newBeg.x += delta;
		xPasteDir = 1;
	}
	if (newEnd.y < 0)
	{
		int delta = 0 - newEnd.y;
		newEnd.y = 0;
		newBeg.y += delta;
		yPasteDir = 1;
	}

	if (newEnd.x > 1990)
	{
		int delta = 1990 - newEnd.x;
		newEnd.x = 1990;
		newBeg.x += delta;
		xPasteDir = 0;
	}
	if (newEnd.y > 1989)
	{
		int delta = 1990 - newEnd.y;
		newEnd.y = 1990;
		newBeg.y += delta;
		yPasteDir = 0;
	}

	Globals::var().pastebinObjectPtr->ptBeg = newBeg;
	Globals::var().pastebinObjectPtr->ptEnd = newEnd;

	int xfocus, yfocus;
	if (xPasteDir == 0)  //yapıştırma yönlerine göre odak noktasını al
		xfocus = newBeg.x < newEnd.x ? newBeg.x : newEnd.x;
	else
		xfocus = newBeg.x > newEnd.x ? newBeg.x : newEnd.x;

	if (yPasteDir == 0)
		yfocus = newBeg.y < newEnd.y ? newBeg.y : newEnd.y;
	else
		yfocus = newBeg.y > newEnd.y ? newBeg.y : newEnd.y;

	POINT r;
	r.x = xfocus;
	r.y = yfocus;
	return r;
}

void PushCurrentNewText(TextObj& T)
{
	if (T.text.size() > 0)
	{
		T.endFinished = true;
		Globals::var().drawObjList.push_back(new TextObj(T));
		Globals::var().mlog.OP_add(&T);
	}
	T.Clean();
	Globals::var().selectedObjectPtr = nullptr;
	Globals::var().hasSelected = false;
}

void Undo()
{
	HMENU hMenu2 = GetSubMenu(hMenu, 1);
	EnableMenuItem(hMenu2, ID_Redo, MF_BYCOMMAND | MF_ENABLED);  //geri al/yinele düğmelerini değiştir
	EnableMenuItem(hMenu2, ID_Undo, MF_BYCOMMAND | MF_GRAYED);
	Globals::var().mlog.Undo();
}

void Redo()
{
	HMENU hMenu2 = GetSubMenu(hMenu, 1);
	EnableMenuItem(hMenu2, ID_Redo, MF_BYCOMMAND | MF_GRAYED);  //geri al/yinele düğmelerini değiştir
	EnableMenuItem(hMenu2, ID_Undo, MF_BYCOMMAND | MF_ENABLED);
	Globals::var().mlog.Redo();
}

//geri alma/yineleme işlemi için newText'i güncelle
void UpdateNewText(vector<string> vs, POINT in)
{
	Globals::var().newText.text = vs;
	Globals::var().newText.inputPos = in;
	Globals::var().newText.CalculateCaretPosition();
}

void ToggleUndoButton()
{
	HMENU hMenu2 = GetSubMenu(hMenu, 1);
	EnableMenuItem(hMenu2, ID_Redo, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMenu2, ID_Undo, MF_BYCOMMAND | MF_ENABLED);
	Globals::var().modifyState = 1;
}

//p'nin merkezi ekranın dışındaysa ekranı kaydır
void FocusSelectedObject(DrawObj* p, HWND hwnd)
{
	if (!Globals::var().hasSelected || p == nullptr)
		return;

	//ortanın ekranın dışında olup olmadığını kontrol et
	int xcenter = (p->ptBeg.x + p->ptEnd.x) / 2 - xCurrentScroll;
	int ycenter = (p->ptBeg.y + p->ptEnd.y) / 2 - yCurrentScroll;
	AutoScroll(hwnd, xcenter, ycenter, xCurrentScroll, yCurrentScroll, rect);
}
