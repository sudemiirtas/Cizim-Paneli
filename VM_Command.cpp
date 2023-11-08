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
