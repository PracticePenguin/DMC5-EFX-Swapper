#include "MainFrame.h"


//constructor
MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, "DMC5 EFX Editor") {
	createMenu();
	createPanels();
	createLPanelItems();
	createRPanelItems();
}

void MainFrame::createMenu() {
	wxMenuBar* menu = new wxMenuBar();
	wxMenu* fileMenu = new wxMenu();
	auto mOpenButton = fileMenu->Append(wxID_ANY, "&Open Library");
	auto mOpenTButton = fileMenu->Append(wxID_ANY, "&Open TargetFile");
	auto mSaveButton = fileMenu->Append(wxID_SAVE);
	menu->Append(fileMenu, "File");
	SetMenuBar(menu);
	//bind events
	this->Bind(wxEVT_MENU, &MainFrame::onOpenLibraryFile, this, mOpenButton->GetId());
	this->Bind(wxEVT_MENU, &MainFrame::onOpenTargetFile, this, mOpenTButton->GetId());
}

void MainFrame::createPanels() {
	//panel
	wxPanel* leftpanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100, 200));
	wxPanel* rightpanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100, 200));
	wxPanel* bottompanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100, 200));
	//color debug only
	//rightpanel->SetBackgroundColour(wxColor(100, 100, 200));
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
	ltitle = new wxStaticText(leftpanel, wxID_ANY, "Effects Library");
	//list
	leftListView = new wxListView(leftpanel);
	leftListView->AppendColumn("Name");
	leftListView->SetColumnWidth(0, 200);
	leftListView->AppendColumn("Size");
	leftListView->SetColumnWidth(1, 150);
	//buttons
	wxButton* install = new wxButton(leftpanel, wxID_ANY, "Add");
	//button bindings
	install->Bind(wxEVT_BUTTON, &MainFrame::onInstallClicked, this);
	//set sizer
	wxBoxSizer* lsizer = new wxBoxSizer(wxVERTICAL);
	lsizer->Add(ltitle, 1, wxEXPAND | wxLEFT, 5);
	lsizer->Add(leftListView, 22, wxEXPAND | wxALL, 5);
	lsizer->Add(install, 1, wxALIGN_CENTER, 0);
	leftpanel->SetSizerAndFit(lsizer);
}

//right panel items
void MainFrame::createRPanelItems() {
	//title
	rtitle = new wxStaticText(rightpanel, wxID_ANY, "Installed Effects");
	//list
	rightListView = new wxListView(rightpanel);
	rightListView->AppendColumn("Name");
	rightListView->SetColumnWidth(0, 200);
	rightListView->AppendColumn("Size");
	rightListView->SetColumnWidth(1, 150);
	//buttons
	wxButton* remove = new wxButton(rightpanel, wxID_ANY, "Remove");
	wxButton* save = new wxButton(rightpanel, wxID_ANY, "Save File");
	//button bindings
	remove->Bind(wxEVT_BUTTON, &MainFrame::onRemoveClicked, this);
	save->Bind(wxEVT_BUTTON, &MainFrame::onSaveClicked, this);
	//set sizer
	wxBoxSizer* rsizer = new wxBoxSizer(wxVERTICAL);
	rsizer->Add(rtitle, 1, wxEXPAND | wxLEFT, 5);
	rsizer->Add(rightListView, 22, wxEXPAND | wxALL, 5);
	wxBoxSizer* rButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	rButtonSizer->Add(remove, 1, wxALIGN_CENTER, 0);
	rButtonSizer->Add(save, 1, wxALIGN_CENTER, 0);
	rsizer->Add(rButtonSizer, 1, wxALIGN_CENTER, 0);
	rightpanel->SetSizerAndFit(rsizer);
}

//open library file
void MainFrame::onOpenLibraryFile(wxCommandEvent& evt) {
	wxFileDialog fileDialog = new wxFileDialog(this, "Open Library File", "", "", "Efx Files (*.efx.*) | *.efx.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	//if cancel
	if (fileDialog.ShowModal() == wxID_CANCEL) {
		return;
	}
	//open file
	if (true) {
		originFileManager = std::make_unique<FileManager>(std::string(fileDialog.GetPath().mb_str()));
	}
	//read data
	bool success = originFileManager->openAndReadFile();	
	if (!success) {
		wxLogError("File contains unsupported Segments");
		return;
	}
	//populate library
	leftListView->DeleteAllItems();
	for (const auto& effect : originFileManager->getEffects()) {
		insertEfxList(effect.second, leftListView, effect.second.id, effect.second.id);
	}
	//change title
	ltitle->SetLabel("Effects Library: " + fileDialog.GetFilename());
}

void MainFrame::onOpenTargetFile(wxCommandEvent& evt) {
	wxFileDialog fileDialog = new wxFileDialog(this, "Open Library File", "", "", "Efx Files (*.efx.*) | *.efx.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	//if cancel
	if (fileDialog.ShowModal() == wxID_CANCEL) {
		return;
	}
	//open file
	if (true) {
		targetFileManager = std::make_unique<FileManager>(std::string(fileDialog.GetPath().mb_str()));
	}
	//read data
	bool success = targetFileManager->openAndReadFile();
	if (!success) {
		wxLogError("File contains unsupported Segments");
		return;
	}
	//populate library
	rightListView->DeleteAllItems();
	for (const auto& effect : targetFileManager->getEffects()) {
		insertEfxList(effect.second, rightListView, effect.second.id, effect.second.id);
	}
	//change title
	rtitle->SetLabel("Installed Effects: " + fileDialog.GetFilename());
}

void MainFrame::onInstallClicked(wxCommandEvent& evt) {
	int selCount = leftListView->GetSelectedItemCount();
	if (selCount <= 0) {
		wxLogError("Please select an Item");
		return;
	}
	if (!originFileManager || !targetFileManager) {
		wxLogError("Please open both Files");
		return;
	}
	//first item
	auto data = leftListView->GetItemData(leftListView->GetFirstSelected());
	auto efx = originFileManager->getEffects().at(static_cast<int>(data));
	auto newindex = rightListView->GetItemCount();
	//insert first item into targetlists
	efx.id = 0;
	while (targetFileManager->getEffects().contains(efx.id)) {
		//change id/key until unique
		efx.id++;
	}
	targetFileManager->getEffects().emplace(std::make_pair(efx.id, efx));
	targetFileManager->getHeader().effectCount++;
	insertEfxList(efx, rightListView, efx.id, newindex);
}

void MainFrame::onRemoveClicked(wxCommandEvent& evt) {
	int selCount = rightListView->GetSelectedItemCount();
	if (selCount <= 0) {
		wxLogError("Please select an Item");
		return;
	}
	if (!targetFileManager) {
		wxLogError("Target File empty!");
		return;
	}
	//first item
	auto firstitem = rightListView->GetFirstSelected();
	auto data = rightListView->GetItemData(firstitem);
	rightListView->DeleteItem(rightListView->GetFirstSelected());
	targetFileManager->getEffects().erase(static_cast<int>(data));
	targetFileManager->getHeader().effectCount--;
}

void MainFrame::onSaveClicked(wxCommandEvent& evt) {
	//write target into file 
	if (!targetFileManager) {
		wxLogError("Please open a Target File");
		return;
	}
	targetFileManager->saveFile();
}

void MainFrame::insertEfxList(const Effect& effect, wxListView* listview, uint32_t data, uint32_t index) {
	listview->InsertItem(index, effect.name);
	listview->SetItem(index, 1, std::format("{:#x}", effect.size));
	listview->SetItemData(index, data);
}

