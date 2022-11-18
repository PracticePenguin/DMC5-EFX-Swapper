#pragma once
#include<wx/wx.h>
#include <wx/listctrl.h>
#include "../InternalCompute/FileManager.h"

class MainFrame : public wxFrame{
	//Attr
	wxPanel* rightpanel;
	wxPanel* leftpanel;
	wxPanel* bottompanel;
	wxListView* leftListView;
	wxListView* rightListView;
	std::unique_ptr<FileManager> fileManager;
	//create menu bar
	void createMenu();
	//create Panels
	void createPanels();
	//event methods
	void onOpenLibraryFile(wxCommandEvent& evt);
	//panel items
	void createLPanelItems();
	void createRPanelItems();
public:
	//constructor
	MainFrame(const wxString& title);

};
