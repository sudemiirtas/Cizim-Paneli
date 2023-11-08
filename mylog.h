#pragma once
/****************************** Modül Başlığı ******************************\
* Modül Adı: DrawObj.h
* Proje: Paint_Programing
* Sürüm: 1.0
*
* Geçmiş:
* * 3/12/2016 Tran Trung Tinh Tarafından Oluşturuldu
\***************************************************************************/

#include "stdafx.h"  // Önişlemci komut dosyası içe aktar
#include "json.hpp"  // json veri yapısını kullanmak için nlohmann/json kütüphanesini içe aktar
#include <string>  // C++ dizesi kullanmak için <string> başlık dosyasını içe aktar
#include <list>  // Çift bağlı liste veri yapısını kullanmak için <list> başlık dosyasını içe aktar
#include "DrawObj.h"  // Özel çizim nesnelerini içe aktar

using namespace std;  // std ad alanını kullan
using json = nlohmann::json;  // json veri yapısını json olarak kullan

// İşlem türleri:
// 0=add, 1=delete, 2=move, 3=resize, 4=modify, 5=modify text
class mylog
{
	list<json> ops;  // json işlem listesini tanımla
	void PushObject(DrawObj* d, json j);  // çizim nesnesini json işlem listesine eklemek için yardımcı işlevi tanımla
	json jmove;  // taşıma işlemi için kullanılan json nesnesi
	json jredo;  // geri alma işlemi için kullanılan json nesnesi
public:
	/*mylog();
	~mylog();*/
	void Undo();  // Geri alma işlemini yap (Back)
	void Redo();  // Geri alma işlemini geri al (UnBack)
	void ClearLogs();  // İşlem günlüğünü temizle

	void OP_add(DrawObj* d);               // Yeni şekil ekleme işlemi
	void OP_del(DrawObj* d, int pos);      // Şekli silme işlemi
	void OP_moveStart(DrawObj* d, int pos);  // Taşıma işlemi başlatma (Belirli indeksli nesneyi taşıma)
	void OP_moveEnd(DrawObj* d);          // Taşıma işlemi sonlandırma (Belirli indeksli nesneyi taşıma)
	void OP_sizeStart(DrawObj* d, int pos);  // Yeniden boyutlandırma işlemi başlatma (Belirli indeksli nesneyi yeniden boyutlandırma)
	void OP_sizeEnd(DrawObj* d);           // Yeniden boyutlandırma işlemi sonlandırma (Belirli indeksli nesneyi yeniden boyutlandırma)
	void OP_textStart(DrawObj* d, int pos);  // Metin taşıma işlemi başlatma (Belirli indeksli metni taşıma)
	void OP_textEnd(DrawObj* d);           // Metin taşıma işlemi sonlandırma (Belirli indeksli metni taşıma)
	void OP_modifyStart(DrawObj* d, int pos); // Metni düzenleme işlemi başlatma
	void OP_modifyEnd(DrawObj* d);            // Metni düzenleme işlemi sonlandırma
};
