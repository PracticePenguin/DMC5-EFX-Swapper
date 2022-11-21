#pragma once
#include<wx/wx.h>
#include <wx/listctrl.h>
#include "../InternalCompute/FileManager.h"
#include <format>

class MainFrame : public wxFrame{
	//Attr
	wxPanel* leftpanel;
	wxPanel* rightpanel;
	wxListView* leftListView;
	wxListView* rightListView;
	wxListView* bleftListView;
	wxListView* brightListView;
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
	void onInstallSegment(wxCommandEvent& evt);
	void onRemoveSegment(wxCommandEvent& evt);
	void onleftListSelected(wxListEvent& event);
	void onrightListSelected(wxListEvent& event);
	//panel items
	void createLPanelItems();
	void createRPanelItems();
	//helper
	void insertEfxList(const Effect& effect, wxListView* listview, uint32_t data, uint32_t id);
	void insertSegmentList(const Segment& segment, wxListView* listview, uint32_t data, uint32_t id);
public:
	//constructor
	MainFrame(const wxString& title);

};
