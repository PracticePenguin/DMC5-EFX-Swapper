#pragma once
#include<wx/wx.h>
#include <wx/listctrl.h>
#include "../InternalCompute/FileManager.h"
#include <format>

class MainFrame : public wxFrame{
	//Attr
	wxPanel* rightpanel;
	wxPanel* leftpanel;
	wxPanel* bottompanel;
	wxListView* leftListView;
	wxListView* rightListView;
	wxListView* bottomListView;
	wxStaticText* rtitle;
	wxStaticText* ltitle;
	std::unique_ptr<FileManager> originFileManager{};
	std::unique_ptr<FileManager> targetFileManager{};
	//create menu bar
	void createMenu();
	//create Panels
	void createPanels();
	//event methods
	void onOpenLibraryFile(wxCommandEvent& evt);
	void onOpenTargetFile(wxCommandEvent& evt);
	void onInstallClicked(wxCommandEvent& evt);
	void onRemoveClicked(wxCommandEvent& evt);
	void onSaveClicked(wxCommandEvent& evt);
	//panel items
	void createLPanelItems();
	void createRPanelItems();
	//helper
	void insertEfxList(const Effect& effect, wxListView* listview, uint32_t data, uint32_t id);
public:
	//constructor
	MainFrame(const wxString& title);

};
