#include "EFX_Editor.h"
#include "gui/MainFrame.h"

wxIMPLEMENT_APP(EFX_Editor);

bool EFX_Editor::OnInit() {
	MainFrame* mainFrame = new MainFrame("EFX Editor");
	mainFrame->Show();
	mainFrame->SetClientSize(1280,720);
	return true;
}
