#pragma once
#include<wx/wx.h>
#include <wx/listctrl.h>

class MainFrame : public wxFrame{
	//Attr
	wxPanel* rightpanel;
	wxPanel* leftpanel;
	wxPanel* bottompanel;
	wxListView* leftListView;
	//create menu bar
	void createMenu();
	//create Panels
	void createPanels();
	//event methods
	void onOpenMenuFile(wxCommandEvent& evt);
	//left panel items
	void createLPanelItems();
public:
	//constructor
	MainFrame(const wxString& title);

};
