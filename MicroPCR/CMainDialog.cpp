// CMainDialog.cpp: 구현 파일
//

#include "stdafx.h"
#include "CMainDialog.h"
#include "afxdialogex.h"
#include "resource.h"

#include "PasswordDialog.h"
#include "SetupDialog.h"
#include "FileManager.h"

#include <numeric>

// CMainDialog 대화 상자

IMPLEMENT_DYNAMIC(CMainDialog, CDialogEx)

CMainDialog::CMainDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MAIN, pParent)
	, lastSelectedProtocol(L"")
	, isProtocolLoaded(false)
	, isConnected(false)
	, isStarted(false)
	, magnetoProtocolIdx(0)
	, isFirstDraw(false)
	, isCompletePCR(false)
	, isTargetArrival(false)
	, m_prevTargetTemp(25)
	, m_currentTargetTemp(25)
	, m_kp(0.0)
	, m_ki(0.0)
	, m_kd(0.0)
	, m_cArrivalDelta(0.5)
	, maxCycles(40)
	, compensation(0)
	, integralMax(INTGRALMAX)
	, displayDelta(0.0f)
	, flRelativeMax(FL_RELATIVE_MAX)
	, m_timerCounter(0)
	, totalLeftSec(0)
	, leftSec(0)
	, m_currentActionNumber(-1)
	, targetTempFlag(false)
	, m_timeOut(0)
	, m_leftGotoCount(-1)
	, filterIndex(0)
	, filterRunning(false)
	, shotCounter(0)
	, ledControl_wg(1)
	, ledControl_r(1)
	, ledControl_g(1)
	, ledControl_b(1)
	, emergencyStop(false)
	, freeRunning(false)
	, freeRunningCounter(0)
	, currentCycle(0)
	, recordingCount(0)
	, useFam(false)
	, useHex(false)
	, useRox(false)
	, useCy5(false)
	, logStopped(false)
{

}

CMainDialog::~CMainDialog()
{
	if (device != NULL)
		delete device;
	if (m_Timer != NULL)
		delete m_Timer;
}

void CMainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_PROTOCOLS, protocolList);
	DDX_Control(pDX, IDC_COMBO_DEVICE_LIST, deviceList);
	DDX_Control(pDX, IDC_PROGRESS_STATUS, progressStatus);
	// progressStatus
}


BEGIN_MESSAGE_MAP(CMainDialog, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_START, &CMainDialog::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_SETUP, &CMainDialog::OnBnClickedButtonSetup)
	ON_LBN_SELCHANGE(IDC_COMBO_PROTOCOLS, &CMainDialog::OnLbnSelchangeComboProtocols)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CMainDialog::OnBnClickedButtonConnect)
	ON_WM_TIMER()
	ON_MESSAGE(WM_MMTIMER, OnmmTimer)
	ON_BN_CLICKED(IDC_BUTTON_FILTER_FAM, &CMainDialog::OnBnClickedButtonFilterFam)
	ON_BN_CLICKED(IDC_BUTTON_FILTER_HEX, &CMainDialog::OnBnClickedButtonFilterHex)
	ON_BN_CLICKED(IDC_BUTTON_FILTER_ROX, &CMainDialog::OnBnClickedButtonFilterRox)
	ON_BN_CLICKED(IDC_BUTTON_FILTER_CY5, &CMainDialog::OnBnClickedButtonFilterCy5)
END_MESSAGE_MAP()


// CMainDialog 메시지 처리기


BOOL CMainDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 190318 YJ check the temp file
	int res = FileManager::getTempValue();

	if (res == -1) {
		AfxMessageBox(L"diskOffset.txt is not exist.");
		AfxGetMainWnd()->SendMessage(WM_CLOSE);
	}
	else if (res == 0) {
		AfxMessageBox(L"Invalid data in diskOffset.txt.");
		AfxGetMainWnd()->SendMessage(WM_CLOSE);
	}

	CAxis *axis;
	axis = m_Chart.AddAxis(kLocationBottom);
	axis->m_TitleFont.lfWidth = 20;
	axis->m_TitleFont.lfHeight = 15;
	axis->SetTitle(L"PCR Cycles");
	axis->SetRange(0, maxCycles);
	axis = m_Chart.AddAxis(kLocationLeft);
	axis->m_TitleFont.lfWidth = 20;
	axis->m_TitleFont.lfHeight = 15;
	axis->SetTitle(L"Sensor Value");
	axis->SetRange(0, 1000);

	// Load bitmap
	offImg.LoadBitmapW(IDB_BITMAP_OFF);
	famImg.LoadBitmapW(IDB_BITMAP_FAM);
	hexImg.LoadBitmapW(IDB_BITMAP_HEX);
	roxImg.LoadBitmapW(IDB_BITMAP_ROX);
	cy5Img.LoadBitmapW(IDB_BITMAP_CY5);

	SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_FAM, offImg);
	SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_HEX, offImg);
	SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_ROX, offImg);
	SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_CY5, offImg);

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	SetDlgItemText(IDC_EDIT_CONNECTI_STATUS, L"Disconnected");
	SetDlgItemText(IDC_STATIC_PROGRESS_STATUS, L"Idle..");

	progressStatus.SetRange(0, 100);

	// init chart data
	clearChartValue();

	loadConstants();
	loadProtocolList();

	// Initialize the magneto
	initMagneto();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CMainDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.

	if (IsIconic())
	{
	}
	else {
		CRect graphRect;
		int oldMode = dc.SetMapMode(MM_LOMETRIC);
		graphRect.SetRect(15, 130, 470, 500);

		dc.DPtoLP((LPPOINT)&graphRect, 2);
		CDC *dc2 = CDC::FromHandle(dc.m_hDC);
		m_Chart.OnDraw(dc2, graphRect, graphRect);
		dc.SetMapMode(oldMode);
	}
}

void CMainDialog::OnBnClickedButtonSetup()
{
	CPasswordDialog passwordDialog;

	int res = passwordDialog.DoModal();
	if (res == IDOK) {
		SetupDialog dlg;
		dlg.DoModal();

		// Reload the protocol list
		loadProtocolList();
		loadConstants();
	}
}


BOOL CMainDialog::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
			return TRUE;

		if (pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CMainDialog::loadProtocolList() {
	protocolList.ResetContent();

	FileManager::loadProtocols(protocols);

	int idx = 0;
	for (int i = 0; i < protocols.size(); ++i) {
		protocolList.AddString(protocols[i].protocolName);

		if (lastSelectedProtocol.Compare(protocols[i].protocolName) == 0) {
			idx = i;
		}
	}

	if (protocolList.GetCount() > 0) {
		protocolList.SetCurSel(idx);
		// Load the data from protocol
		currentProtocol = protocols[idx];
		initProtocol();
		isProtocolLoaded = true;
	}
	else {
		AfxMessageBox(L"You need to make the protocol first.");
	}
}

void CMainDialog::initProtocol() {
	useFam = currentProtocol.useFam;
	useHex = currentProtocol.useHex;
	useRox = currentProtocol.useRox;
	useCy5 = currentProtocol.useCY5;

	if (currentProtocol.useFam) {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_FAM, famImg);
		GetDlgItem(IDC_BUTTON_FILTER_FAM)->EnableWindow();
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_FAM, offImg);
		GetDlgItem(IDC_BUTTON_FILTER_FAM)->EnableWindow(FALSE);
	}

	if (currentProtocol.useHex) {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_HEX, hexImg);
		GetDlgItem(IDC_BUTTON_FILTER_HEX)->EnableWindow();
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_HEX, offImg);
		GetDlgItem(IDC_BUTTON_FILTER_HEX)->EnableWindow(FALSE);
	}

	if (currentProtocol.useRox) {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_ROX, roxImg);
		GetDlgItem(IDC_BUTTON_FILTER_ROX)->EnableWindow();
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_ROX, offImg);
		GetDlgItem(IDC_BUTTON_FILTER_ROX)->EnableWindow(FALSE);
	}

	if (currentProtocol.useCY5) {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_CY5, cy5Img);
		GetDlgItem(IDC_BUTTON_FILTER_CY5)->EnableWindow();
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_CY5, offImg);
		GetDlgItem(IDC_BUTTON_FILTER_CY5)->EnableWindow(FALSE);
	}

	calcTotalTime();
}

void CMainDialog::loadConstants() {
	FileManager::loadConstants(maxCycles, compensation, integralMax, displayDelta, flRelativeMax, pids);

	// if there is no pids, make initial pid data
	if (pids.empty()) {
		PID pid1(25.0f, 95.0f, 0.0f, 0.0f, 0.0f);
		PID pid2(95.0f, 60.0f, 0.0f, 0.0f, 0.0f);
		PID pid3(60.0f, 72.0f, 0.0f, 0.0f, 0.0f);
		PID pid4(72.0f, 95.0f, 0.0f, 0.0f, 0.0f);
		PID pid5(95.0f, 50.0f, 0.0f, 0.0f, 0.0f);
		pids.push_back(pid1);
		pids.push_back(pid2);
		pids.push_back(pid3);
		pids.push_back(pid4);
		pids.push_back(pid5);
	}

	CAxis *axis = m_Chart.GetAxisByLocation(kLocationBottom);
	axis->SetRange(0, maxCycles);
	InvalidateRect(&CRect(15, 130, 470, 500));
}

void CMainDialog::OnLbnSelchangeComboProtocols() {
	int selectedIdx = protocolList.GetCurSel();
	protocolList.GetLBText(selectedIdx, lastSelectedProtocol);

	// Load the data from protocol
	currentProtocol = protocols[selectedIdx];
	initProtocol();
}

void CMainDialog::calcTotalTime() {
	totalLeftSec = 0;

	vector<Action> tempActions;

	// Deep copy
	for (int i = 0; i < currentProtocol.actionList.size(); ++i) {
		tempActions.push_back(Action());
		tempActions[i].Label = currentProtocol.actionList[i].Label;
		tempActions[i].Temp = currentProtocol.actionList[i].Temp;
		tempActions[i].Time = currentProtocol.actionList[i].Time;
	}

	for (int i = 0; i < tempActions.size(); ++i) {
		Action action = tempActions[i];

		if (action.Label.Compare(L"GOTO") == 0) {
			CString gotoLabel;
			gotoLabel.Format(L"%d", (int)action.Temp);
			int remain = (int)action.Time - 1;
			tempActions[i].Time = (double)remain;

			if (remain != -1) {
				int gotoIndex = -1;
				for (int j = 0; j < tempActions.size(); ++j) {
					if (tempActions[j].Label.Compare(gotoLabel) == 0) {
						gotoIndex = j;
					}
				}

				i = gotoIndex - 1;
			}
		}
		else if(action.Label.Compare(L"SHOT") != 0){
			totalLeftSec += (int)action.Time;
		}
	}

	int second = totalLeftSec % 60;
	int minute = totalLeftSec / 60;
	// int hour = minute / 60;
	// minute = minute - hour * 60;

	CString totalTime;

	if (minute == 0)
		totalTime.Format(L"%02ds", second);
	else
		totalTime.Format(L"%02dm %02ds", minute, second);

	// Display total remaining time
	SetDlgItemText(IDC_STATIC_PROGRESS_REMAINING_TIME, totalTime);
}

void CMainDialog::initMagneto(){
	vector<CString> portList;

	m_Timer = new CMMTimers(1, GetSafeHwnd());
	device = new CDeviceConnect(GetSafeHwnd());
	magneto = new CMagneto();
	magneto->searchPort(portList);

	deviceList.ResetContent();

	for (int i = 0; i < portList.size(); ++i) {
		deviceList.AddString(portList[i]);
	}

	if (deviceList.GetCount() != 0) {
		deviceList.SetCurSel(0);
	}
}

void CMainDialog::OnBnClickedButtonConnect()
{
	// Check the button status
	CString buttonState;
	GetDlgItemText(IDC_BUTTON_CONNECT, buttonState);

	if (buttonState.Compare(L"Connect") == 0) {
		// Getting the device COM port
		int selectedIdx = deviceList.GetCurSel();

		if (selectedIdx != -1) {
			CString comport;
			deviceList.GetLBText(selectedIdx, comport);
			comport.Replace(L"COM", L"");
			int comPortNumber = _ttoi(comport);

			DriverStatus::Enum res = magneto->connect(comPortNumber);

			if (res == DriverStatus::CONNECTED)
			{
				magneto->setHwnd(GetSafeHwnd());

				// Getting the magneto data and check magneto data
				MagnetoProtocol magnetoData = FileManager::loadMagnetoProtocol(currentProtocol.protocolName);
				CString magnetoProtocolRes = magneto->loadProtocolFromData(magnetoData.protocolData);

				if (magneto->isCompileSuccess(magnetoProtocolRes)) {
					// initialize the protocol
					vector<ActionBeans> treeList;
					magneto->generateActionList(treeList);

					int maxActions = magneto->getTotalActionNumber();
					progressStatus.SetRange(0, maxActions);
				}
				else {
					if (magneto->isConnected()) {
						magneto->disconnect();
					}

					AfxMessageBox(L"Magneto data is not exist. Please setting the magneto protocol.");
					return;
				}

				// Getting the serialNumber
				long serialNumber = magneto->getSerialNumber();

				int deviceNums = device->GetDevices();
				for (int i = 0; i < deviceNums; ++i) {
					CString deviceSerial = device->GetDeviceSerial(i);
					deviceSerial.Replace(L"QuPCR", L"");

					int usbSerial = _ttoi(deviceSerial);

					// Connected the device
					if (usbSerial == serialNumber) {
						BOOL res = device->OpenDevice(LS4550EK_VID, LS4550EK_PID, device->GetDeviceSerialForConnection(i), TRUE);

						if (res) {
							isConnected = true;
						}
						else {
							AfxMessageBox(L"Magneto is connected but PCR is failed to connect(Unknown error).");
						}
						break;
					}
				}

				if (isConnected) {
					// Connection processing
					SetDlgItemText(IDC_EDIT_CONNECTI_STATUS, L"Connected");
					SetDlgItemText(IDC_BUTTON_CONNECT, L"Disconnect");
					GetDlgItem(IDC_COMBO_DEVICE_LIST)->EnableWindow(FALSE);

					if (isProtocolLoaded) {
						GetDlgItem(IDC_BUTTON_START)->EnableWindow();
					}
				}
				else {
					AfxMessageBox(L"No matched the PCR device for magneto.");
				}
			}
			else if (res == DriverStatus::NOT_CONNECTED)
			{
				AfxMessageBox(L"This device is not magneto.");
			}
			else
			{
				AfxMessageBox(L"All motors are not connected! Please check your device.");
			}
		}
		else {
			AfxMessageBox(L"Please select the device first.");
		}
	}
	else {
		isConnected = false;
		if (magneto->isConnected()) {
			magneto->disconnect();
		}

		device->CloseDevice();

		SetDlgItemText(IDC_EDIT_CONNECTI_STATUS, L"Disconnected");
		SetDlgItemText(IDC_BUTTON_CONNECT, L"Connect");
		GetDlgItem(IDC_COMBO_DEVICE_LIST)->EnableWindow();
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	}
}

void CMainDialog::OnBnClickedButtonStart()
{
	if (isConnected && isProtocolLoaded) {
		isStarted = !isStarted;
		if (isStarted) {
			progressStatus.SetPos(0);
			GetDlgItem(IDC_COMBO_PROTOCOLS)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_SETUP)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(FALSE);
			SetDlgItemText(IDC_BUTTON_START, L"Stop");
			SetDlgItemText(IDC_STATIC_PROGRESS_STATUS, L"Preparing the Magneto..");

			// init all values
			logStopped = false;
			initValues();
			calcTotalTime();
			initLog();

			// initialize the values
			currentCmd = CMD_PCR_RUN;

			m_prevTargetTemp = 25;

			m_currentTargetTemp = (BYTE)currentProtocol.actionList[0].Temp;

			findPID();

			clearChartValue();

			magneto->start();
			SetTimer(Magneto::TimerRuntaskID, Magneto::TimerRuntaskDuration, NULL);
		}
		else {
			GetDlgItem(IDC_COMBO_PROTOCOLS)->EnableWindow();
			GetDlgItem(IDC_BUTTON_SETUP)->EnableWindow();
			SetDlgItemText(IDC_BUTTON_START, L"Start");

			PCREndTask();

			KillTimer(Magneto::TimerRuntaskID);
			m_Timer->stopTimer();
		}
	}
	else {
		AfxMessageBox(L"Error occured!");
	}
}

void CMainDialog::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == Magneto::TimerRuntaskID)
	{
		if (!magneto->runTask()) {
			KillTimer(Magneto::TimerRuntaskID);
			AfxMessageBox(L"Limit 스위치가 설정되어 task 가 종료되었습니다.\n기기를 확인하세요.");
			return;
		}

		// Progress
		progressStatus.SetPos(magneto->getCurrentActionNumber());

		// Check magneto command
		if (magneto->getCurrentCmd() == ProtocolCmd::GO) {
			if(magneto->getCurrentfilter() == 1){
				SetDlgItemText(IDC_STATIC_PROGRESS_STATUS, L"Washing 1...");
			}
			else if (magneto->getCurrentfilter() == 2) {
				SetDlgItemText(IDC_STATIC_PROGRESS_STATUS, L"Washing 2...");
			}
			else if(magneto->getCurrentfilter() == 5){
				SetDlgItemText(IDC_STATIC_PROGRESS_STATUS, L"Elution...");
			}
			else if (magneto->getCurrentfilter() == 6) {
				SetDlgItemText(IDC_STATIC_PROGRESS_STATUS, L"Lysis...");
			}
			else if (magneto->getCurrentfilter() == 10) {
				SetDlgItemText(IDC_STATIC_PROGRESS_STATUS, L"Mixing...");
			}
			else if (magneto->getCurrentfilter() == 12) {
				SetDlgItemText(IDC_STATIC_PROGRESS_STATUS, L"Moving to PCR...");
			}
		}

		if (magneto->isIdle()) {
			progressStatus.SetPos(magneto->getTotalActionNumber());
			KillTimer(Magneto::TimerRuntaskID);
			SetDlgItemText(IDC_STATIC_PROGRESS_STATUS, L"PCR in progress...");
			m_Timer->startTimer(TIMER_DURATION, FALSE);
			return;
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

LRESULT CMainDialog::OnmmTimer(WPARAM wParam, LPARAM lParam) {
	if (isStarted)
	{
		int errorPrevent = 0;
		timeTask();
	}

	RxBuffer rx;
	TxBuffer tx;
	float currentTemp = 0.0;

	memset(&rx, 0, sizeof(RxBuffer));
	memset(&tx, 0, sizeof(TxBuffer));

	tx.cmd = currentCmd;
	tx.currentTargetTemp = (BYTE)m_currentTargetTemp;
	tx.ledControl = 1;
	tx.led_wg = ledControl_wg;
	tx.led_r = ledControl_r;
	tx.led_g = ledControl_g;
	tx.led_b = ledControl_b;
	tx.compensation = compensation;
	tx.currentCycle = currentCycle;

	// pid 값을 buffer 에 복사한다.
	memcpy(&(tx.pid_p1), &(m_kp), sizeof(float));
	memcpy(&(tx.pid_i1), &(m_ki), sizeof(float));
	memcpy(&(tx.pid_d1), &(m_kd), sizeof(float));

	// integral max 값을 buffer 에 복사한다.
	memcpy(&(tx.integralMax_1), &(integralMax), sizeof(float));

	BYTE senddata[65] = { 0, };
	BYTE readdata[65] = { 0, };
	memcpy(senddata, &tx, sizeof(TxBuffer));

	device->Write(senddata);

	if (device->Read(&rx) == 0)
		return FALSE;

	memcpy(readdata, &rx, sizeof(RxBuffer));

	// 기기로부터 받은 온도 값을 받아와서 저장함.
	// convert BYTE pointer to float type for reading temperature value.
	memcpy(&currentTemp, &(rx.chamber_temp_1), sizeof(float));

	// 기기로부터 받은 Photodiode 값을 받아와서 저장함.
	photodiode_h = rx.photodiode_h;
	photodiode_l = rx.photodiode_l;

	if (currentCmd == CMD_FAN_OFF){
		currentCmd = CMD_READY;
	}
	else if (currentCmd == CMD_PCR_STOP) {
		currentCmd = CMD_READY;
	}

	if (currentTemp < 0.0)
		return FALSE;

	// 150918 YJ added, For falling stage routine
	if (targetTempFlag && !freeRunning)
	{
		// target temp 이하가 되는 순간부터 freeRunning 으로 
		if (currentTemp <= m_currentTargetTemp)
		{
			freeRunning = true;
			freeRunningCounter = 0;
		}
	}

	if (freeRunning)
	{
		freeRunningCounter++;
		// 기기와 다르게 3초 후부터 arrival 로 인식하도록 변경
		if (freeRunningCounter >= (3000 / TIMER_DURATION))
		{
			targetTempFlag = false;
			freeRunning = false;
			freeRunningCounter = 0;
			isTargetArrival = true;
		}
	}

	if (fabs(currentTemp - m_currentTargetTemp) < m_cArrivalDelta && !targetTempFlag)
		isTargetArrival = true;

	// Check the error from device
	static bool onceShow = true;
	if (rx.currentError == ERROR_ASSERT && onceShow) {
		onceShow = false;
		AfxMessageBox(L"Software error occured!\nPlease contact to developer");
	}
	else if (rx.currentError == ERROR_OVERHEAT && onceShow) {
		onceShow = false;
		emergencyStop = true;
		PCREndTask();
	}

	// logging
	if (!logStopped) {
		CString values;
		double currentTime = (double)(timeGetTime() - recStartTime);
		recordingCount++;
		values.Format(L"%6d	%8.0f	%3.1f\n", recordingCount, currentTime, currentTemp);
		m_recFile.WriteString(values);

		// for log message per 1 sec
		/*
		if (recordingCount % 20 == 0) {
			int elapsed_time = (int)((double)(timeGetTime() - recStartTime) / 1000.);
			int min = elapsed_time / 60;
			int sec = elapsed_time % 60;
			CString elapseTime, lineTime, totalTime;
			elapseTime.Format(L"%dm %ds", min, sec);

			min = leftSec / 60;
			sec = leftSec % 60;

			// current left protocol time
			if (min == 0)
				lineTime.Format(L"%ds", sec);
			else
				lineTime.Format(L"%dm %ds", min, sec);

			CString tempStr;
			tempStr.Format(L"%3.1f", currentTemp);

			double lights = (double)(photodiode_h & 0x0f)*256. + (double)(photodiode_l);

			CString log;
			log.Format(L"cmd: %d, targetTemp: %3.1f, temp: %s, elapsed time: %s, line Time: %s, protocol Time: %s, device TargetArr: %d, mfc TargetArr: %d, free Running: %d, free Running Counter: %d, ArrivalDelta: %3.1f, tempFlag: %d, photodiode: %3.1f\n",
				currentCmd, m_currentTargetTemp, tempStr, elapseTime, lineTime, totalTime, (int)rx.targetArrival, (int)isTargetArrival, (int)freeRunning, (int)freeRunningCounter, m_cArrivalDelta, (int)targetTempFlag, lights);
			FileManager::log(log);
		}
		*/
	}

	return FALSE;
}

void CMainDialog::findPID()
{
	if (fabs(m_prevTargetTemp - m_currentTargetTemp) < .5)
		return; // if target temp is not change then do nothing 1->.5 correct

	double dist = 10000;
	int paramIdx = 0;

	for (UINT i = 0; i < pids.size(); i++)
	{
		double tmp = fabs(m_prevTargetTemp - pids[i].startTemp) + fabs(m_currentTargetTemp - pids[i].targetTemp);

		if (tmp < dist)
		{
			dist = tmp;
			paramIdx = i;
		}
	}

	m_kp = pids[paramIdx].kp;
	m_ki = pids[paramIdx].ki;
	m_kd = pids[paramIdx].kd;
}

void CMainDialog::initValues() {
	if (isStarted) {
		currentCmd = CMD_PCR_STOP;
	}

	magnetoProtocolIdx = 0;
	m_currentActionNumber = -1;
	m_leftGotoCount = -1;
	leftSec = 0;
	totalLeftSec = 0;
	m_timerCounter = 0;
	m_timeOut = 0;
	isTargetArrival = false;
	filterIndex = 0;
	filterRunning = false;
	shotCounter = 0;

	ledControl_wg = 1;
	ledControl_r = 1;
	ledControl_g = 1;
	ledControl_b = 1;

	m_kp = 0;
	m_ki = 0;
	m_kd = 0;
	emergencyStop = false;
	freeRunning = false;
	freeRunningCounter = 0;
	currentCycle = 0;
	recordingCount = 0;

	m_prevTargetTemp = m_currentTargetTemp = 25;
}

void CMainDialog::timeTask() {
	m_timerCounter++;

	// 1s 마다 실행되도록 설정
	if (m_timerCounter == (1000 / TIMER_DURATION))
	{
		m_timerCounter = 0;

		CString debug;
		debug.Format(L"Current cmd : %d, current action : %d target temp : %.1f, current left sec : %d\n", currentCmd, m_currentActionNumber, m_currentTargetTemp, leftSec);
		::OutputDebugString(debug);

		if (leftSec == 0) {
			m_currentActionNumber++;

			if ((m_currentActionNumber) >= currentProtocol.actionList.size())
			{
				::OutputDebugString(L"complete!\n");
				isCompletePCR = true;
				PCREndTask();
				return;
			}

			debug.Format(L"label : %s filterIndex : %d\n", currentProtocol.actionList[m_currentActionNumber].Label, filterIndex);
			::OutputDebugString(debug);

			// If the current protocol is SHOT
			if (currentProtocol.actionList[m_currentActionNumber].Label.Compare(L"SHOT") == 0)
			{
				if (filterIndex == 0) {
					// Check the this filter is used.
					if (!currentProtocol.useFam) {
						filterIndex = 1;
					}
				}
				if (filterIndex == 1) {
					// Check the this filter is used.
					if (!currentProtocol.useHex) {
						filterIndex = 2;
					}
				}
				if (filterIndex == 2) {
					// Check the this filter is used.
					if (!currentProtocol.useRox) {
						filterIndex = 3;
					}
				}
				if (filterIndex == 3) {
					// Check the this filter is used.
					if (!currentProtocol.useCY5) {
						filterIndex = 4;
					}
				}

				// 4 is finished
				if (filterIndex == 4) {
					filterIndex = 0;
					currentCycle++;

					CString values;
					double currentTime = (double)(timeGetTime() - recStartTime);
					double sensorValue = 0.0, sensorValue2 = 0.0, sensorValue3 = 0.0, sensorValue4 = 0.0;

					if (currentProtocol.useFam) {
						sensorValue = sensorValuesFam[currentCycle - 1];
					}
					if (currentProtocol.useHex) {
						sensorValue2 = sensorValuesHex[currentCycle - 1];
					}
					if (currentProtocol.useRox) {
						sensorValue3 = sensorValuesRox[currentCycle - 1];
					}
					if (currentProtocol.useCY5) {
						sensorValue4 = sensorValuesCy5[currentCycle - 1];
					}

					values.Format(L"%6d	%8.0f	%3.1f	%3.1f	%3.1f	%3.1f\n", currentCycle, currentTime, sensorValue, sensorValue2, sensorValue3, sensorValue4);
					m_recPDFile.WriteString(values);
				}
				else {
					// Run the filter
					if (filterRunning) {
						// Maybe finished..
						if (magneto->isFilterActionFinished() == false) {
							int *led;
							vector<double> *sensorValues;

							if (filterIndex == 0) {
								led = &ledControl_b;
								sensorValues = &sensorValuesFam;
							}
							else if (filterIndex == 1) {
								led = &ledControl_wg;
								sensorValues = &sensorValuesHex;
							}
							else if (filterIndex == 2) {
								led = &ledControl_g;
								sensorValues = &sensorValuesRox;
							}
							else if (filterIndex == 3) {
								led = &ledControl_r;
								sensorValues = &sensorValuesCy5;
							}

							// Turn on the led
							*led = 0;

							// Shot sequence
							shotCounter++;
							if (shotCounter >= 2) {
								// Getting the photodiode data
								double lights = (double)(photodiode_h & 0x0f)*256. + (double)(photodiode_l);
								sensorValues->push_back(lights);
								setChartValue();

								debug.Format(L"filter value : %d, %d, %f\n", photodiode_h, photodiode_l, lights);
								::OutputDebugString(debug);

								// Turn off led
								*led = 1;
								shotCounter = 0;

								// Next filter
								filterIndex++;
								filterRunning = false;
							}
						}
					}
					else {
						filterRunning = true;
						magneto->runFilterAction(filterIndex);
					}
					m_currentActionNumber--;	// For checking the shot command 
				}
			}
			// If the current protocol is GOTO
			else if (currentProtocol.actionList[m_currentActionNumber].Label.Compare(L"GOTO") == 0)
			{
				if (m_leftGotoCount < 0)
					m_leftGotoCount = (int)currentProtocol.actionList[m_currentActionNumber].Time;

				if (m_leftGotoCount == 0)
					m_leftGotoCount = -1;
				else
				{
					m_leftGotoCount--;

					// GOTO 문의 target label 값을 넣어줌
					CString tmpStr;
					tmpStr.Format(L"%d", (int)currentProtocol.actionList[m_currentActionNumber].Temp);

					int pos = 0;
					for (pos = 0; pos < currentProtocol.actionList.size(); ++pos)
						if (tmpStr.Compare(currentProtocol.actionList[pos].Label) == 0)
							break;

					m_currentActionNumber = pos - 1;
				}
			}
			// Label
			else {
				m_prevTargetTemp = m_currentTargetTemp;
				m_currentTargetTemp = (int)currentProtocol.actionList[m_currentActionNumber].Temp;

				targetTempFlag = m_prevTargetTemp > m_currentTargetTemp;

				isTargetArrival = false;
				leftSec = (int)(currentProtocol.actionList[m_currentActionNumber].Time);
				m_timeOut = TIMEOUT_CONST * 10;

				// find the proper pid values.
				findPID();
			}
		}
		else { // The action is in progress.
			if (!isTargetArrival)
			{
				m_timeOut--;

				if (m_timeOut == 0)
				{
					AfxMessageBox(L"The target temperature cannot be reached!!");
					PCREndTask();
				}
			}
			else {
				leftSec--;
				totalLeftSec--;

				CString leftTime;
				int min = totalLeftSec / 60;
				int sec = totalLeftSec % 60;

				if (min == 0)
					leftTime.Format(L"%ds", sec);
				else
					leftTime.Format(L"%dm %ds", min, sec);
				SetDlgItemText(IDC_STATIC_PROGRESS_REMAINING_TIME, leftTime);
			}
		}
	}
}

void CMainDialog::PCREndTask(){
	// Stop timer
	m_Timer->stopTimer();

	while (true)
	{
		RxBuffer rx;
		TxBuffer tx;

		memset(&rx, 0, sizeof(RxBuffer));
		memset(&tx, 0, sizeof(TxBuffer));

		tx.cmd = CMD_PCR_STOP;

		BYTE senddata[65] = { 0, };
		BYTE readdata[65] = { 0, };
		memcpy(senddata, &tx, sizeof(TxBuffer));

		device->Write(senddata);

		device->Read(&rx);

		memcpy(readdata, &rx, sizeof(RxBuffer));

		if (rx.state == STATE_READY) break;

		Sleep(TIMER_DURATION);
	}

	m_Timer->startTimer(TIMER_DURATION, FALSE);

	initValues();
	clearLog();

	isStarted = false;

	progressStatus.SetPos(0);
	GetDlgItem(IDC_COMBO_PROTOCOLS)->EnableWindow();
	GetDlgItem(IDC_BUTTON_SETUP)->EnableWindow();
	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow();
	SetDlgItemText(IDC_BUTTON_START, L"Start");
	SetDlgItemText(IDC_STATIC_PROGRESS_STATUS, L"Idle..");
	SetDlgItemText(IDC_EDIT_CT_FAM, L"");
	SetDlgItemText(IDC_EDIT_CT_HEX, L"");
	SetDlgItemText(IDC_EDIT_CT_ROX, L"");
	SetDlgItemText(IDC_EDIT_CT_CY5, L"");

	if (!emergencyStop)
	{
		if (isCompletePCR) {
			setCTValue(sensorValuesFam, 0);
			setCTValue(sensorValuesHex, 1);
			setCTValue(sensorValuesRox, 2);
			setCTValue(sensorValuesCy5, 3);
			AfxMessageBox(L"PCR ended!!");
		}
		else AfxMessageBox(L"PCR incomplete!!");
	}
	else
	{
		AfxMessageBox(L"Emergency stop!(overheating)");
	}

	emergencyStop = false;
	isCompletePCR = false;

	// reset the remaining time
	calcTotalTime();
}

void CMainDialog::setChartValue() {
	m_Chart.DeleteAllData();

	double *dataFam = new double[sensorValuesFam.size() * 2];
	double *dataHex = new double[sensorValuesHex.size() * 2];
	double *dataRox = new double[sensorValuesRox.size() * 2];
	double *dataCy5 = new double[sensorValuesCy5.size() * 2];

	vector<double> copySensorValuesFam, copySensorValuesHex, copySensorValuesRox, copySensorValuesCy5;
	double meanFam = 0.0, meanHex = 0.0, meanRox = 0.0, meanCy5 = 0.0;

	int maxY = 0;

	if (useFam)
	{
		// Calculate the mean value first
		if (sensorValuesFam.size() >= 10) {
			double sum = std::accumulate(sensorValuesFam.begin(), sensorValuesFam.begin() + 10, 0.0);
			meanFam = sum / 10;
		}
		else {
			double sum = std::accumulate(sensorValuesFam.begin(), sensorValuesFam.end(), 0.0);
			meanFam = sum / sensorValuesFam.size();
		}

		// Copy data
		for (int i = 0; i < sensorValuesFam.size(); ++i) {
			copySensorValuesFam.push_back(sensorValuesFam[i] - meanFam);
		}

		int	nDims_fam = 2, dims_fam[2] = { 2, copySensorValuesFam.size() };
		for (int i = 0; i < copySensorValuesFam.size(); ++i)
		{
			dataFam[i] = i;
			dataFam[i + copySensorValuesFam.size()] = copySensorValuesFam[i];
		}
		m_Chart.SetDataColor(m_Chart.AddData(dataFam, nDims_fam, dims_fam), RGB(0, 0, 255));

		int tempMax = *max_element(copySensorValuesFam.begin(), copySensorValuesFam.end());

		if (maxY < tempMax) {
			maxY = tempMax;
		}
	}

	if (useHex)
	{
		// Calculate the mean value first
		if (sensorValuesHex.size() >= 10) {
			double sum = std::accumulate(sensorValuesHex.begin(), sensorValuesHex.begin() + 10, 0.0);
			meanHex = sum / 10;
		}
		else {
			double sum = std::accumulate(sensorValuesHex.begin(), sensorValuesHex.end(), 0.0);
			meanHex = sum / sensorValuesHex.size();
		}

		// Copy data
		for (int i = 0; i < sensorValuesHex.size(); ++i) {
			copySensorValuesHex.push_back(sensorValuesHex[i] - meanHex);
		}

		int	nDims_hex = 2, dims_hex[2] = { 2, copySensorValuesHex.size() };
		for (int i = 0; i < copySensorValuesHex.size(); ++i)
		{
			dataHex[i] = i;
			dataHex[i + copySensorValuesHex.size()] = copySensorValuesHex[i];
		}
		m_Chart.SetDataColor(m_Chart.AddData(dataHex, nDims_hex, dims_hex), RGB(0, 255, 0));

		int tempMax = *max_element(copySensorValuesHex.begin(), copySensorValuesHex.end());

		if (maxY < tempMax) {
			maxY = tempMax;
		}
	}

	if (useRox)
	{
		// Calculate the mean value first
		if (sensorValuesRox.size() >= 10) {
			double sum = std::accumulate(sensorValuesRox.begin(), sensorValuesRox.begin() + 10, 0.0);
			meanRox = sum / 10;
		}
		else {
			double sum = std::accumulate(sensorValuesRox.begin(), sensorValuesRox.end(), 0.0);
			meanRox = sum / sensorValuesRox.size();
		}

		// Copy data
		for (int i = 0; i < sensorValuesRox.size(); ++i) {
			copySensorValuesRox.push_back(sensorValuesRox[i] - meanRox);
		}

		int	nDims_rox = 2, dims_rox[2] = { 2, copySensorValuesRox.size() };
		for (int i = 0; i < copySensorValuesRox.size(); ++i)
		{
			dataRox[i] = i;
			dataRox[i + copySensorValuesRox.size()] = copySensorValuesRox[i];
		}
		m_Chart.SetDataColor(m_Chart.AddData(dataRox, nDims_rox, dims_rox), RGB(0, 128, 0));

		int tempMax = *max_element(copySensorValuesRox.begin(), copySensorValuesRox.end());

		if (maxY < tempMax) {
			maxY = tempMax;
		}
	}

	if (useCy5)
	{
		// Calculate the mean value first
		if (sensorValuesCy5.size() >= 10) {
			double sum = std::accumulate(sensorValuesCy5.begin(), sensorValuesCy5.begin() + 10, 0.0);
			meanCy5 = sum / 10;
		}
		else {
			double sum = std::accumulate(sensorValuesCy5.begin(), sensorValuesCy5.end(), 0.0);
			meanCy5 = sum / sensorValuesCy5.size();
		}

		// Copy data
		for (int i = 0; i < sensorValuesCy5.size(); ++i) {
			copySensorValuesCy5.push_back(sensorValuesCy5[i] - meanCy5);
		}

		int	nDims_cy5 = 2, dims_cy5[2] = { 2, copySensorValuesCy5.size() };
		for (int i = 0; i < copySensorValuesCy5.size(); ++i)
		{
			dataCy5[i] = i;
			dataCy5[i + copySensorValuesCy5.size()] = copySensorValuesCy5[i];
		}
		m_Chart.SetDataColor(m_Chart.AddData(dataCy5, nDims_cy5, dims_cy5), RGB(255, 0, 0));

		int tempMax = *max_element(copySensorValuesCy5.begin(), copySensorValuesCy5.end());

		if (maxY < tempMax) {
			maxY = tempMax;
		}
	}

	maxY += displayDelta;

	CAxis *axis = m_Chart.GetAxisByLocation(kLocationLeft);
	axis->SetRange(-displayDelta, maxY);

	InvalidateRect(&CRect(15, 130, 470, 500));
	Invalidate(FALSE);
}

void CMainDialog::clearChartValue() {
	sensorValuesFam.clear();
	sensorValuesHex.clear();
	sensorValuesRox.clear();
	sensorValuesCy5.clear();

	sensorValuesFam.push_back(1.0);
	sensorValuesHex.push_back(1.0);
	sensorValuesRox.push_back(1.0);
	sensorValuesCy5.push_back(1.0);

	m_Chart.DeleteAllData();
	InvalidateRect(&CRect(15, 130, 470, 500));
}

void CMainDialog::setCTValue(vector<double> &sensorValue, int filterIndex) {
	float threshold = 0.697 * flRelativeMax / 10.;
	float logThreshold = log(threshold);
	float ct;
	CString ctText;
	
	int idx = sensorValue.size();
	for (int i = 0; i < sensorValue.size(); ++i) {
		if (log(sensorValue[i]) > logThreshold) {
			idx = i;
			break;
		}
	}

	if (idx >= sensorValue.size() || idx <= 0) {
		return;
	}
	else {
		float cpos = idx + 1;
		float cval = log(sensorValue[idx]);
		float delta = cval - log(sensorValue[idx - 1]);
		ct = cpos - (cval - logThreshold) / delta;
		ctText.Format(L"%.1f", ct);
	}

	if (filterIndex == 0 && currentProtocol.useFam) {
		SetDlgItemText(IDC_EDIT_CT_FAM, ctText);
	}
	else if (filterIndex == 1 && currentProtocol.useHex) {
		SetDlgItemText(IDC_EDIT_CT_HEX, ctText);
	}
	else if (filterIndex == 2 && currentProtocol.useRox) {
		SetDlgItemText(IDC_EDIT_CT_ROX, ctText);
	}
	else if (filterIndex == 3 && currentProtocol.useCY5) {
		SetDlgItemText(IDC_EDIT_CT_CY5, ctText);
	}

}

void CMainDialog::initLog() {
	CreateDirectory(L"./Record/", NULL);

	CString fileName, fileName2;
	CTime time = CTime::GetCurrentTime();
	fileName = time.Format(L"./Record/%Y%m%d-%H%M-%S.txt");
	fileName2 = time.Format(L"./Record/pd%Y%m%d-%H%M-%S.txt");

	m_recFile.Open(fileName, CStdioFile::modeCreate | CStdioFile::modeWrite);
	m_recFile.WriteString(L"Number	Time	Temperature\n");

	m_recPDFile.Open(fileName2, CStdioFile::modeCreate | CStdioFile::modeWrite);
	m_recPDFile.WriteString(L"Cycle	Time	FAM	HEX	ROX	CY5\n");

	recStartTime = timeGetTime();
}

void CMainDialog::clearLog() {
	logStopped = true;
	m_recFile.Close();
	m_recPDFile.Close();
}

void CMainDialog::OnBnClickedButtonFilterFam()
{
	useFam = !useFam;
	if (useFam) {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_FAM, famImg);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_FAM, offImg);
	}

	setChartValue();
	InvalidateRect(&CRect(15, 130, 470, 500));
}


void CMainDialog::OnBnClickedButtonFilterHex()
{
	useHex = !useHex;
	if (useHex) {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_HEX, hexImg);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_HEX, offImg);
	}

	setChartValue();
	InvalidateRect(&CRect(15, 130, 470, 500));
}


void CMainDialog::OnBnClickedButtonFilterRox()
{
	useRox = !useRox;
	if (useRox) {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_ROX, roxImg);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_ROX, offImg);
	}

	setChartValue();
	InvalidateRect(&CRect(15, 130, 470, 500));
}


void CMainDialog::OnBnClickedButtonFilterCy5()
{
	useCy5 = !useCy5;
	if (useCy5) {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_CY5, cy5Img);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_FILTER_CY5, offImg);
	}

	setChartValue();
	InvalidateRect(&CRect(15, 130, 470, 500));
}
