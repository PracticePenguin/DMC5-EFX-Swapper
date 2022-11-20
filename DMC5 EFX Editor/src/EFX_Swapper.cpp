#include "EFX_Swapper.h"
#include "gui/MainFrame.h"

wxIMPLEMENT_APP(EFX_Swapper);

bool EFX_Swapper::OnInit() {
	MainFrame* mainFrame = new MainFrame("EFX Editor");
	mainFrame->Show();
	mainFrame->SetClientSize(1280,720);
	return true;
}
