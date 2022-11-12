#include "MainFrame.h"


//constructor
MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, "DMC5 EFX Editor") {
	createMenu();
	createPanels();
	createLPanelItems();
}

void MainFrame::createMenu() {
	wxMenuBar* menu = new wxMenuBar();
	wxMenu* fileMenu = new wxMenu();
	auto mOpenButton = fileMenu->Append(wxID_OPEN);
	auto mSaveButton = fileMenu->Append(wxID_SAVE);
	menu->Append(fileMenu, "File");
	SetMenuBar(menu);
	//bind events
	this->Bind(wxEVT_MENU, &MainFrame::onOpenMenuFile, this, wxID_OPEN);
}

void MainFrame::createPanels() {
	//panel
	wxPanel* leftpanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100, 200));
	wxPanel* rightpanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100, 200));
	wxPanel* bottompanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100, 200));
	//color debug only
	rightpanel->SetBackgroundColour(wxColor(100, 100, 200));
	bottompanel->SetBackgroundColour(wxColor(100, 200, 200));
	//leftpanel->SetBackgroundColour(wxColor(200, 100, 200));
	//make panel global
	this->rightpanel = rightpanel;
	this->bottompanel = bottompanel;
	this->leftpanel = leftpanel;
	//sizer
	wxBoxSizer* tbsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* tsizer = new wxBoxSizer(wxHORIZONTAL);
	//top bottom sizer
	tbsizer->Add(tsizer, 1, wxEXPAND, 3);
	tbsizer->Add(bottompanel, 1, wxEXPAND | wxALL, 3);
	//top l/r sizer
	tsizer->Add(leftpanel, 1, wxEXPAND, 3);
	tsizer->Add(rightpanel, 1, wxEXPAND | wxLEFT, 3);
	//append to main window
	SetSizerAndFit(tbsizer);
}

//left Panel Items
void MainFrame::createLPanelItems() {
	//title
	wxStaticText* title = new wxStaticText(leftpanel, wxID_ANY, "Effects Library");
	//list
	leftListView = new wxListView(leftpanel);
	leftListView->AppendColumn("Name");
	//list bindings
	
	//buttons
	wxButton* install = new wxButton(leftpanel, wxID_ANY, "Install");
	//button bindings
	
	//set sizer
	wxBoxSizer* lsizer = new wxBoxSizer(wxVERTICAL);
	lsizer->Add(title, 1, wxEXPAND, 0);
	lsizer->Add(leftListView, 22, wxEXPAND | wxALL, 5);
	lsizer->Add(install, 1, wxALIGN_CENTER, 0);
	leftpanel->SetSizerAndFit(lsizer);
}

void MainFrame::onOpenMenuFile(wxCommandEvent& evt) {
	
}

