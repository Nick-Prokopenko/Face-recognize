//---------------------------------------------------------------------------

#ifndef Kode1H
#define Kode1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TMain : public TForm
{
__published:	// IDE-managed Components
        TMainMenu *MainMenu1;
        TMenuItem *N1;
        TMenuItem *N3;
        TMenuItem *N4;
        TMenuItem *N5;
        TMenuItem *N6;
        TPanel *Panel1;
        TEdit *Edit1;
        TEdit *Edit2;
        TButton *BPca;
        TButton *Button2;
        TPanel *Panel2;
        TEdit *Edit3;
        TEdit *Edit4;
        TPanel *Panel3;
        TLabel *Label1;
        TLabel *Label2;
        TMenuItem *N7;
        TMenuItem *N8;
        TButton *BHmm;
        TLabel *Label3;
        TPanel *Panel4;
        TButton *Button3;
        TButton *BSaveFoto;
        TButton *DelPrev;
        TButton *BAddBaza;
        TCheckBox *CheckBox1;
        TLabel *Label10;
        TLabel *Label8;
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall BPcaClick(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall N5Click(TObject *Sender);
        void __fastcall N6Click(TObject *Sender);
        void __fastcall N7Click(TObject *Sender);
        void __fastcall N8Click(TObject *Sender);
        void __fastcall BHmmClick(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall BSaveFotoClick(TObject *Sender);
        void __fastcall DelPrevClick(TObject *Sender);
        void __fastcall BAddBazaClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TMain(TComponent* Owner);
        void __fastcall TMain::IdleLoop(TObject*, bool& done);
};
//---------------------------------------------------------------------------
extern PACKAGE TMain *Main;
//---------------------------------------------------------------------------
#endif
