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
	//color debug only
	//leftpanel->SetBackgroundColour(wxColor(100, 100, 200));
	//rightpanel->SetBackgroundColour(wxColor(200, 100, 200));
	//make panel global
	this->rightpanel = rightpanel;
	this->leftpanel = leftpanel;
	//sizer
	wxBoxSizer* lrsizer = new wxBoxSizer(wxHORIZONTAL);
	//l/r sizer
	lrsizer->Add(leftpanel, 1, wxEXPAND, 3);
	lrsizer->Add(rightpanel, 1, wxEXPAND | wxLEFT, 3);
	//append to main window
	SetSizerAndFit(lrsizer);
}

//left Panel Items
void MainFrame::createLPanelItems() {
	//title
	ltitle = new wxStaticText(leftpanel, wxID_ANY, "Effects Library");
	//list
	leftListView = new wxListView(leftpanel, wxID_ANY);
	leftListView->AppendColumn("Name");
	leftListView->SetColumnWidth(0, 200);
	leftListView->AppendColumn("Size");
	leftListView->SetColumnWidth(1, 150);
	//list bindings 
	leftListView->Bind(wxEVT_LIST_ITEM_SELECTED, &MainFrame::onleftListSelected, this);
	//buttons
	wxButton* install = new wxButton(leftpanel, wxID_ANY, "Add Effect");
	//button bindings
	install->Bind(wxEVT_BUTTON, &MainFrame::onInstallClicked, this);
	//set sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(ltitle, 1, wxEXPAND | wxLEFT, 5);
	sizer->Add(leftListView, 22, wxEXPAND | wxALL, 5);
	sizer->Add(install, 1, wxALIGN_CENTER, 0);
	//Segment section
	//title
	wxStaticText* segTitle = new wxStaticText(leftpanel, wxID_ANY, "Library Segments");
	//list
	bleftListView = new wxListView(leftpanel, wxID_ANY);
	bleftListView->AppendColumn("Name");
	bleftListView->SetColumnWidth(0, 200);
	bleftListView->AppendColumn("Size");
	bleftListView->SetColumnWidth(1, 150);
	//buttons
	wxButton* binstall = new wxButton(leftpanel, wxID_ANY, "Add Segment");
	//button bindings
	binstall->Bind(wxEVT_BUTTON, &MainFrame::onInstallSegment, this);
	//set sizer
	sizer->Add(segTitle, 1, wxEXPAND | wxLEFT, 5);
	sizer->Add(bleftListView, 22, wxEXPAND | wxALL, 5);
	sizer->Add(binstall, 1, wxALIGN_CENTER, 0);
	leftpanel->SetSizerAndFit(sizer);
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
	//list bindings 
	rightListView->Bind(wxEVT_LIST_ITEM_SELECTED, &MainFrame::onrightListSelected, this);
	//buttons
	wxButton* remove = new wxButton(rightpanel, wxID_ANY, "Remove Effect");
	wxButton* save = new wxButton(rightpanel, wxID_ANY, "Save File");
	//button bindings
	remove->Bind(wxEVT_BUTTON, &MainFrame::onRemoveClicked, this);
	save->Bind(wxEVT_BUTTON, &MainFrame::onSaveClicked, this);
	//set sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(rtitle, 1, wxEXPAND | wxLEFT, 5);
	sizer->Add(rightListView, 22, wxEXPAND | wxALL, 5);
	wxBoxSizer* rButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	rButtonSizer->Add(remove, 1, wxALIGN_CENTER, 0);
	rButtonSizer->Add(save, 1, wxALIGN_CENTER, 0);
	sizer->Add(rButtonSizer, 1, wxALIGN_CENTER, 0);
	//segments
	//title
	wxStaticText* segTitle = new wxStaticText(rightpanel, wxID_ANY, "Target Segments");
	//list
	brightListView = new wxListView(rightpanel);
	brightListView->AppendColumn("Name");
	brightListView->SetColumnWidth(0, 200);
	brightListView->AppendColumn("Size");
	brightListView->SetColumnWidth(1, 150);
	//buttons
	wxButton* segRemove = new wxButton(rightpanel, wxID_ANY, "Remove Segment");
	//button bindings
	segRemove->Bind(wxEVT_BUTTON, &MainFrame::onRemoveSegment, this);
	//sizer
	sizer->Add(segTitle, 1, wxEXPAND | wxLEFT, 5);
	sizer->Add(brightListView, 22, wxEXPAND | wxALL, 5);
	sizer->Add(segRemove, 1, wxALIGN_CENTER, 0);
	rightpanel->SetSizerAndFit(sizer);
}

//open library file
void MainFrame::onOpenLibraryFile(wxCommandEvent& evt) {
	wxFileDialog fileDialog = new wxFileDialog(this, "Open Library File", "", "", "", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	fileDialog.SetWildcard("EFX files (*.efx.*)|*.efx.*");
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
	wxFileDialog fileDialog = new wxFileDialog(this, "Open Library File", "", "", "", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	fileDialog.SetWildcard("EFX files (*.efx.*)|*.efx.*");
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

void MainFrame::onleftListSelected(wxListEvent& event){
	bleftListView->DeleteAllItems();
	auto key = event.GetData();
	//populate segments
	for (const auto& segPair : originFileManager->getEffects().at(key).segments) {
		insertSegmentList(segPair.second, bleftListView, segPair.second.id, bleftListView->GetItemCount());
	}
}

void MainFrame::onrightListSelected(wxListEvent& event) {
	brightListView->DeleteAllItems();
	auto key = event.GetData();
	//populate segments
	for (const auto& segPair : targetFileManager->getEffects().at(key).segments) {
		insertSegmentList(segPair.second, brightListView, segPair.second.id, brightListView->GetItemCount());
	}
}

void MainFrame::onInstallSegment(wxCommandEvent& evt) {
	int selCount = bleftListView->GetSelectedItemCount();
	int rselCount = rightListView->GetSelectedItemCount();
	if (selCount <= 0 || rselCount <= 0) {
		wxLogError("Please select both Items");
		return;
	}
	if (!originFileManager || !targetFileManager) {
		wxLogError("Please open both Files");
		return;
	}
	//first item
	auto keyefx = leftListView->GetItemData(leftListView->GetFirstSelected());
	auto targetkeyefx = rightListView->GetItemData(rightListView->GetFirstSelected());
	auto keyseg = bleftListView->GetItemData(bleftListView->GetFirstSelected());
	auto segment = originFileManager->getEffects().at(keyefx).segments.at(keyseg);
	auto newindex = brightListView->GetItemCount();
	//insert first item into targetlists
	segment.id = 0;
	while (targetFileManager->getEffects().at(targetkeyefx).segments.contains(segment.id)) {
		//change id/key until unique
		segment.id++;
	}
	targetFileManager->getEffects().at(targetkeyefx).segments.emplace(std::make_pair(segment.id, segment));
	insertSegmentList(segment, brightListView, segment.id, newindex);
}

void MainFrame::onRemoveSegment(wxCommandEvent& evt) {
	int selCount = brightListView->GetSelectedItemCount();
	int refxselCount = rightListView->GetSelectedItemCount();
	if (selCount <= 0 ||refxselCount <= 0) {
		wxLogError("Please select an Item");
		return;
	}
	if (!targetFileManager) {
		wxLogError("Target File empty!");
		return;
	}
	//first item
	auto efxkey = rightListView->GetItemData(rightListView->GetFirstSelected());
	auto segkey = brightListView->GetItemData(brightListView->GetFirstSelected());
	brightListView->DeleteItem(brightListView->GetFirstSelected());
	targetFileManager->getEffects().at(efxkey).segments.erase(segkey);
}

void MainFrame::insertSegmentList(const Segment& segment, wxListView* listview, uint32_t data, uint32_t index) {
	listview->InsertItem(index, segment.segName);
	listview->SetItem(index, 1, std::format("{:#x}", segment.size));
	listview->SetItemData(index, data);
}

