//---------------------------------------------------------------------------

#ifndef EditBazaH
#define EditBazaH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <DB.hpp>
#include <DBGrids.hpp>
#include <Grids.hpp>
#include <IBCustomDataSet.hpp>
#include <IBDatabase.hpp>
#include <DBCtrls.hpp>
#include <Mask.hpp>
#include <IBUpdateSQL.hpp>
#include <IBQuery.hpp>
#include <IBSQL.hpp>
#include <AppEvnts.hpp>
//---------------------------------------------------------------------------
class TEditDB : public TForm
{
__published:	// IDE-managed Components
        TMainMenu *MainMenu1;
        TMenuItem *N1;
        TMenuItem *N2;
        TMenuItem *N3;
        TMenuItem *N4;
        TMenuItem *N5;
        TMenuItem *N6;
        TPageControl *PageControl1;
        TTabSheet *TabSheet1;
        TGroupBox *GroupBox1;
        TButton *BDelDB;
        TButton *BSaveEdDB;
        TButton *BEditDB;
        TTabSheet *TabSheet2;
        TGroupBox *GroupBox3;
        TLabel *Label2;
        TLabel *Label5;
        TLabel *Label6;
        TLabel *Label7;
        TLabel *Label8;
        TLabel *Label9;
        TLabel *Label10;
        TEdit *Edit1;
        TEdit *Edit2;
        TEdit *Edit3;
        TComboBox *ComboBox1;
        TButton *Creat_Kat;
        TPanel *Panel1;
        TPanel *Panel2;
        TButton *BSaveFoto;
        TButton *DelPrev;
        TButton *BAddBaza;
        TTabSheet *TabSheet3;
        TGroupBox *GroupBox4;
        TButton *Button3;
        TButton *Button2;
        TGroupBox *GroupBox5;
        TLabel *Label12;
        TEdit *Edit8;
        TLabel *Label13;
        TEdit *Edit9;
        TLabel *Label14;
        TEdit *Edit10;
        TLabel *Label15;
        TComboBox *ComboBox2;
        TLabel *Label16;
        TComboBox *ComboBox3;
        TLabel *Label17;
        TDateTimePicker *DateTimePicker1;
        TRadioButton *RadioButton1;
        TRadioButton *RadioButton2;
        TRadioButton *RadioButton3;
        TRadioButton *RadioButton4;
        TRadioButton *RadioButton5;
        TRadioButton *RadioButton6;
        TButton *BSort;
        TButton *Button4;
        TButton *Button5;
        TButton *Button6;
        TButton *Button7;
        TLabel *Label18;
        TLabel *Label19;
        TLabel *Label20;
        TLabel *Label21;
        TButton *BEditFoto;
        TButton *provetka;
        TIBDatabase *IBDatabase1;
        TIBDataSet *IBDataSet1;
        TDBGrid *DBGrid1;
        TDataSource *DataSource1;
        TIBTransaction *IBTransaction1;
        TIntegerField *IBDataSet1ID_USER;
        TIBStringField *IBDataSet1FIO;
        TIntegerField *IBDataSet1TAB_NUM;
        TIBStringField *IBDataSet1ORGANIZ;
        TIBStringField *IBDataSet1UROV;
        TDBEdit *DBEdit1;
        TDBEdit *DBEdit2;
        TDBEdit *DBEdit3;
        TComboBox *ComboBox4;
        TDBEdit *DBEdit4;
        TDBEdit *DBEdit5;
        TIBQuery *IBQuery1;
        TIntegerField *IBDataSet1ID_KAT;
        TDBGrid *DBGrid2;
        TIBDataSet *IBDataSet2;
        TDataSource *DataSource2;
        TDataSource *DataSource3;
        TIBDataSet *IBDataSet3;
        TIntegerField *OPMAX;
        TDBEdit *DBEdit6;
        TIntegerField *IBDataSet2ID_SOB;
        TIBStringField *IBDataSet2FIO;
        TDateTimeField *IBDataSet2VREMIA;
        TIBStringField *IBDataSet2REZT;
        TMonthCalendar *MonthCalendar1;
        TIBDataSet *IBDataSet4;
        TIBSQL *IBSQL1;
        TIBDataSet *IBDataSet5;
        TDataSource *DataSource4;
        TButton *Button1;
        void __fastcall N2Click(TObject *Sender);
        void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall TabSheet2Show(TObject *Sender);
        void __fastcall TabSheet1Show(TObject *Sender);
        void __fastcall TabSheet3Show(TObject *Sender);
        void __fastcall N4Click(TObject *Sender);
        void __fastcall N5Click(TObject *Sender);
        void __fastcall BEditDBClick(TObject *Sender);
        void __fastcall BSaveEdDBClick(TObject *Sender);
        void __fastcall BEditFotoClick(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall DelPrevClick(TObject *Sender);
        void __fastcall BSaveFotoClick(TObject *Sender);
        void __fastcall BAddBazaClick(TObject *Sender);
        void __fastcall provetkaClick(TObject *Sender);
        void __fastcall IBDataSet1AfterScroll(TDataSet *DataSet);
        void __fastcall Button7Click(TObject *Sender);
        void __fastcall Button6Click(TObject *Sender);
        void __fastcall BDelDBClick(TObject *Sender);
        void __fastcall Edit1Change(TObject *Sender);
        void __fastcall Edit2Change(TObject *Sender);
        void __fastcall Edit3Change(TObject *Sender);
        void __fastcall Edit4Change(TObject *Sender);
        void __fastcall Edit5Change(TObject *Sender);
        void __fastcall ComboBox1Change(TObject *Sender);
        void __fastcall Creat_KatClick(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall FormHide(TObject *Sender);
        void __fastcall Button5Click(TObject *Sender);
        void __fastcall Button4Click(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall Button3Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TEditDB(TComponent* Owner);
     //   void __fastcall TEditDB::IdleLoop(TObject*, bool& done);
};
//---------------------------------------------------------------------------
extern PACKAGE TEditDB *EditDB;
//---------------------------------------------------------------------------
#endif
