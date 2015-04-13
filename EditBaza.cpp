//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "Kode1.h"
#include "EditBaza.h"
#include "FaceRecognize.cpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
#define WIDTHBYTES(bits) ((((bits) + 31) / 32) * 4)
TEditDB *EditDB;
int snimok=0;

CvCapture* capture1 = 0;

IplImage *frame1=0, *cop1=0, *frame_copy1 = 0;
IplImage* gray1=0;
IplImage* panel1=0;

static CvHaarClassifierCascade* cascade1 = 0;
const char* cascade_name1 ="haarcascade_frontalface_alt2.xml";
void ProcessFrame1( IplImage* image1 );
void TimeRasp();    //для внесения в журнал жанных
//о распознанном лице

HBITMAP CreateRGBBitmap(IplImage* _Grab)
         {
char *App;
             LPBITMAPINFO lpbi = new BITMAPINFO;
             lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
             lpbi->bmiHeader.biWidth = _Grab->width;
             lpbi->bmiHeader.biHeight =_Grab->height;
             lpbi->bmiHeader.biPlanes = 1;
             lpbi->bmiHeader.biBitCount = 24;
             lpbi->bmiHeader.biCompression = BI_RGB;
             lpbi->bmiHeader.biSizeImage = WIDTHBYTES((DWORD)_Grab->width * 8) * _Grab->height;
             lpbi->bmiHeader.biXPelsPerMeter = 0;
             lpbi->bmiHeader.biYPelsPerMeter = 0;
             lpbi->bmiHeader.biClrUsed = 0;
             lpbi->bmiHeader.biClrImportant = 0;
             void* pBits;
             HBITMAP hBitmap = CreateDIBSection(
                 NULL,
                 lpbi,
                 DIB_RGB_COLORS,
                 (void **)&pBits,
                 NULL,
                 0 );
             delete lpbi;
             if ( hBitmap )
App=(char*)pBits;

if(_Grab->nChannels==1) // Серое или бинарное
  {

for (int i=0;i<_Grab->height;i++)
{
 for (int j=0;j<_Grab->width;j++)
 {
  App[_Grab->width*3*(_Grab->height-i-1)+j*3]=_Grab->imageData[_Grab->width*(i)+j];
  App[_Grab->width*3*(_Grab->height-i-1)+j*3+1]=_Grab->imageData[_Grab->width*(i)+j];
  App[_Grab->width*3*(_Grab->height-i-1)+j*3+2]=_Grab->imageData[_Grab->width*(i)+j];
 }
}

  }

if(_Grab->nChannels==3) // Цветное
  {
for (int i=0;i<_Grab->height;i++)
{
memcpy(App+_Grab->width*3*(_Grab->height-i-1),_Grab->imageData+_Grab->width*3*i,_Grab->width*3);           // Копируем память
}
}
return hBitmap;
}


//Для вывода изображения
void  APIDrawIpl(int x,int y,IplImage* _Grab,void *HANDLE)
{
HDC hMemDC,hDC;
hDC=GetDC(HANDLE);
hMemDC = CreateCompatibleDC(hDC);
HBITMAP Bitmap=CreateRGBBitmap(_Grab);
SelectObject(hMemDC,Bitmap);
BitBlt(hDC,x,y,_Grab->width,_Grab->height,hMemDC,0,0,SRCCOPY);
DeleteObject(Bitmap);
DeleteDC(hMemDC);
DeleteDC(hDC);
}


void TimeRasp()
{
EditDB->IBDataSet4->Open() ;
EditDB->IBDataSet4->Insert() ;
EditDB->IBDataSet4->FieldByName("ID_USR")->AsInteger=4;
//вместо 4 - номер каталога, к-й будет определяться при
// при распозновании
EditDB->IBDataSet4->FieldByName("VREMIA")->AsString=DateToStr(Date())+" "+TimeToStr(Time());
EditDB->IBDataSet4->FieldByName("REZT")->AsString="распознан";
//результат будет определяться значением переменной
// которая будет 1 есть в базе и 0, елси нет
EditDB->IBDataSet4->Post();
EditDB->IBDataSet2->Close() ;
EditDB->IBDataSet2->Open() ;
}
//---------------------------------------------------------------------------
__fastcall TEditDB::TEditDB(TComponent* Owner)
        : TForm(Owner)
{ /*
//Загружаем каскад Хаара
cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
    if( !cascade )
    {Close();}
capture = cvCaptureFromCAM(-1);
cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 640);//1280);
cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 480);//960);
Application->OnIdle = IdleLoop; // Поток обработки простоя}  */
}

/*void __fastcall TEditDB::IdleLoop(TObject*, bool& done)
{
    done = false;// Поток обработки простоя
    frame=0;  //кадр из видеопотока
    if(capture)
    {
// Даем приказ на захват кадра
            if( !cvGrabFrame( capture ))
                goto skip;
// Если удачно - получаем кадр
        //   frame = cvRetrieveFrame( capture );
              frame = cvQueryFrame( capture );
            if( !frame )
// Если захват не удался - пропускаем
                goto skip;
// размер как у кадра, реальный
           if( !frame_copy )
              frame_copy = cvCreateImage( cvSize(frame->width,frame->height),
                                        IPL_DEPTH_8U, frame->nChannels );
// Делаем копию кадра
           cvCopy( frame, frame_copy, 0 );
//            if( !frame_copy )
//               frame_copy = cvCreateImage(cvSize(200,200),IPL_DEPTH_8U, 3);

// Масштабируем под заданный размер

//cvResize( frame, frame_copy);
  cvFlip(frame_copy,frame_copy);
// Выделяем память под серую копию кадра
            if( !gray )
                gray = cvCreateImage( cvSize(frame_copy->width,frame_copy->height),
                                            IPL_DEPTH_8U,1);
// Обработка полученного кадра
            ProcessFrame( frame_copy );
    }
skip:;

}
void ProcessFrame(IplImage* Grab)
{
static CvMemStorage* storage = 0;
storage = cvCreateMemStorage(0);
cvCvtColor(Grab, gray, CV_BGR2GRAY);      // Получаем серый цвет
// Необходимо удалить белый бордюр
cvRectangle(gray, cvPoint(0,0), cvPoint(gray->width-1,gray->height-1),CV_RGB(0,0,0));
//---------------------------------------------------------------------------
//----------------------Поиск лица-------------------------------------------
    if( cascade )
    {
//  главная функция
CvSeq* faces = cvHaarDetectObjects( gray, cascade, storage,
                                            1.1, 2, 0
                                          //  |CV_HAAR_FIND_BIGGEST_OBJECT
                                           // |CV_HAAR_DO_ROUGH_SEARCH
                                          |CV_HAAR_DO_CANNY_PRUNING
                                            //|CV_HAAR_SCALE_IMAGE
                                            ,cvSize(30, 30));
// Обводим все лица в кадре
       for(int i = 0; i < (faces ? faces->total : 0); i++ )
        {CvRect* r = (CvRect*)cvGetSeqElem( faces, i );

//----------------------Обработка области лица----------
//------------------------------------------------------
cvRectangle(Grab, cvPoint(r->x-20,r->y-20), cvPoint(r->x+10+r->width,r->y+20+r->height),CV_RGB(0,255,0));
        cvSetImageROI(gray,*r);

cop = cvCreateImage(cvSize(92,112),IPL_DEPTH_8U, 1);
IplImage* cop1= cvCreateImage(cvSize(92,112),IPL_DEPTH_8U, 1);
// Масштабируем под заданный размер
cvResize( gray, cop);

        float kernel[9];
        kernel[0]=-0.1;
        kernel[1]=-0.1;
        kernel[2]=-0.1;

        kernel[3]=-0.1;
        kernel[4]=2;
        kernel[5]=-0.1;

        kernel[6]=-0.1;
        kernel[7]=-0.1;
        kernel[8]=-0.1;

        // матрица
        CvMat kernel_matrix=cvMat(3,3,CV_32FC1,kernel);
        // накладываем фильтр
     //   cvFilter2D(cop, cop1, &kernel_matrix, cvPoint(-1,-1));



//if (i<3){
//const char* name =("qqq"+IntToStr(i)+".pgm").c_str();
//cvSaveImage(name, cop);
         //}

const char* name ="123.pgm";
cvSaveImage(name, cop);




//--- Сбросили прямоугольник лица
        cvResetImageROI(gray);

       }
    }
//----------------------Поиск лица-------------------------------------------
//---------------------------------------------------------------------------


APIDrawIpl(0,0,Grab,EditDB->Panel1->Handle);

//APIDrawIpl(0,0,Grab,EditDB->Panel2->Handle);
//APIDrawIpl(10,10,Grab,Main->Handle);          // Рисуем результат
cvReleaseMemStorage(&storage);
}  */
//---------------------------------------------------------------------------
void __fastcall TEditDB::N2Click(TObject *Sender)
{
EditDB->Hide();
Main->Show() ;
}
//---------------------------------------------------------------------------
void __fastcall TEditDB::FormCloseQuery(TObject *Sender, bool &CanClose)
{
EditDB->Hide();
Main->Show() ;
}
//---------------------------------------------------------------------------



void __fastcall TEditDB::FormCreate(TObject *Sender)
{
BSaveEdDB->Enabled=false;
BEditFoto->Enabled=false;
}
//---------------------------------------------------------------------------





void __fastcall TEditDB::TabSheet2Show(TObject *Sender)
{
Creat_Kat->Enabled=false;
Button1->Enabled=false;
BAddBaza->Enabled=false;
BSaveFoto->Enabled=false;
DelPrev->Enabled=false;
IBDataSet5->Open() ;
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::TabSheet1Show(TObject *Sender)
{
BDelDB->Enabled =true;
BSaveEdDB->Enabled=false;
BEditFoto->Enabled=false;
BEditDB->Enabled=true;
DBEdit1->Enabled=false;
DBEdit2->Enabled=false;
DBEdit3->Enabled=false;
ComboBox4->Enabled=false;
ComboBox4->Text=DBEdit4->Text;
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::TabSheet3Show(TObject *Sender)
{
BSort->Enabled=false;
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::N4Click(TObject *Sender)
{

EditDB->TabSheet1->TabVisible=true;
EditDB->TabSheet2->TabVisible=true;
EditDB->TabSheet3->TabVisible=false;
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::N5Click(TObject *Sender)
{
EditDB->TabSheet1->TabVisible=false;
EditDB->TabSheet2->TabVisible=false;
EditDB->TabSheet3->TabVisible=true;
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::BEditDBClick(TObject *Sender)
{
BSaveEdDB->Enabled=true;
BDelDB->Enabled=false;
BEditFoto->Enabled=true;
DBEdit1->Enabled=true;
DBEdit2->Enabled=true;
DBEdit3->Enabled=true;
ComboBox4->Enabled=true;
IBDataSet1->Edit();
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::BSaveEdDBClick(TObject *Sender)
{
IBDataSet1->FieldByName("FIO")->AsString=DBEdit1->Text;
IBDataSet1->FieldByName("TAB_NUM")->AsString=DBEdit2->Text;
IBDataSet1->FieldByName("ORGANIZ")->AsString=DBEdit3->Text;
IBDataSet1->FieldByName("UROV")->AsString=ComboBox4->Text;
IBDataSet1->Post();
IBDataSet1->Close() ;
IBDataSet1->Open() ;
BSaveEdDB->Enabled=false;
BDelDB->Enabled=true;
BEditFoto->Enabled=false;
DBEdit1->Enabled=false;
DBEdit2->Enabled=false;
DBEdit3->Enabled=false;
ComboBox4->Enabled=false;

}
//---------------------------------------------------------------------------

void __fastcall TEditDB::BEditFotoClick(TObject *Sender)
{
//вывести сообщение, о том что перед добавлением новых фото
//все старые фото будут удалены
//написать процедуру удаление файлов из каталога
Edit1->Enabled=false;
Edit2->Enabled=false;
Edit3->Enabled=false;  //отключаем поля ввода данных и кнопку создать каталог
ComboBox1->Enabled=false;
Creat_Kat->Enabled=false;


EditDB->TabSheet2->Show() ;
Button1->Enabled=true;
BAddBaza->Enabled=false;  //включить кнопки для работы с фото
BSaveFoto->Enabled=false;
DelPrev->Enabled =false;
EditDB->TabSheet1->TabVisible=false;

}



//---------------------------------------------------------------------------

int sohr_foto=0;

void __fastcall TEditDB::Button1Click(TObject *Sender)
{
snimok=1;
BAddBaza->Enabled=false;  //включить кнопки для работы с фото
BSaveFoto->Enabled=true;
DelPrev->Enabled =true;
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::DelPrevClick(TObject *Sender)
{
if (sohr_foto>0) //удаляем, если что-то сохранено
{

sohr_foto=sohr_foto-1 ;
snimok=0;
Edit1->Text=sohr_foto;
}
else{
ShowMessage("Нет сохраненных фото");
}

}
//---------------------------------------------------------------------------

void __fastcall TEditDB::BSaveFotoClick(TObject *Sender)
{
if (sohr_foto>10){
ShowMessage("База фотографий создана. Сохраните изменения");
BAddBaza->Enabled=true;
BAddBaza->SetFocus();
BSaveFoto->Enabled=false;
}
if (snimok>0){
//save foto procedere
sohr_foto=sohr_foto+1 ;
Edit1->Text=sohr_foto;
}
else{
ShowMessage("Нет снимков.Нажмите кнопку-Сделать снимок");
}
if (sohr_foto==10) {
ShowMessage("Это последняя фотография для базы шаблонов. Следующая будет основной");
}

}
//---------------------------------------------------------------------------

void __fastcall TEditDB::BAddBazaClick(TObject *Sender)
{
sohr_foto=0;
IBDataSet1->Post() ;
//BSaveEdDB->Click() ;
EditDB->TabSheet1->TabVisible=true;
EditDB->TabSheet1->Show() ;

}
//---------------------------------------------------------------------------





void __fastcall TEditDB::provetkaClick(TObject *Sender)  // проверка при добавлении
{
if (Edit1->Text !="" && Edit2->Text !="" && Edit3->Text !="" && ComboBox1->Text !="")
{Creat_Kat->Enabled=true;}
else{
Creat_Kat->Enabled=false;
}

}
//---------------------------------------------------------------------------

void __fastcall TEditDB::IBDataSet1AfterScroll(TDataSet *DataSet)
{
ComboBox4->Text=DBEdit4->Text;        
}
//---------------------------------------------------------------------------






void __fastcall TEditDB::Button7Click(TObject *Sender)
{
IBDataSet1->Next();
if (IBDataSet1->Eof){
IBDataSet1->First() ;}
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::Button6Click(TObject *Sender)
{
IBDataSet1->Prior();
if (IBDataSet1->Bof){
IBDataSet1->Last() ;}
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::BDelDBClick(TObject *Sender)
{
//IBDataSet2->Active=false;
AnsiString zap1;
zap1="delete from SHURNAL where ID_USR='"+DBEdit5->Text+ "'";
//ShowMessage(zap1);
//IBDataSet2->DeleteSQL->Add(zap1);
//IBDataSet2->Active=true;
IBSQL1->Close() ;
IBSQL1->SQL->Add(zap1);
IBSQL1->Open ;
//IBDataSet2->Delete() ;
IBDataSet1->Delete() ;
IBDataSet1->Close() ;
IBDataSet1->Open() ;
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::Edit1Change(TObject *Sender)
{
provetka->Click() ;
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::Edit2Change(TObject *Sender)
{
provetka->Click() ;        
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::Edit3Change(TObject *Sender)
{
provetka->Click() ;        
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::Edit4Change(TObject *Sender)
{
provetka->Click() ;        
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::Edit5Change(TObject *Sender)
{
provetka->Click() ;        
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::ComboBox1Change(TObject *Sender)
{
provetka->Click() ;
}
//---------------------------------------------------------------------------


void __fastcall TEditDB::Creat_KatClick(TObject *Sender)
{
IBDataSet3->Close() ;
IBDataSet3->Open() ;
IBDataSet5->Open() ;
IBDataSet1->Open() ;
IBDataSet1->Insert() ;
IBDataSet1->FieldByName("FIO")->AsString=Edit1->Text;
IBDataSet1->FieldByName("TAB_NUM")->AsInteger=StrToInt(Edit2->Text);
IBDataSet1->FieldByName("ORGANIZ")->AsString=Edit3->Text;
IBDataSet1->FieldByName("UROV")->AsString=ComboBox1->Text;
if (StrToInt(DBEdit6->Text)>0){
IBDataSet1->FieldByName("ID_KAT")->AsString=StrToInt(DBEdit6->Text)+1;}
IBDataSet1->Post() ;
Creat_Kat->Enabled=false;

AnsiString dirname =ExtractFileDir(ParamStr(0))+"\\faces\\"+(StrToInt(DBEdit6->Text)+1);
Main->Edit4->Text= dirname;
ForceDirectories(dirname);
BEditFoto->Click() ;
IBDataSet1->Edit() ;
EditDB->Hide() ;
Main->CheckBox1->Checked=true;
Main->Panel1->Visible=false;
Main->Panel2->Visible=false;
Main->Label3->Visible=false;

Main->Show();
Main->Panel3->Visible=true;
Main->Panel4->Visible=true;
Main->Button3->Visible=true;
Main->BAddBaza->Visible=true;
Main->BSaveFoto->Visible=true;

}
//---------------------------------------------------------------------------

void __fastcall TEditDB::FormShow(TObject *Sender)
{
IBDatabase1->Connected=true;
IBTransaction1->Active=true;
IBDataSet1->Active=true;
IBDataSet2->Active=true;
IBDataSet3->Active=true;

}
//---------------------------------------------------------------------------


void __fastcall TEditDB::FormHide(TObject *Sender)
{
IBDataSet1->Active=false;
IBDataSet2->Active=false;
IBDataSet3->Active=false;
IBDatabase1->Connected=false;
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::Button5Click(TObject *Sender)
{
IBDataSet2->Prior();
if (IBDataSet2->Bof){
IBDataSet2->Last() ;}
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::Button4Click(TObject *Sender)
{
IBDataSet2->Next();
if (IBDataSet2->Eof){
IBDataSet2->First() ;}
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::Button2Click(TObject *Sender)
{
IBDataSet2->Delete() ;
IBDataSet2->Active=false;
IBDataSet2->Active=true;
}
//---------------------------------------------------------------------------

void __fastcall TEditDB::Button3Click(TObject *Sender)
{
Edit8->Text=DateToStr(Date())+" "+TimeToStr(Time());
//DateTimePicker1->DateTime ;
Edit9->Text=MonthCalendar1->Date ;
TimeRasp();
}
//---------------------------------------------------------------------------





