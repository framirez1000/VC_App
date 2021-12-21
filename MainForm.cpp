
#include "pch.h"
#include "MainForm.h"

System::Void CppCLRWinformsProjekt::MainForm::drawChnlsView(int rows, int columns, System::Object^ Sender, System::EventArgs^ e)
{
	ChnlViewForm^ pChnlViewItem;
	

	if (this->panel1->Controls->Count > 0) {
		this->panel1->Controls->Clear();
		pChnlsViewList.clear();
		pChnlsNames4formulas->Clear();
		DataFile.monitoredChnls.clear();
		m_cmdMsg->ViewActive = false;
		pFreqCmds->clear();
	}
	Windows::Forms::TableLayoutPanel^ chnlFrame;
	int posX = 1, posY = 50,  panelSizeW = 196, panelSizeH = 91, adicX=1, adicY=1;
	for (int i = 0; i < rows; i++) {
		adicX = 1;
		for (int j = 0; j < columns; j++) {
			// Draw channel frames
			chnlFrame = gcnew Windows::Forms::TableLayoutPanel;
			chnlFrame->ColumnCount = 1;
			chnlFrame->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 100)));
			chnlFrame->Location = System::Drawing::Point(posX + (adicX + panelSizeW) * j, posY+ i * (panelSizeH + adicY));
			chnlFrame->Name = L"tableLayoutPanel" + i;
			chnlFrame->RowCount = 1;
			chnlFrame->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 100)));
			chnlFrame->Size = System::Drawing::Size(panelSizeW+7, panelSizeH+6);
			chnlFrame->CellBorderStyle = System::Windows::Forms::TableLayoutPanelCellBorderStyle::Single;
			Label^ str = gcnew Label;
			str->Text="      ------------------------";
			str->Anchor = System::Windows::Forms::AnchorStyles::None;
			chnlFrame->Controls->Add(str);
			this->panel1->Controls->Add(chnlFrame);
			chnlFrame->Show();
			// Draw channel Views
			pChnlViewItem = gcnew ChnlViewForm(% m_mainDataStruct, %pChnlsViewList, pGroupNames, pChnlsNames4formulas);
			pChnlViewItem->MdiParent = this;
			pChnlViewItem->ChnlName = CHANNEL_VIEW_DEFAULT_NAME;
			pChnlViewItem->OriginalPosX = (posX + 2) + (adicX + panelSizeW) * j;
			pChnlViewItem->OriginalPosY = (posY + 2) + i * (panelSizeH + adicY);
			pChnlViewItem->Location = System::Drawing::Point(pChnlViewItem->OriginalPosX, pChnlViewItem->OriginalPosY);
			pChnlViewItem->Size = System::Drawing::Size(panelSizeW, panelSizeH);
			pChnlViewItem->BackColor = System::Drawing::Color::AliceBlue;
			pChnlViewItem->ChnlCnf->Color = System::Convert::ToString(-984833);
			if ((M_ViewCnfList->Count > 0) && 
				((i * columns + j) < this->M_ViewCnfList->Count) &&
				(M_ViewCnfList[i * columns + j]->Visible)){
				pChnlViewItem->ChnlCnf->cpyData(M_ViewCnfList[i * columns + j]);
				pChnlViewItem->lbl2_NameChnlView->Text = M_ViewCnfList[i * columns + j]->ViewName;
				pChnlViewItem->txtBx1_VoltSPChnlView->Text = pChnlViewItem->ChnlCnf->VoltageSet;
				pChnlViewItem->txtBx2_CurrtSPChnlView->Text = pChnlViewItem->ChnlCnf->CurrentSet;
				pChnlViewItem->lbl_GroupName->Text = M_ViewCnfList[i * columns + j]->Group;
				System::Array^ colorSplit = this->M_ViewCnfList[i * columns + j]->Color->Split(',');
				pChnlViewItem->panel1->BackColor = System::Drawing::Color::FromArgb(System::Convert::ToInt32(this->M_ViewCnfList[i * columns + j]->Color));
				pChnlViewItem->ChnlCnf->ChannelName = M_ViewCnfList[i * columns + j]->ChannelName;
				pChnlViewItem->ChnlName = pChnlViewItem->ChnlCnf->ChannelName;
				pChnlViewItem->ChnlCnf->ViewName = M_ViewCnfList[i * columns + j]->ViewName;
				pChnlViewItem->ChnlCnf->Row = i;
				pChnlViewItem->ChnlCnf->Col = j;
				pChnlViewItem->ModDir = pChnlViewItem->ChnlCnf->ModDir;
				
				if (pChnlViewItem->ChnlCnf->Visible) {
					ControlExtension::Draggable(pChnlViewItem, true);
					// Insert channel in available channels list for recording data
					DataFile.monitoredChnls.insert(CheckedList::make_value(pChnlViewItem->lbl2_NameChnlView->Text, false));
					// Insert channel in frequent channels list to get its reading regurlaly
					// If there is/isn't hardware populate the most freq cmds List for regular scan readings
					if (((m_mainDataStruct.ptrMainCrateList->Count > 0)
							&& m_mainDataStruct.GetChnlConnectedStatus(pChnlViewItem->ChnlCnf->ChannelName)
							&& m_mainDataStruct.GetChnlEnableStatus(pChnlViewItem->ChnlCnf->ChannelName)
							|| (m_mainDataStruct.ptrMainCrateList->Count == 0)))
					{
						String^ cmd = pChnlViewItem->ChnlCnf->ChannelName; 
						RegCmdStruct_T^ item = gcnew RegCmdStruct_T;
						item->row = pChnlViewItem->ChnlCnf->Row;
						item->col = pChnlViewItem->ChnlCnf->Col;
						if ((m_mainDataStruct.ptrMainCrateList->Count > 0) &&
							(m_mainDataStruct.GetChnlEnableStatus(pChnlViewItem->ChnlCnf->ChannelName))){
							item->scanned = true;
							m_cmdMsg->ViewActive = true;
						}
						pChnlViewItem->txtBx1_VoltSPChnlView->Text = pChnlViewItem->ChnlCnf->VoltageSet;
						pChnlViewItem->txtBx2_CurrtSPChnlView->Text = pChnlViewItem->ChnlCnf->CurrentSet;
						pFreqCmds->insert(pFreqCmds->begin(), FreqCmdsMapTable_T::make_value(cmd, item));
					}
				}
			}
			else {
					pChnlViewItem->ChnlCnf->ViewName = "-----";
				}
			String^ chnlVisibleName = gcnew String(pChnlViewItem->ChnlCnf->ViewName);
			pChnlsNames4formulas->Add(chnlVisibleName);
			String^ chnlVisibleNameU = gcnew String(pChnlViewItem->ChnlCnf->ViewName + "_U");
			pChnlsNames4formulas->Add(chnlVisibleNameU);
			
			this->panel1->Controls->Add(pChnlViewItem);
			if (pChnlViewItem->ChnlCnf->Visible){
				if (pChnlViewItem->ChnlCnf->UseVoltageFormula)
					pChnlViewItem->txtBx1_VoltSPChnlView->Enabled = false;
				pChnlViewItem->BringToFront();
				pChnlViewItem->Show();
			}
			int n = pChnlsViewList.size();
			int m = pChnlsViewList.capacity();
			pChnlsViewList.push_back(pChnlViewItem);
			adicX = 6;
		}
		adicY = 5;
	}
	// Iterate through ViewList for setting formaulaRamp calculation
	for each (ChnlViewForm^ item in pChnlsViewList) {
		if (item->ChnlCnf->UseVoltageFormula) {
			item->GetformEvalAtRamp(item->ChnlCnf->VoltageFormula);
		}
	}
	this->ActiveControl = this->panel1;
	return System::Void();
}

System::Boolean CppCLRWinformsProjekt::MainForm::CreateServerPipes()
{
	HANDLE hPipe;
	
	hPipe = CreateNamedPipe(PIPE_TESTING_GUI_LIFE,
		PIPE_ACCESS_DUPLEX, // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
		PIPE_TYPE_MESSAGE | PIPE_ACCESS_INBOUND | PIPE_NOWAIT,
		PIPE_UNLIMITED_INSTANCES,
		1024 * 16,
		1024 * 16,
		0,
		NULL);
	Pipe_GuiAlive = hPipe;
	
	HANDLE hPipe2 = CreateNamedPipe(PIPE_DATA_FILE,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_ACCESS_OUTBOUND | PIPE_NOWAIT,
		PIPE_UNLIMITED_INSTANCES,
		1024 * 16,
		1024 * 16,
		0,
		NULL);
	Pipe4Data2File = hPipe2;

	return (hPipe != INVALID_HANDLE_VALUE);
}

System::Boolean CppCLRWinformsProjekt::MainForm::checkPipe(HANDLE pipe)
{
	int result, error;
	char buffer[1024];
	DWORD dwRead;

	if (!PipeConnectionIO && !(result = ConnectNamedPipe(pipe, NULL))) {
		error = GetLastError();
		if (error == ERROR_PIPE_CONNECTED)
			result = 0;
		else if (error == ERROR_IO_PENDING)
			result = 1;
		PipeConnectionIO = true; // May be connection cause client can connect before this
	}
	else {
		try {
			result = ReadFile(pipe, buffer, sizeof(buffer) - 1, &dwRead, NULL);
			String^ str = gcnew String(buffer);
			Console::ForegroundColor = ConsoleColor::White;
			if (str->Contains("TEST")) {
				m_cmdMsg->statusBarMsg2 = "Comm GUI -> IO... Ok ";
				Console::WriteLine(m_cmdMsg->statusBarMsg2);
			}
			else {
				m_cmdMsg->statusBarMsg2 = "Server received from Pipe: " + str;
				Console::WriteLine(m_cmdMsg->statusBarMsg2);
			}
		}
		catch (Exception^ e) {

		}
	}
	
		
	return System::Boolean();
}

System::Void CppCLRWinformsProjekt::MainForm::SendData2Pipe(HANDLE Pipe, String^ Data)
{
	char buffer[1024*16];
	DWORD dwWritten=0;
	m_cmdMsg->statusBarMsg2 = "Sending data to record";
	Console::WriteLine(m_cmdMsg->statusBarMsg2);
	IntPtr ip = Marshal::StringToHGlobalAnsi(Data);
	const char* stream = static_cast<const char*>(ip.ToPointer());
	if ((Pipe != INVALID_HANDLE_VALUE)) {
		try {
			WriteFile(Pipe,
				stream,
				Data->Length + 1,   // = length of string + terminating '\0' !!!
				&dwWritten,
				NULL);
		}
		catch (Exception^ e) {
		}
		if (dwWritten == 0)
			dwWritten = 0;
		//CloseHandle(Pipe);  // Closed by MainForm
	}
	Marshal::FreeHGlobal(ip);
	return System::Void();
}

System::Boolean CppCLRWinformsProjekt::MainForm::ValidateVoltgReading(String ^voltMeas, String^ voltSP)
{
	Double val;
	if (Double::TryParse(voltMeas, val)) {
		Double val2;
		if (Double::TryParse(voltSP, val2)) {
			val2 = abs(val2);
			val = abs(abs(val) - val2);
			if (val >= (val2 * this->PercentageDeviationAllow) + this->VoltgDeviationAllow)
				return true;
		}
	}
	return false;
}
