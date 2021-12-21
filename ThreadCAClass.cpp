#pragma once
#include "pch.h"
#include "ThreadCAClass.h"

/* Thread entry point fuction */
void Thread_CA::ThreadCAClass::ThreadCaEntryPoint()
{
	Console::ForegroundColor = ConsoleColor::White;
	Console::WriteLine("Thread Comm IO created");
	//throw gcnew System::NotImplementedException();
	double doubArray1[MAX_VIEW_CHANNELS];
	double doubArray2[MAX_VIEW_CHANNELS];
	char isOnStatus[MAX_VIEW_CHANNELS][40];
	char isVoltageRamp[MAX_VIEW_CHANNELS][40];
	char isEmergency[MAX_VIEW_CHANNELS][40];
	char isVoltageBound[MAX_VIEW_CHANNELS][40];
	char isTrip[MAX_VIEW_CHANNELS][40];
	char isVoltageLimit[MAX_VIEW_CHANNELS][40];
	char isCC[MAX_VIEW_CHANNELS][40];
	char isCV[MAX_VIEW_CHANNELS][40];
	char isCurrentBound[MAX_VIEW_CHANNELS][40];
	char isExternalInhibit[MAX_VIEW_CHANNELS][40];
	char isCurrentLimit[MAX_VIEW_CHANNELS][40];
	int iValue;
	int commFailTime = 0;
	int sleepTimeScan = SAMPLE_TIME_mSEC;
	bool commFailure = false;
	List<String^>^ cmmFailCrates = gcnew List<String^>;
	// time variables
	int time2TestGUIlife = 0, GUI_timeDown = 0, checkCommTime = 0, check1sec = SAMPLE_TIME_mSEC;
	double f;
	bool stop_accessIO = false, GUIPipeConnected = false, hardwareConnected = false;
	ini_Map();
	GUIPipeConnected=CreateNamedPipeClient();
	//Commands->cmdType = 1;
	Commands->deviceCmd = "ISEG:5230225";
	String^ str = gcnew String("");
	Globals::globalVar = L"From Thread: started .... and sleeping for 20 sec ..";

	//System::Threading::Thread::Sleep(20000);
	double* n = nullptr;
	if (valdDouble != nullptr) {
		double% x = *valdDouble;
		pin_ptr<double> pp = &x;
		n = pp;
	}
	// testing adding crate at startup
	Commands->deviceCmd = "ISEG:5230225";
	// End test adding crate at startup
	checkCommTime = (60 * 1000 / sleepTimeScan) * CHECK_COMM_IO_TIME_MIN - 10;
	System::Threading::Thread::Sleep(sleepTimeScan);
	while (!stop_accessIO)
	{
		System::Threading::Thread::Sleep(sleepTimeScan);
		// Iterate MainCrateList to Check if any crate connected every X min for 
		// keeping Channels open
		if (checkCommTime++ >= (60 * 1000 / sleepTimeScan) * CHECK_COMM_IO_TIME_MIN) {
			// For now just check the working crate but must be every crate in MainCrateList
			DateTime cpCurrentDateTime = DateTime::Now;
			String^ strTemp = cpCurrentDateTime.ToShortDateString();
			strTemp = String::Concat(strTemp, " ");
			strTemp = String::Concat(strTemp, cpCurrentDateTime.ToLongTimeString());
			Console::ForegroundColor = ConsoleColor::White;
			Console::Write(strTemp + " ");
			checkCommTime = 0;
			if (!hardwareConnected) {
				Console::ForegroundColor = ConsoleColor::DarkYellow;
				Console::WriteLine("Not Comm IO ->.... ISEG:5230225");
			}
		}
		// Check for new commands from USER (later implementation: namedPipes)
		if (((Commands != nullptr) && (Commands->execRequest) && (!commFailure)) || (Commands->cmdType == DISCONNECT)) {
			// Call function according to command type
			Commands->progressRate = (Commands->strCmdsToExcList->Count > 0) ? Commands->strCmdsToExcList->Count : 1;
			switch (Commands->cmdType)
			{
			case DETECT_HRW:
				for each (String ^ device in Commands->strCmdsToExcList) {
					if (Commands->cmdResult = CA_Interf->DetectHrw(device)) {
						// Add it to Hrw List (Map(crate, connState))
						pMainHrwList->insert(CheckedList::make_value(device, false));
					}
				}
				Commands->cmdExecuted = true;
				break;
			case CHECK_CRATE:
				// Call check existance and base parameters
				CA_Interf->test_Crate_Comm(Commands->deviceCmd);
				Commands->cmdExecuted = true;
				break;
			case ADD_CRATE:
				for each (String ^ crate in Commands->strCmdsToExcList) {
					if (CA_Interf->Add_Crate(crate)) {
						// Set its connected state
						CheckedList::const_iterator it = pMainHrwList->find(crate);
						CheckedList::const_reference cref = *it;
						cref->second = true;
					}
				}
				if (m_pCrateList->Count > 0) {
					hardwareConnected = true;
					Commands->HrwConntd = true;
				}
				else Commands->statusBarMsg = "Not hardware found";
				Commands->cmdExecuted = true;
				break;
			case SET_CONF_VALUE: {
				// Go through cmmdsToExc list and send them
				String^ cmd;
				String^ cmd2;
				bool result;
				int i = 0;
				static char strCharCmd[_MAX_PATH];
				IntPtr ip;
				for each (cmd in Commands->strCmdsToExcList) {
					ip = Marshal::StringToHGlobalAnsi(cmd);
					if (i != (Commands->strCmdsToExcList->Count) - 1)
						m_ptrCA_Interface->setCAValue(cmd, Commands->strParamsList[i], false);
					else {
						result = m_ptrCA_Interface->setCAValue(cmd, Commands->strParamsList[i], true);
						if (result == E_OK) {
							Commands->cmdResult = true;
							Console::WriteLine("All cmds sent successfully ");
						}
					}
					i++;
				}
				UpdateLists();
				
				Marshal::FreeHGlobal(ip);
				UpdateLists();
				Commands->execRequest = false;
				Commands->cmdExecuted = true;
				break;
			}
			case CHANNELS_TO_ON_OFF:{
				// iterate through all cmd list, send camds 0ne by one and with 
				// final cmd set sendOrder to true for sending all cmds on hold
				String^ cmd;
				IntPtr ip;
				int i = 0, result;
				bool update = true;
				for each (cmd in Commands->strCmdsToExcList) {
					ip = Marshal::StringToHGlobalAnsi(cmd);
					if (i != (Commands->strCmdsToExcList->Count) - 1)
						m_ptrCA_Interface->setCAValue(cmd, Commands->strParamsList[i], false);
					else {
						result = m_ptrCA_Interface->setCAValue(cmd, Commands->strParamsList[i], true);
						if (result == E_OK) {
							Console::WriteLine("All cmds sent successfully ");
						}
					}
					i++;
					if (cmd->Contains(":Control:setOn"))
						update = false;
				}
				if (update) 
					UpdateLists();
				Marshal::FreeHGlobal(ip);
				Commands->cmdExecuted = true;
				break;
			}
			case DISCONNECT:
				commFailure = false;
				commFailTime = 0;
				sleepTimeScan = SAMPLE_TIME_mSEC;
				Commands->execRequest = false;
				Commands->cmdExecuted = true;
				cmmFailCrates->Clear();
				Commands->CleanCmdsLists();
				break;
			case STOP:
				// Stop iterating, close all channels IO access (epics)
				stop_accessIO = true;
				break;
			default:
				Commands->execRequest = false;
				break;
			}
			Commands->barProgressValue = 100;
			Commands->execRequest = false;
		}
		else 
			if (Commands->execRequest) {
				Commands->execRequest = false;
				Commands->cmdExecuted = true;
				Commands->CleanCmdsLists();
				Commands->StatusBarMsgIndex = 26;
				//Commands->statusBarMsg = "Not executed, not connection";
				if (Commands->cmdType == DISCONNECT) 
					commFailure = false;
			}
		if (hardwareConnected) {
			if (Commands->ViewActive) {
				String^ cmdItem;
				int index = 0;
				// if there are hardware comm failure => try to reconnect
				if (commFailure) {
					//Commands->statusBarMsg = COMM_TRY_MSG;
					Commands->StatusBarMsgIndex = 1;
					System::Threading::Thread::Sleep(50);
					for each (CheckedList::value_type crate in pMainHrwList) {
						if (CA_Interf->DetectHrw(crate->first) == ECA_NORMAL) {
							CheckedList::const_iterator it = m_HrdwFailingList->find(crate->first);
							if (it != m_HrdwFailingList->end()) {
								CheckedList::const_reference cref = *it;
								m_HrdwFailingList->erase(crate->first);
							}
							//if (m_HrdwFailingList->Contains(crate->first))
							//	m_HrdwFailingList->Remove((crate->first));
							//	//cmmFailCrates->Remove((crate->first));
						}
					}
					Commands->statusBarMsg = " - Fail(s): " + m_HrdwFailingList->size()
						+ " Total crates(s): " + pMainHrwList->size();
					Commands->StatusBarMsgIndex = 0;
				}
				bool send = false;
				for each (FreqCmdsMapTable_T::value_type elem in pMainFreqCmds) {
					// Check if hardware related to this chnl in connected by comparing 
					// S/N & chnlName included in cmd
					cmdItem = elem->first;
					if ((chnlHrwConnected(elem, m_HrdwFailingList)) && elem->second->scanned) {
						CA_Interf->FreqCmdsMgr(doubArray1, nullptr, nullptr, cmdItem + ":VoltageMeasure", index, false);
						CA_Interf->FreqCmdsMgr(nullptr, doubArray2, nullptr, cmdItem + ":CurrentMeasure", index, false);
						CA_Interf->FreqCmdsMgr(nullptr, nullptr, isOnStatus, cmdItem + ":isOn", index, false);
						CA_Interf->FreqCmdsMgr(nullptr, nullptr, isVoltageRamp, cmdItem + ":isVoltageRamp", index, false);
						CA_Interf->FreqCmdsMgr(nullptr, nullptr, isEmergency, cmdItem + ":isEmergency", index, false);
						CA_Interf->FreqCmdsMgr(nullptr, nullptr, isTrip, cmdItem + ":isTrip", index, false);
						CA_Interf->FreqCmdsMgr(nullptr, nullptr, isVoltageLimit, cmdItem + ":isVoltageLimit", index, false);
						send = true;
					}
					index++;
				}
				if (send && (CA_Interf->FreqCmdsMgr(nullptr, nullptr, nullptr, "", -1, true) != ECA_NORMAL)) {
					if (commFailTime >= COMM_FAILURE_TIME_SEC * (1000/sleepTimeScan)) {
						
						sleepTimeScan += 1000; // Slow down time scan by 1 sec till 10 sec
						if (sleepTimeScan > MAX_SAMPLE_TIME_mSEC_AT_FAILURE)
							sleepTimeScan = MAX_SAMPLE_TIME_mSEC_AT_FAILURE;
						commFailure = true;
						
						for each (CheckedList::value_type crate in pMainHrwList) {
							if (CA_Interf->DetectHrw(crate->first) != ECA_NORMAL) {
								String^ crateName = gcnew String(crate->first);
								m_HrdwFailingList->insert(CheckedList::make_value(crateName, true));
								//if (!m_HrdwFailingList->Contains(crate->first)) {
								//	String^ str = gcnew String(crate->first);
								//	m_HrdwFailingList->Add((str));
								//	//cmmFailCrates->Add(crate->first);
								//}
							}
						}
						Commands->statusBarMsg = " - Fail(s): " + m_HrdwFailingList->size()
													+ " Total crates(s): " + pMainHrwList->size();
						Commands->StatusBarMsgIndex = 0;
					}
					else {
						commFailTime++;
					}
				}
				else if (send && commFailure) {
					sleepTimeScan = SAMPLE_TIME_mSEC;
					commFailTime = 0;
					commFailure = false;
					//Commands->statusBarMsg = COMM_BACK_MSG;
					Commands->StatusBarMsgIndex = 2;
					m_HrdwFailingList->clear();
				} 
				
				// Make/Put returned values available for the View (back in frqCmdList->Item->econd)
				index = 0;
				
				for each (FreqCmdsMapTable_T::value_type elem in pMainFreqCmds) {
					elem->second->vValue = doubArray1[index];
					elem->second->iValue = doubArray2[index];
					String^ strIsOnStatus = gcnew String(isOnStatus[index]);
					elem->second->onStateValue = strIsOnStatus;
					String^ strIsVoltageRamp = gcnew String(isVoltageRamp[index]);
					elem->second->isVoltageRamp = strIsVoltageRamp;
					String^ strIsEmergency = gcnew String(isEmergency[index]);
					elem->second->isEmergency = strIsEmergency;
					String^ strIsTrip = gcnew String(isTrip[index]);
					elem->second->isTrip = strIsTrip;
					String^ strIsVoltageLimit = gcnew String(isVoltageLimit[index]);
					elem->second->isVoltageLimit = strIsVoltageLimit;
					index++;
				}
			}
		}
		Globals::globalVar = "From comm thread to " + crateObject->Name + ":" + crateObject->Address + " : time to die in sec .. " + (20 )
			+ " -> " + (*valdDouble);// +" -> " + (*n);
		
		System::Threading::Monitor::Enter(crateObject);  //EnterCriticalSection()
		crateObject->Status = status;
		
		for (int i = 0; i < MAX_CHANNELS; i++) {
			crateObject->voltageMeasurements[i] = doubArray1[i];
			crateObject->currentMeasurements[i] = doubArray2[i];
			String^ str1 = gcnew String(isOnStatus[i]);
			crateObject->isOnStatus[i] = str1;
		}
				
		System::Threading::Monitor::Exit(crateObject);  // LeaveCriticalSection()
		Globals::globalVar = System::Convert::ToString(crateObject->voltageMeasurements.at(0)) + " vector legth: " + crateObject->voltageMeasurements.size();
		if (time2TestGUIlife++ >= (1000 / SAMPLE_TIME_mSEC) * TIME_2_TEST_GUI_LIFE_SEC) {
			if (check1sec++ >= (1000 / SAMPLE_TIME_mSEC)) {
				Commands->statusBarMsg2 = "Checking Comm IO -> Main GUI... ";
				Console::ForegroundColor = ConsoleColor::White;
				Console::Write(Commands->statusBarMsg2);
				if (!TestPipeConn() && (GUIPipeConnected)) {
					Commands->statusBarMsg2 = "\n Not Comm with IO -> Main GUI";
					Console::ForegroundColor = ConsoleColor::DarkRed;
					Console::WriteLine(Commands->statusBarMsg2);
					if (GUI_timeDown++ >= MAX_GUI_TIME_DOWN_SEC) {
						stop_accessIO = true;
						Console::ForegroundColor = ConsoleColor::Red;
						Console::WriteLine("\n Stopping Thread IO => Not Comm with Main GUI");
					}
				}
				else {
					GUI_timeDown = 0;
					time2TestGUIlife = 0;
					Console::WriteLine("Ok");
				}
				check1sec = 0;
			}
		}
		

	}
}

System::Boolean Thread_CA::ThreadCAClass::CreateNamedPipeClient()
{
	System::Threading::Thread::Sleep(SAMPLE_TIME_mSEC);
	HANDLE pipe = CreateFile(PIPE_TESTING_GUI_LIFE, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	string message = "TEST";
	DWORD numWritten;
	
	if (pipe != INVALID_HANDLE_VALUE) {
		Pipe = pipe;
		try {
			WriteFile(pipe, message.c_str(), message.length(), &numWritten, NULL);
		}
		catch (Exception^ e) {}
	}
	else return (false);
	
	return true;
}

System::Boolean Thread_CA::ThreadCAClass::TestPipeConn()
{
	String^ msg = gcnew String("TEST");
	char buffer[1024];
	DWORD numWritten, dwRead;
	//WriteFile(Pipe, message.c_str(), message.length(), &numWritten, NULL);
	//return( WaitNamedPipe(	Pipe, NMPWAIT_USE_DEFAULT_WAIT));
	//return (ReadFile(Pipe, buffer, sizeof(buffer) - 1, &dwRead, NULL));
	if (Pipe != INVALID_HANDLE_VALUE) {
		bool result;
		try {
			IntPtr ip = Marshal::StringToHGlobalAnsi(msg);
			const char* stream = static_cast<const char*>(ip.ToPointer());
			result = WriteFile(Pipe, stream, msg->Length + 1 , &numWritten, NULL);
			Marshal::FreeHGlobal(ip);
		}
		catch (Exception^ e) {
			Console::ForegroundColor = ConsoleColor::Red;
			Console::WriteLine("Error trying to Write on Main GUI Pipe");
			return false;
		}
		return (result);
	}
	return false;
}

System::Void Thread_CA::ThreadCAClass::UpdateLists()
{
	Commands->doCaSearch = false;
	int j, k;
	for each (Crate ^ crate in CrateList) {
		j = 0;
		Commands->deviceCmd = crate->Name;
		for each (Module ^ mod in crate->m_lstModules) {
			k = 0;
			for each (Channel ^ chnl in mod->lstChannels) {
				m_ptrCA_Interface->getChnlCnf(chnl, int::Parse(crate->Address), j, k);
				k++;
			}
			j++;
		}
	}
	return System::Void();
}

System::Boolean Thread_CA::ThreadCAClass::chnlHrwConnected(FreqCmdsMapTable_T::value_type cmd, CheckedList^ cmmFailCrates)
{
	String^ str = gcnew String(cmd->first);
	for each (CheckedList::value_type crate in pMainHrwList) {
		if (str->Contains(crate->first) && crate->second && (cmmFailCrates->find(crate->first)==cmmFailCrates->end()))
			return true;
	}
	return false;
}

