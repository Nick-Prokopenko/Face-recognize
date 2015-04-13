//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "cv.h"
#include "highgui.h"
#include "cvcam.h"
#include "cvaux.h"
//---------------------------------------------------------------------------
USEFORM("Kode1.cpp", Main);
USEFORM("About.cpp", AboutBox);
USEFORM("EditBaza.cpp", EditDB);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->CreateForm(__classid(TMain), &Main);
                 Application->CreateForm(__classid(TAboutBox), &AboutBox);
                 Application->CreateForm(__classid(TEditDB), &EditDB);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        catch (...)
        {
                 try
                 {
                         throw Exception("");
                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
        return 0;
}
//---------------------------------------------------------------------------
