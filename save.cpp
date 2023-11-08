#include "stdafx.h"
#include "Save.h"
#include "Globals.h"

int SaveToFile(const list<DrawObj*>& saveList, string& fileName)
{
	// JSON nesnesi oluşturulur.
	json j;
	// Her nesneyi JSON formatına dönüştürerek j nesnesine ekler.
	for (auto& it : saveList)
	{
		json jit;
		jit["objectType"] = it->objectType;
		jit["ptBeg"] = { it->ptBeg.x, it->ptBeg.y };
		jit["ptEnd"] = { it->ptEnd.x, it->ptEnd.y };
		jit["color"] = it->color;
		jit["backgroundColor"] = it->backgroundColor;
		jit["lineWidth"] = it->lineWidth;
		// Eğer nesne bir metin nesnesi ise, metin verilerini JSON'a ekler.
		if (it->objectType == 4)
		{
			TextObj* t = dynamic_cast<TextObj*>(it);
			vector<string> vs = t->text;
			jit["text"] = vs;
		}
		j.push_back(jit);
	}

	// Dosya seçim penceresi açılır ve kaydedilecek dosyanın yolu alınır.
	ofstream outfile;
	PWSTR filePath = BasicFileSave();
	if (filePath)
	{
		string fp = WstrToStr(filePath);
		// Dosya uzantısını kontrol eder ve ".json" ekler.
		if (!EndWith(fp, ".json"))
			fp.append(".json");
		// Dosyayı açar ve JSON verilerini dosyaya yazar.
		outfile.open(fp);
		outfile << j << endl;
		outfile.close();
		// Son kaydedilen dosya yolu saklanır.
		Globals::var().lastFilePath = filePath;
	}
	else
	{
		// Dosya seçilmediyse uyarı verilir ve fonksiyon 1 döndürür.
		MessageBox(NULL, L"You did not save.", L"Warning!", MB_OK);
		return 1;
	}

	// Dosya adını döndürür.
	string name = WstrToStr(filePath);
	auto const pos = name.find_last_of('\\');
	auto const result = name.substr(pos + 1);
	fileName = result;

	// Fonksiyon başarıyla tamamlandığı için 0 döndürülür.
	return 0;
}


int ReadFromFile(list<DrawObj*>& readList, string& fileName)
{
	// Silinen işaretçiyi geçici olarak NULL olarak ayarlar (pointer workaround).
	Globals::var().preSelectObject = nullptr;
	// Dosya açma penceresi açılır ve dosya yolunu alır.
	PWSTR filePath = BasicFileOpen();
	// Dosya yolunun alınmadığı durumda hata mesajı gösterir ve fonksiyon 1 döndürür.
	if (!filePath)
	{
		MessageBox(NULL, L"File open failed.", L"Error!", MB_OK);
		return 1;
	}


	json j;

	// Dosyayı okuma modunda açar.
	ifstream infile;
	infile.open(filePath);
	// Dosya açılamazsa hata mesajı gösterir ve fonksiyon 1 döndürülerek işlem sonlandırılır.
	if (!infile)
	{
		MessageBox(NULL, L"File open failed.", L"Error!", MB_OK);
		return 1;
	}
	// JSON veriyi dosyadan okur.
	infile >> j;
	// Dosyayı kapatır.
	infile.close();

	// Dosya yolunu ana programa geçirir.
	Globals::var().lastFilePath = filePath;
	// Dosya ismini alır ve sadece dosya adını saklar.
	string name = WstrToStr(filePath);
	auto const pos = name.find_last_of('\\');
	auto const result = name.substr(pos + 1);
	// Ana programa dosya adını geçirir.
	fileName = result;

	// Okunan nesneleri temizler.
	readList.clear();


	int i = 0;
	for (json::iterator it = j.begin(); it != j.end(); it++)
	{
		// JSON'dan bir nesneyi alır.
		json j5 = *it;
		// JSON nesnesindeki "objectType" değerine göre farklı nesneler oluşturur ve listeye ekler.
		switch ((int)j5["objectType"])
		{
		case Line:
		{
			// LineObj türünde yeni bir çizgi nesnesi oluşturur.
			LineObj newLine;
			// Çizgi nesnesinin başlangıç ve bitiş noktalarını JSON veriden alarak ayarlar.
			newLine.MakeStart(j5["ptBeg"][0], j5["ptBeg"][1], j5["color"], j5["backgroundColor"], j5["lineWidth"]);
			newLine.MakeEnd(j5["ptEnd"][0], j5["ptEnd"][1], 0, 0);
			// Oluşturulan çizgi nesnesini readList'e ekler.
			readList.push_back(new LineObj(newLine));
			break;
		}
		case Rect:
		{
			// RectangularObj türünde yeni bir dikdörtgen nesnesi oluşturur.
			RectangularObj newRect;
			// Dikdörtgen nesnesinin başlangıç ve bitiş noktalarını JSON veriden alarak ayarlar.
			newRect.MakeStart(j5["ptBeg"][0], j5["ptBeg"][1], j5["color"], j5["backgroundColor"], j5["lineWidth"]);
			newRect.MakeEnd(j5["ptEnd"][0], j5["ptEnd"][1], 0, 0);
			// Oluşturulan dikdörtgen nesnesini readList'e ekler.
			readList.push_back(new RectangularObj(newRect));
			break;
		}
		case Circle:
		{
			// CircleObj türünde yeni bir daire nesnesi oluşturur.
			CircleObj newCircle;
			// Daire nesnesinin başlangıç ve bitiş noktalarını JSON veriden alarak ayarlar.
			newCircle.MakeStart(j5["ptBeg"][0], j5["ptBeg"][1], j5["color"], j5["backgroundColor"], j5["lineWidth"]);
			newCircle.MakeEnd(j5["ptEnd"][0], j5["ptEnd"][1], 0, 0);
			// Oluşturulan daire nesnesini readList'e ekler.
			readList.push_back(new CircleObj(newCircle));
			break;
		}
		case Text:
		{
			// TextObj türünde yeni bir metin nesnesi oluşturur.
			TextObj newText;
			// Metin nesnesinin başlangıç ve bitiş noktalarını JSON veriden alarak ayarlar.
			newText.MakeStart(j5["ptBeg"][0], j5["ptBeg"][1], j5["color"], j5["backgroundColor"], j5["lineWidth"]);
			newText.MakeEnd(j5["ptEnd"][0], j5["ptEnd"][1], 0, 0);
			// Metin içeriğini JSON veriden alır.
			vector<string> text = j5["text"];
			newText.text = text;
			// Metin kutusunun genişliğini hesaplar.
			{
				int textHeight = newText.ptEnd.y - newText.ptBeg.y;
				int maxLine = (newText.ptEnd.y - newText.ptBeg.y) / 13;
				int lineSize = (newText.ptEnd.x - newText.ptBeg.x) / 8;
				if (lineSize <= 0)
					lineSize = 1;
				POINT t;
				t.x = 0;
				t.y = 0;

				// Metin içeriğini tek bir metin haline getirir ve boyutları hesaplar.
				for (auto it : text)
				{
					int i = 0;
					t.x = 0;
					for (auto it2 : it)
					{
						if (i > 0 && i % lineSize == 0)
						{
							t.y += 1;
							t.x = 0;
						}
						i++;
						t.x += 1;
					}
					t.y += 1;
				}
				// Metin yüksekliği yeterli değilse genişletir.
				if (t.y > maxLine)
				{
					textHeight = t.y * 13;
					newText.ptEnd.y = newText.ptBeg.y + textHeight;
				}
			}
			// Oluşturulan metin nesnesini readList'e ekler.
			readList.push_back(new TextObj(newText));
			break;
		}
		default:
			// Hata durumunda mesaj gösterir.
			MessageBox(NULL, L"JSON read error!", L"ERROR", MB_OK);
			break;
		}
		i++;
	}
	// JSON veriyi temizler.
	j.clear();
	// Fonksiyon başarılı bir şekilde tamamlanır.
	return 0;
}


int SaveToLastFilePath(const list<DrawObj*>& saveList)
{
	// JSON veri oluşturma işlemi
	json j;
	for (auto& it : saveList)  // DrawObjList içindeki her nesneyi gez
	{
		json jit;
		jit["objectType"] = it->objectType;  // Nesnenin türünü JSON'a ekle
		jit["ptBeg"] = { it->ptBeg.x, it->ptBeg.y };  // Başlangıç noktasını JSON'a ekle
		jit["ptEnd"] = { it->ptEnd.x, it->ptEnd.y };  // Bitiş noktasını JSON'a ekle
		jit["color"] = it->color;  // Rengi JSON'a ekle
		jit["backgroundColor"] = it->backgroundColor;  // Arka plan rengini JSON'a ekle
		jit["lineWidth"] = it->lineWidth;  // Çizgi genişliğini JSON'a ekle
		if (it->objectType == 4)
		{
			// Eğer nesne bir metin nesnesiyse, metin içeriğini JSON'a ekle
			TextObj* t = dynamic_cast<TextObj*>(it);
			vector<string> ls = t->text;
			jit["text"] = ls;
		}
		j.push_back(jit);  // Oluşturulan JSON nesnesini ana JSON nesnesine ekle
	}

	// Dosyayı yazma modunda aç
	ofstream outfile;
	if (Globals::var().lastFilePath.size() > 0)
	{
		// Son kaydedilen dosya yolunu al
		string fp = WstrToStr(Globals::var().lastFilePath);
		// JSON verisini dosyaya yaz
		outfile.open(fp);
		if (outfile.is_open())
		{
			outfile << j << endl;
			outfile.close();
		}
		else
		{
			// Dosya açma hatası durumunda uyarı göster
			MessageBox(NULL, L"SaveToLastFilePath Failed", L"ERROR!", MB_OK);
		}
	}
	else
	{
		// Son kaydedilen dosya yoksa uyarı göster ve hata kodunu döndür
		MessageBox(NULL, L"SaveToLastFilePath Failed", L"ERROR!", MB_OK);
		return 1;
	}
	j.clear();  // JSON veriyi temizle
	return 0;  // Başarılı işlem durumunda 0 döndür
}


PWSTR BasicFileOpen()
{
	PWSTR pszFilePath = NULL;  // Dosya yolu için PWSTR türünde bir pointer oluştur

	// COM alt sistemi başlatılır
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen;  // Dosya açma dialog penceresi için bir pointer oluştur

		// FileOpenDialog nesnesi oluşturulur
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
		// CoCreateInstance, bellek tahsis etmez, yalnızca bir COM nesnesi oluşturur.

		if (SUCCEEDED(hr))
		{
			// Dosya açma penceresini göster
			hr = pFileOpen->Show(NULL);

			// Dosya adını al
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					// Dosya yolunu al
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Kullanıcıya dosya adını göster
					if (SUCCEEDED(hr))
					{
						// Burada dosya adını kullanabilir veya işlemler yapabilirsiniz
						// Örneğin, dosya adını bir metin kutusuna yazdırabilirsiniz
					}
					pItem->Release();  // Belleği serbest bırak
				}
			}
			pFileOpen->Release();  // Belleği serbest bırak
		}
		CoUninitialize();  // COM alt sistemi kapatılır
	}

	return pszFilePath;  // Seçilen dosya yolunu döndür
}


PWSTR BasicFileSave()
{
	PWSTR pszFilePath = NULL;  // Dosya yolu için PWSTR türünde bir pointer oluştur

	// COM alt sistemi başlatılır
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileSaveDialog* pFileSave;  // Dosya kaydetme dialog penceresi için bir pointer oluştur

		// FileSaveDialog nesnesi oluşturulur
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
			IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

		if (SUCCEEDED(hr))
		{
			// Dosya kaydetme penceresine seçenekleri ayarla
			DWORD dwFlags;

			// Her zaman önce seçenekleri alarak mevcut seçenekleri üzerine yazmamak için
			hr = pFileSave->GetOptions(&dwFlags);

			if (SUCCEEDED(hr))
			{
				// Bu durumda, yalnızca dosya sistemine ait öğeler için shell öğelerini al
				hr = pFileSave->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);

				// Gösterilecek dosya türlerini ayarla
				// Dikkat edilmesi gereken şey, bu dizinin 1 tabanlı bir dizi olmasıdır.
				COMDLG_FILTERSPEC rgSpec[] =
				{
					{ L"JSON Files (*.json)", L"*.json" },
				};
				hr = pFileSave->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

				// Varsayılan uzantıyı ayarla, örneğin ".doc" dosyası
				//hr = pFileSave->SetDefaultExtension(L"txt");

				// Dosya açma penceresini göster
				hr = pFileSave->Show(NULL);

				// Dosya adını al
				if (SUCCEEDED(hr))
				{
					IShellItem* pItem;
					hr = pFileSave->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						// Dosya yolunu al
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

						// Kullanıcıya dosya adını göster
						if (SUCCEEDED(hr))
						{
							// Burada dosya adını kullanabilir veya işlemler yapabilirsiniz
							// Örneğin, dosya adını bir metin kutusuna yazdırabilirsiniz
						}
						pItem->Release();  // Belleği serbest bırak
					}
				}
				pFileSave->Release();  // Belleği serbest bırak
			}
		}
		CoUninitialize();  // COM alt sistemi kapatılır
	}

	return pszFilePath;  // Seçilen dosya yolunu döndür
}


std::string WstrToStr(const std::wstring& wstr)
{
	std::string strTo;  // Sonuç olarak döndürülecek UTF-8 dizisi

	// Geniş karakter dizisini tutacak belleği oluştur
	char* szTo = new char[wstr.length() + 1];

	// Geniş karakter dizisinin sonuna NULL karakter ekle
	szTo[wstr.size()] = '\0';

	// Geniş karakter dizisini çok baytlı diziye dönüştür
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);

	// Dönüştürülen çok baytlı diziyi std::string'e atayın
	strTo = szTo;

	// Belleği temizle
	delete[] szTo;

	// Sonuç olarak döndür
	return strTo;
}


bool EndWith(const std::string& str, const std::string& suffix)
{
	// str dizesi, suffix dizesi ile bitiyor mu kontrol et
	return str.size() >= suffix.size() &&
		str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}


std::wstring StrToWstr(const std::string& s)
{
	// std::wstring'e dönüştürme yap
	std::wstring stemp = std::wstring(s.begin(), s.end());
	return stemp;
}


int DisplayConfirmClearMessageBox(const std::string fileName)
{
	// std::string'i std::wstring'e dönüştür
	std::wstring ws = StrToWstr(fileName);

	// Uyarı mesajı penceresini göster ve kullanıcının seçimini döndür
	int msgboxID = MessageBox(
		NULL,
		ws.c_str(),
		L"Confirm to Clear",
		MB_ICONEXCLAMATION | MB_YESNO
	);

	return msgboxID;
}

int DisplayConfirmNewFileMessageBox(const std::string fileName)
{
	// std::string'i std::wstring'e dönüştür
	std::wstring ws = StrToWstr(fileName);

	// Uyarı mesajı penceresini göster ve kullanıcının seçimini döndür
	int msgboxID = MessageBox(
		NULL,
		ws.c_str(),
		L"Confirm Save",
		MB_ICONEXCLAMATION | MB_YESNO
	);

	return msgboxID;
}
