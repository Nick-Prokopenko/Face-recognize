//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "Kode1.h"
#include "About.h"
#include "EditBaza.h"
#include "FaceRecognize.cpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
#define WIDTHBYTES(bits) ((((bits) + 31) / 32) * 4)
TMain *Main;
int sohr_foto=0;
int snimok=0;
int resultPCA;
int resultHMM ;
const unsigned int N_Samples=39;  //количество человек дял Главных компонент
const unsigned int N_Sampl=10; // Кол-во фоток на каждого
CvEHMM **Ehmms;
CvSize size;
//---------------------------------------------------------------------------
CvCapture* capture = 0;
IplImage *frame=0, *cop=0, *frame_copy = 0;
IplImage* gray=0;
IplImage* panel=0;

static CvHaarClassifierCascade* cascade = 0;
const char* cascade_name ="haarcascade_frontalface_alt2.xml";
void ProcessFrame( IplImage* image );
void ResultRaspozn(int resultPCA,int resultHMM );




//---------------------------------------------------------------------------
// Создание API шного битмапа из интеловского RGB изображения
//---------------------------------------------------------------------------
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



void ResultRaspozn(int resultPCA,int resultHMM ){  //результат распознования
//ShowMessage(resultPCA);
//ShowMessage(resultHMM);
if (resultPCA==resultHMM && resultPCA==0 && resultHMM==0){
Main->Label3->Caption="ничего нет";
Main->Label3->Color=clYellow;
}
if (resultPCA==resultHMM && resultPCA!=0 && resultHMM!=0 ){
Main->Label3->Caption="Лицо есть в базе";
Main->Label3->Color=clLime;

IplImage* img_l;
img_l=cvLoadImage(("faces//s"+IntToStr(resultHMM)+"//1.pgm").c_str());
APIDrawIpl(0,0,img_l,Main->Panel2->Handle);
///ShowMessage("Доступ получен");
}
if (resultPCA!=resultHMM){
Main->Label3->Caption="Лица нет в базе";
Main->Label3->Color=clRed;
//ShowMessage("Доступ не получен");
}
}




//НММ
//----------------------------------------------------------------
void ExtractDCT( float* src, float* dst, int numVec, int dstLen )
{
    float *tmpSrc = src + 1;
    float *tmpDst = dst;
	int i;

    for( i = 0; i < numVec; i++ )
    {
        memcpy( tmpDst, tmpSrc, dstLen * sizeof( float ) );
        tmpSrc += dstLen + 1;
        tmpDst += dstLen;
    }
} 

//*****************************************************************************
// Обучение HMM
//*****************************************************************************
CvEHMM * Train(IplImage **images,int numImages/*,CvEHMM *tmplEhmm*/)
{
//Параметры обучения
const int OBS_WIDTH = 12;
const int OBS_HEIGHT = 12;

const int NO_DCT_COEFF_X = 4;
const int NO_DCT_COEFF_Y = 4;

const int STEP_X = 4;
const int STEP_Y = 4;

const bool SUPPRESS_INTESITY = true;
const int MAX_ITER = 80;
const double STOP_STEP_ITER = 0.01;

CvEHMM *tmplEhmm;

// 5 - суперсостояний по 3,6,6,6,3 - соответственно состояний в каждом
int noStates[] = { 5, 3, 6, 6, 6, 3 };

// количество Гауссовых смесей для каждого состояния.
int numMixtures[] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

//NO_DCT_COEFF_X * NO_DCT_COEFF_Y - 1    т.к. не учитываем первый коэффициент
int vecSize = NO_DCT_COEFF_X * NO_DCT_COEFF_Y - 1;

// Переводим в cvSize что бы удобнее было использовать
CvSize dctSize = cvSize( OBS_WIDTH, OBS_HEIGHT);
CvSize stepSize = cvSize( STEP_X, STEP_Y );
CvSize noDCTCoeff = cvSize( NO_DCT_COEFF_X, NO_DCT_COEFF_Y);
// Вектор наблюдений
CvImgObsInfo **obsInfoVec;
// Временное изображение
IplImage *iplImg;
// Размерность вектора наблюдений
int obsVecLen = NO_DCT_COEFF_X * NO_DCT_COEFF_Y;
// Количество наблюдений
CvSize noObs;
// Создали тренируемый объект
tmplEhmm = cvCreate2DHMM( noStates, numMixtures, vecSize );
// Для хранения предыдущего значения степени похожести
float oldLikelihood = 0;
// Счетчик итераций
int counter = 0;

int i;

    if( SUPPRESS_INTESITY )
    {
	//Подавляем первый коэффициент дискретного косинусного преобразования (ДКП)
	obsVecLen--;
    }

   // Создаем массив наблюдений
   obsInfoVec = new CvImgObsInfo*[ numImages ];

	for( i = 0; i < numImages; i++ )
    {
   // Загружаем указатель на изображение
       	iplImg = images[i];
   // Рассчитываем, сколько нужно произвести ДКП (DCT)
	noObs.width = ( iplImg->width - dctSize.width + stepSize.width ) / stepSize.width;
	noObs.height = ( iplImg->height - dctSize.height + stepSize.height ) / stepSize.height;
   // Создаем наблюдения для каждого преобразования
        obsInfoVec[ i ] = cvCreateObsInfo( noObs, obsVecLen );

        if( SUPPRESS_INTESITY )
        {
            float *observations = new float[ noObs.height * noObs.width * ( obsVecLen + 1 ) ];
			cvImgToObs_DCT( iplImg, observations, dctSize, noDCTCoeff, stepSize );
                        // Подавляем первый коэффициент ДКП
			ExtractDCT( observations, obsInfoVec[ i ]->obs, noObs.height * noObs.width, obsVecLen );
                        // Очистка
			if ( observations )
			{
				delete( observations);
			}
        }
     // Если не подавляем первый коэффициент
        else
        {
            cvImgToObs_DCT(iplImg, obsInfoVec[ i ]->obs, dctSize, noDCTCoeff, stepSize );
        }
		cvUniformImgSegm( obsInfoVec[ i ], tmplEhmm );
		cvReleaseImage( &iplImg );
    }

    cvInitMixSegm( obsInfoVec, numImages, tmplEhmm );

    int trained=0;

	// Процедура обучения
	while( ( !trained ) &&( counter < MAX_ITER ) )
    { 
	   int j;
	 	float likelihood = 0;
		counter++;

        cvEstimateHMMStateParams( obsInfoVec, numImages, tmplEhmm ); //error
        cvEstimateTransProb( obsInfoVec, numImages, tmplEhmm);
        for( j = 0; j < numImages; j++ )
        {
           cvEstimateObsProb( obsInfoVec[ j ], tmplEhmm );
           likelihood += cvEViterbi( obsInfoVec[ j ], tmplEhmm );
        }
        likelihood /= numImages * obsInfoVec[ 0 ]->obs_size;
        cvMixSegmL2( &obsInfoVec[ 0 ], numImages, tmplEhmm );
        trained = ( fabs( likelihood - oldLikelihood ) < STOP_STEP_ITER );
        oldLikelihood = likelihood;
    
   }

	// Очистка
	for( i = 0; i < numImages; i++ )
	{
		cvReleaseObsInfo( &(obsInfoVec[ i ]) );
	}
	delete []obsInfoVec;
return tmplEhmm;
} 
//*****************************************************************************
// Распознавание HMM
//*****************************************************************************
float ComputeLikelihood( IplImage* img, CvEHMM &tmplEhmm)
{
//Параметры распознавания
const int OBS_WIDTH = 12;
const int OBS_HEIGHT = 12;
const int NO_DCT_COEFF_X = 4;
const int NO_DCT_COEFF_Y = 4;
const int STEP_X = 4;
const int STEP_Y = 4;
const bool SUPPRESS_INTESITY = true;

CvSize dctSize = cvSize( OBS_WIDTH, OBS_HEIGHT);
CvSize stepSize = cvSize( STEP_X, STEP_Y );
CvSize noDCTCoeff = cvSize( NO_DCT_COEFF_X, NO_DCT_COEFF_Y);

CvSize noObs;

    int obsVecLen = noDCTCoeff.width * noDCTCoeff.height;
    CvImgObsInfo* info;
    
    if( SUPPRESS_INTESITY )
    {
        obsVecLen--;
    }

   // Рассчитываем, сколько нужно произвести ДКП (DCT)
	noObs.width = ( img->width - dctSize.width + stepSize.width ) / stepSize.width;
	noObs.height = ( img->height - dctSize.height + stepSize.height ) / stepSize.height;

	info = cvCreateObsInfo( noObs, obsVecLen );
	assert( info != 0 );
    
    if( SUPPRESS_INTESITY )
    {
        float *observations = new float[ noObs.height * noObs.width * ( obsVecLen + 1 ) ];
		cvImgToObs_DCT( img, observations, dctSize, noDCTCoeff, stepSize );
		ExtractDCT( observations, info->obs, noObs.height * noObs.width, obsVecLen );
		if ( observations )
		{
			delete( observations);
		}
    }
    else
    {
        cvImgToObs_DCT( &img, info->obs, dctSize, noDCTCoeff, stepSize );
    }
	//cvEstimateObsProb( info, &tmplEhmm );
	//return cvEViterbi( info, &tmplEhmm );
        cvEstimateObsProb( info, &tmplEhmm );
float retval ;
retval = cvEViterbi( info, &tmplEhmm );
return retval;
cvReleaseObsInfo(&info);

}

//-----------------------------------------------------------------





__fastcall TMain::TMain(TComponent* Owner)
        : TForm(Owner)
{
//Загружаем каскад Хаара
cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
    if( !cascade )
    {Close();}
capture = cvCaptureFromCAM(-1);
cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 640);//1280);
cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 480);//960);
Application->OnIdle = IdleLoop; // Поток обработки простоя}
}

void __fastcall TMain::IdleLoop(TObject*, bool& done)
{

    done = false;// Поток обработки простоя
    frame=0;  //кадр из видеопотока
    if(capture)
    {
// Даем приказ на захват кадра
            if( !cvGrabFrame( capture ))
                goto skip;
// Если удачно - получаем кадр
           frame = cvRetrieveFrame( capture );
           //   frame = cvQueryFrame( capture );
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

if (Main->CheckBox1->Checked==false){
if (faces->total==0) {

resultPCA=0;
       resultHMM=0;
     ResultRaspozn(resultPCA,resultHMM );
//pusto = cvCreateImage( cvSize(92,112),IPL_DEPTH_8U,1);
//cvCopy( pusto, cop, 0 );
//const char* name1 ="qqq.pgm";
//cvSaveImage(name1, cop);
      // DeleteFile("qqq.pgm");
      }
 if (faces->total!=0){

   // Обводим все лица в кадре
       for(int i = 0; i < (faces ? faces->total : 0); i++ )
        {CvRect* r = (CvRect*)cvGetSeqElem( faces, i );

//----------------------Обработка области лица----------
//------------------------------------------------------
cvRectangle(Grab, cvPoint(r->x-20,r->y-20), cvPoint(r->x+10+r->width,r->y+20+r->height),CV_RGB(0,255,0));
        cvSetImageROI(gray,*r);

cop = cvCreateImage(cvSize(92,112),IPL_DEPTH_8U, 1);
//IplImage* cop1= cvCreateImage(cvSize(92,112),IPL_DEPTH_8U, 1);
// Масштабируем под заданный размер
cvResize( gray, cop);
   /*
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

      */
const char* name ="qqq.pgm";
cvSaveImage(name, cop);

//--- Сбросили прямоугольник лица
        cvResetImageROI(gray);
       Main->BPca->Click() ;
       Main->BHmm->Click() ;
       ResultRaspozn(resultPCA,resultHMM );

       }
    }
                                }
                                if (Main->CheckBox1->Checked==true){
   // Обводим все лица в кадре
       for(int i = 0; i < (faces ? faces->total : 0); i++ )
        {CvRect* r = (CvRect*)cvGetSeqElem( faces, i );

//----------------------Обработка области лица----------
//------------------------------------------------------
cvRectangle(Grab, cvPoint(r->x-20,r->y-20), cvPoint(r->x+10+r->width,r->y+20+r->height),CV_RGB(0,255,0));
        cvSetImageROI(gray,*r);

cop = cvCreateImage(cvSize(92,112),IPL_DEPTH_8U, 1);
//IplImage* cop1= cvCreateImage(cvSize(92,112),IPL_DEPTH_8U, 1);
// Масштабируем под заданный размер
cvResize( gray, cop);

//--- Сбросили прямоугольник лица
        cvResetImageROI(gray);
       }

                                }
    }

IplImage* bigg;
bigg = cvCreateImage(cvSize(200,200),IPL_DEPTH_8U, 3);
cvResize( Grab, bigg);

APIDrawIpl(0,0,bigg,Main->Panel1->Handle);
APIDrawIpl(0,0,bigg,Main->Panel3->Handle);
cvReleaseMemStorage(&storage);
}



void __fastcall TMain::Button2Click(TObject *Sender)
{
int i=1;
AnsiString dirname =ExtractFileDir(ParamStr(0))+"\\faces\\s1" ;
Edit4->Text= dirname;
ForceDirectories(dirname);
const char* name1 =(dirname+"\\"+i+".pgm").c_str() ;
cvSaveImage(name1, cop);


}






//---------------------------------------------------------------------------
void __fastcall TMain::FormClose(TObject *Sender, TCloseAction &Action)
{
EditDB->Close() ;
//cvReleaseImage( &frame_copy );
//cvReleaseImage( &gray );
//cvReleaseCapture( &capture );
}
//---------------------------------------------------------------------------



void __fastcall TMain::BPcaClick(TObject *Sender)
{
resultPCA=0;
//const unsigned int N_Samples=10; // Количество изображений
//EditDB->IBDataSet3->Active=true;
//int N_SamplesT=StrToInt(EditDB->DBEdit6->Text);
IplImage* img_load=0;
IplImage* img_load_ch1[N_Samples];
IplImage* test_img=0;
IplImage* mean_img=0;             // Усредненное лицо (плавающая точка)


//****************************************
// Загрузка лиц (обучающая выборка)
//****************************************
for (int i=0;i<N_Samples;i++)
 {
  //             ie:;
img_load = cvLoadImage(("faces//s"+IntToStr(i+1)+"//1.pgm").c_str());
   //   if (!img_load)
   //   {i++;
   //   goto ie;}
         size=cvSize(img_load->width,img_load->height);
         img_load_ch1[i] = cvCreateImage( size, IPL_DEPTH_8U, 1 );
         cvSplit(img_load,img_load_ch1[i],NULL,NULL,NULL);
         // Вывод загруженных лиц на форму
       //  APIDrawIpl(10+(size.width+10)*i,50,img_load,Main->Handle);
     //  }
 }
if( !mean_img )
 {
        mean_img = cvCreateImage( size, IPL_DEPTH_32F, 1 );
 }

// Задаем критерии окончания процесса
//-----------------------------------
CvTermCriteria Tc;
Tc.type=CV_TERMCRIT_NUMBER | CV_TERMCRIT_EPS;
Tc.max_iter=100;
Tc.epsilon=0.001;

//-----------------------------------------------------
// Количество векторов базиса = кол-во изображений - 1
//-----------------------------------------------------
//const int nEigens = N_Samples-1;//можно и без -1
 const int nEigens = N_Samples-1;//можно и без -1
IplImage* eig_img[nEigens]; // Это базис
// Это базис
//IplImage* eig_img[nEigens];

CvMat *EigenVals;
EigenVals = cvCreateMat(1,nEigens,IPL_DEPTH_32F);
// Выделяем память под базис
for( i=0;i<N_Samples;i++)
{
        eig_img[i] = cvCreateImage( size, IPL_DEPTH_32F, 1 );
}
// Вычисляем базис
cvCalcEigenObjects(N_Samples, img_load_ch1,eig_img,CV_EIGOBJ_NO_CALLBACK,0,0,&Tc,mean_img,EigenVals->data.fl);
//****************************************
//****************************************
// Загрузка проверяемого лица
//****************************************
       i=0; // Номер индивида
       // Загрузим его второе изображение (для тренировки использовалось первое)
       img_load = cvLoadImage("qqq.pgm");
       test_img = cvCreateImage( size, IPL_DEPTH_8U, 1 );
       cvSplit(img_load,test_img,NULL,NULL,NULL);
       APIDrawIpl(10 + ((size.width+10)*(N_Samples-1))/2,50+(size.height+20)*2,img_load,Main->Handle);
//****************************************
// Выделение памяти под изображения
//****************************************
// Усредненное лицо
if( !mean_img )
 {mean_img = cvCreateImage( size, IPL_DEPTH_32F, 1 );}


//-----------------------------------------------------------
// Разложение тестируемого изображения по полученному базису
//-----------------------------------------------------------
float** coeffs;
coeffs = new float*[N_Samples];
for (i=0;i<N_Samples;i++)
{
coeffs[i] = new float [nEigens];
cvEigenDecomposite( img_load_ch1[i], nEigens, eig_img,CV_EIGOBJ_NO_CALLBACK,0,mean_img, coeffs[i] );
}
//-----------------------------------------------------------
// Задаем тестовое изображение
//-----------------------------------------------------------
float projectedTestFace[nEigens];
cvEigenDecomposite( test_img, nEigens, eig_img,CV_EIGOBJ_NO_CALLBACK,0,mean_img,projectedTestFace);
//-----------------------------------------------------------
double leastDistSq = DBL_MAX;

int iTrain,iNearest =0;
double razn=0;
for (iTrain=0;iTrain<N_Samples;iTrain++)
{
 double distSq=0;
 for (i=0;i<nEigens;i++)
 {
 float d_i = projectedTestFace[i] - coeffs[iTrain][i];
 distSq += d_i*d_i/EigenVals->data.fl[i];
 }
Edit2->Text =distSq;
//if(distSq>0 && distSq<2 ){ShowMessage ("Данного человека нет в базе");
//goto zzz;
//}
if(distSq<leastDistSq){
//Edit1->Text =leastDistSq;
leastDistSq = distSq;
iNearest = iTrain;
//Edit4->Text =distSq;
razn=distSq;
}

            //   if (distSq<leastDistSq)
      //if (distSq>2.1)
//{//ShowMessage ("Данного человека нет в базе");

//Edit4->Text ="qqwwq";
//Edit2->Text ="";
}

//APIDrawIpl(10+(10+size.width),50+(size.height+20)*2,img_load_ch1[iNearest],Main->Handle);

//Edit4->Text =razn;
//}

//****************************************
//Результат ткстирования (Вывод распознанного индивида)
//****************************************

Label2->Caption=IntToStr(iNearest+1);
//APIDrawIpl(0,0,img_load_ch1[iNearest],Panel2->Handle);---------------
resultPCA=iNearest+1;
//ShowMessage(resultPCA);
//APIDrawIpl(10+(100+size.width),50+(size.height+20)*2,img_load_ch1[iNearest],Main->Handle);



//for ( i=0;i<N_Samples;i++)
 //{
         // Вывод загруженных лиц на форму
//if(i!=iNearest)
//{

//cvLine(img_load_ch1[i],cvPoint(0,0),cvPoint(size.width,size.height),CV_RGB(255,0,0), 3, 4);  //error
//cvLine(img_load_ch1[i],cvPoint(size.width,0),cvPoint(0,size.height),CV_RGB(255,0,0), 3, 8); //error
//}
//         APIDrawIpl(10+(size.width+10)*i,50+(size.height+20)*3,img_load_ch1[i],Main->Handle);
 //}

zzz:
//****************************************
// Освобождаем память
//****************************************
for (int k=0;k<N_Samples;k++)
 {
        cvReleaseImage( &eig_img[k] );
        cvReleaseImage( &img_load_ch1[k] );
 }

// Аккуратно освобождаем массив
for (i=0;i<N_Samples;i++)
{
 delete coeffs[i];
}
delete coeffs;

 cvReleaseImage( &img_load);
 cvReleaseImage( &mean_img);
 cvReleaseImage( &test_img);
 

}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------


void __fastcall TMain::N5Click(TObject *Sender)
{
AboutBox->Show() ;
}
//---------------------------------------------------------------------------


void __fastcall TMain::N6Click(TObject *Sender)
{
Main->Close();
}
//---------------------------------------------------------------------------



void __fastcall TMain::N7Click(TObject *Sender)
{   //show edit polzovatel
Main->Hide() ;
EditDB->Show() ;
EditDB->TabSheet1->TabVisible=true;
EditDB->TabSheet2->TabVisible=true;
EditDB->TabSheet3->TabVisible=false;

}
//---------------------------------------------------------------------------

void __fastcall TMain::N8Click(TObject *Sender)
{  //show edit shurnal
Main->Hide() ;
EditDB->Show() ;
EditDB->TabSheet1->TabVisible=false;
EditDB->TabSheet2->TabVisible=false;
EditDB->TabSheet3->TabVisible=true;
}
//---------------------------------------------------------------------------

void __fastcall TMain::BHmmClick(TObject *Sender)
{
resultHMM=0;
IplImage* img_loadh1;
IplImage* img_load_ch1h1[N_Sampl];
IplImage* test_imgh1;


//int i=0; // Номер индивида
     //  j=1 ; // Номер фотографии этого индивида
 img_loadh1 = cvLoadImage("qqq.pgm");
 size=cvSize( img_loadh1->width, img_loadh1->height);
       test_imgh1 = cvCreateImage( size, IPL_DEPTH_8U, 1 );
       cvSplit(img_loadh1,test_imgh1,NULL,NULL,NULL);
//       APIDrawIpl(10 + ((size.width+10)*(N_Sampl-1))/2,50+(size.height+20),img_loadh1,Main->Handle);
//****************************************
// Тест
//****************************************
float *likehood;
likehood = new float[N_Samples];

for (int j=0;j<N_Samples;j++) // Цикл по разным людям
 {
likehood[j] = ComputeLikelihood(test_imgh1,*Ehmms[j]);
 }
float max=likehood[0];
int nmax=0;
for (j=1;j<N_Samples;j++) // Цикл по разным людям
 {
if(max<likehood[j]){max=likehood[j];nmax=j;}
 };
delete likehood;
j=nmax;
/*
for ( int i=0;i<N_Sampl;i++) // Цикл по фотографиям
 {
         img_loadh1 = cvLoadImage(("faces//s"+IntToStr(j+1)+"//"+IntToStr(i+1)+".pgm").c_str());
         size=cvSize(img_loadh1->width,img_loadh1->height);
         img_load_ch1h1[i] = cvCreateImage( size, IPL_DEPTH_8U, 1 );
         cvSplit(img_loadh1,img_load_ch1h1[i],NULL,NULL,NULL);
         // Вывод загруженных лиц на форму
         APIDrawIpl(10+(size.width+10)*i,50,img_loadh1,Main->Handle);
 }*/
resultHMM=j+1;
 cvReleaseImage( &img_loadh1);
 cvReleaseImage( &test_imgh1);
 delete []img_load_ch1h1;
}
//---------------------------------------------------------------------------




void __fastcall TMain::FormShow(TObject *Sender)
{
//-------Обучение HMM---------------------
IplImage* img_loadh;
IplImage* img_load_ch1h[N_Sampl];
IplImage* test_imgh;
Ehmms=new CvEHMM* [N_Samples];

//****************************************
// Загрузка лиц и цикл обучения
//****************************************
for (int j=0;j<N_Samples;j++) // Цикл по разным людям
 {
// Загружаем набор фотографий одного человека и обучаем одну марковскую модель,
// затем загружаем следующий набор и обуаем следующую марковскую модель.

for (int i=0;i<N_Sampl;i++) // Цикл по фотографиям
 {
         img_loadh = cvLoadImage(("faces//s"+IntToStr(j+1)+"//"+IntToStr(i+1)+".pgm").c_str());
         size=cvSize(img_loadh->width,img_loadh->height);
         img_load_ch1h[i] = cvCreateImage( size, IPL_DEPTH_8U, 1 );

         cvSplit(img_loadh,img_load_ch1h[i],NULL,NULL,NULL);
         // Вывод загруженных лиц на форму
         APIDrawIpl(10+(size.width+10)*i,50,img_loadh,Main->Handle);
 }
Ehmms[j]=Train(img_load_ch1h,N_Sampl);
}
//-----Конец обучения HMM-----------------------------

Panel3->Visible=false;
Panel4->Visible=false;
Button3->Visible=false;
BAddBaza->Visible=false;
BSaveFoto->Visible=false;
}
//---------------------------------------------------------------------------






void __fastcall TMain::Button1Click(TObject *Sender)
{
snimok=1;
BAddBaza->Enabled=false;  //включить кнопки для работы с фото
BSaveFoto->Enabled=true;
DelPrev->Enabled =true;

///IplImage* snim;
///snim = cvCreateImage(cvSize(94,112),IPL_DEPTH_8U, 1);
///cvResize(cop, snim);
//cvCvtColor(cop, gray, CV_BGR2GRAY);
const char* name1 ="temp.pgm" ;
cvSaveImage(name1, cop);
APIDrawIpl(0,0,cop,Panel4->Handle);
}
//---------------------------------------------------------------------------

void __fastcall TMain::BSaveFotoClick(TObject *Sender)
{
if (sohr_foto>10){
ShowMessage("База фотографий создана. Сохраните изменения");
BAddBaza->Enabled=true;
BAddBaza->SetFocus();
BSaveFoto->Enabled=false;
}
if (snimok>0){
EditDB->IBDataSet5->Close() ;
EditDB->IBDataSet5->Open() ;
//save foto procedere
AnsiString dirname =ExtractFileDir(ParamStr(0))+"\\faces\\"+(StrToInt(EditDB->DBEdit6->Text)) ;
ForceDirectories(dirname);
const char* forfoto =(dirname+"\\"+(sohr_foto+1)+".pgm").c_str() ;
cvSaveImage(forfoto, cop);
sohr_foto=sohr_foto+1 ;

}
else{
ShowMessage("Нет снимков.Нажмите кнопку-Сделать снимок");
}
if (sohr_foto==10) {
ShowMessage("Это последняя фотография для базы шаблонов. Следующая будет основной");
}
        
}
//---------------------------------------------------------------------------

void __fastcall TMain::DelPrevClick(TObject *Sender)
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

void __fastcall TMain::BAddBazaClick(TObject *Sender)
{
sohr_foto=0;
EditDB->IBDataSet1->Post() ;
//BSaveEdDB->Click() ;
EditDB->TabSheet1->TabVisible=true;
CheckBox1->Checked=false;
EditDB->TabSheet1->Show() ;
        
}
//---------------------------------------------------------------------------





