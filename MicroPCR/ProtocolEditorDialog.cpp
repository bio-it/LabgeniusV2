// ProtocolEditorDialog.cpp: 구현 파일
//

#include "stdafx.h"
#include "ProtocolEditorDialog.h"
#include "afxdialogex.h"
#include "resource.h"
#include "FileManager.h"

#include "Magneto.h"


// CProtocolEditorDialog 대화 상자

IMPLEMENT_DYNAMIC(CProtocolEditorDialog, CDialogEx)

CProtocolEditorDialog::CProtocolEditorDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_PROTOCOL_EDITOR, pParent)
	, protocolType(0)
	, currentLabel(0)
	, isEdit(false)
	, ctFam(38.00)
	, ctHex(38.00)
	, ctRox(38.00)
	, ctCY5(38.00)
{
	// Add the default item
	Action action;
	action.Label = L"1";
	action.Temp = 95.0f;
	action.Time = 60.0f;
	currentProtocol.actionList.push_back(action);
}

CProtocolEditorDialog::~CProtocolEditorDialog()
{
}

void CProtocolEditorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CUSTOM_PROTOCOL_EDITOR, protocolTable);
	DDX_Radio(pDX, IDC_RADIO_PROTOCOL_LABEL, (int &)protocolType);

	DDX_Text(pDX, IDC_EDIT_FILTER_FAM_CT, ctFam);
	DDV_MinMaxFloat(pDX, ctFam, 0, 50);
	DDX_Text(pDX, IDC_EDIT_FILTER_HEX_CT, ctHex);
	DDV_MinMaxFloat(pDX, ctHex, 0, 50);
	DDX_Text(pDX, IDC_EDIT_FILTER_ROX_CT, ctRox);
	DDV_MinMaxFloat(pDX, ctRox, 0, 50);
	DDX_Text(pDX, IDC_EDIT_FILTER_CY5_CT, ctCY5);
	DDV_MinMaxFloat(pDX, ctCY5, 0, 50);
}


BEGIN_MESSAGE_MAP(CProtocolEditorDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_PROTOCOL_FILTER_FAM, &CProtocolEditorDialog::OnBnClickedButtonProtocolFilterFam)
	ON_BN_CLICKED(IDC_BUTTON_PROTOCOL_FILTER_HEX, &CProtocolEditorDialog::OnBnClickedButtonProtocolFilterHex)
	ON_BN_CLICKED(IDC_BUTTON_PROTOCOL_FILTER_ROX, &CProtocolEditorDialog::OnBnClickedButtonProtocolFilterRox)
	ON_BN_CLICKED(IDC_BUTTON_PROTOCOL_FILTER_CY5, &CProtocolEditorDialog::OnBnClickedButtonProtocolFilterCy5)
	ON_BN_CLICKED(IDC_BUTTON_PROTOCOL_SAVE, &CProtocolEditorDialog::OnBnClickedButtonProtocolSave)
	ON_BN_CLICKED(IDC_BUTTON_PROTOCOL_CANCEL, &CProtocolEditorDialog::OnBnClickedButtonProtocolCancel)
	ON_BN_CLICKED(IDC_BUTTON_ADD_PROTOCOL, &CProtocolEditorDialog::OnBnClickedButtonAddProtocol)
END_MESSAGE_MAP()


// CProtocolEditorDialog 메시지 처리기


BOOL CProtocolEditorDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Load bitmap
	offImg.LoadBitmapW(IDB_BITMAP_OFF);
	famImg.LoadBitmapW(IDB_BITMAP_FAM);
	hexImg.LoadBitmapW(IDB_BITMAP_HEX);
	roxImg.LoadBitmapW(IDB_BITMAP_ROX);
	cy5Img.LoadBitmapW(IDB_BITMAP_CY5);

	CString ctFam, ctHex, ctRox, ctCY5;

	// Setting the filter image
	if (currentProtocol.useFam) {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_FAM, famImg);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_FAM, offImg);
	}

	if (currentProtocol.useHex) {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_HEX, hexImg);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_HEX, offImg);
	}

	if (currentProtocol.useRox) {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_ROX, roxImg);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_ROX, offImg);
	}

	if (currentProtocol.useCY5) {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_CY5, cy5Img);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_CY5, offImg);
	}

	// Protocol name setting
	SetDlgItemText(IDC_EDIT_PROTOCOL_NAME, currentProtocol.protocolName);

	// if the editing mode, disable protcol name edit field
	if (isEdit) {
		GetDlgItem(IDC_EDIT_PROTOCOL_NAME)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_EDIT_PROTOCOL_NAME)->EnableWindow(TRUE);
	}

	// Protocol filter result setting
	GetDlgItem(IDC_EDIT_FILTER_FAM)->EnableWindow(currentProtocol.useFam);
	GetDlgItem(IDC_EDIT_FILTER_HEX)->EnableWindow(currentProtocol.useHex);
	GetDlgItem(IDC_EDIT_FILTER_ROX)->EnableWindow(currentProtocol.useRox);
	GetDlgItem(IDC_EDIT_FILTER_CY5)->EnableWindow(currentProtocol.useCY5);

	GetDlgItem(IDC_EDIT_FILTER_FAM_CT)->EnableWindow(currentProtocol.useFam);
	GetDlgItem(IDC_EDIT_FILTER_HEX_CT)->EnableWindow(currentProtocol.useHex);
	GetDlgItem(IDC_EDIT_FILTER_ROX_CT)->EnableWindow(currentProtocol.useRox);
	GetDlgItem(IDC_EDIT_FILTER_CY5_CT)->EnableWindow(currentProtocol.useCY5);

	SetDlgItemText(IDC_EDIT_FILTER_FAM, currentProtocol.labelFam);
	SetDlgItemText(IDC_EDIT_FILTER_HEX, currentProtocol.labelHex);
	SetDlgItemText(IDC_EDIT_FILTER_ROX, currentProtocol.labelRox);
	SetDlgItemText(IDC_EDIT_FILTER_CY5, currentProtocol.labelCY5);

	ctFam.Format(L"%.02f", currentProtocol.ctFam);
	ctHex.Format(L"%.02f", currentProtocol.ctHex);
	ctRox.Format(L"%.02f", currentProtocol.ctRox);
	ctCY5.Format(L"%.02f", currentProtocol.ctCY5);

	SetDlgItemText(IDC_EDIT_FILTER_FAM_CT, ctFam);
	SetDlgItemText(IDC_EDIT_FILTER_HEX_CT, ctHex);
	SetDlgItemText(IDC_EDIT_FILTER_ROX_CT, ctRox);
	SetDlgItemText(IDC_EDIT_FILTER_CY5_CT, ctCY5);

	// Magneto data setting
	SetDlgItemText(IDC_EDIT_MAGNETO_PROTOCOL, currentMagnetoProtocol.protocolData);

	initProtocolTable();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CProtocolEditorDialog::setProtocol(Protocol& protocol) {
	currentProtocol = protocol;
	// Load the magneto protocol from currentProtocol
	currentMagnetoProtocol = FileManager::loadMagnetoProtocol(currentProtocol.protocolName);

	this->ctFam = currentProtocol.ctFam;
	this->ctHex = currentProtocol.ctHex;
	this->ctRox = currentProtocol.ctRox;
	this->ctCY5 = currentProtocol.ctCY5;

	isEdit = true;
}

CString CProtocolEditorDialog::checkProtocol(CString& protocol) {
	vector<CString> convertedProtocol;

	// CString to vector
	CString token;
	int count = 0;
	int protocolCount = 0;
	while (AfxExtractSubString(token, protocol, count, L'\n')) {
		convertedProtocol.push_back(token);
		count++;
	}

	CString compileMessage = L"=====Compile Error=====\n";
	// cmd list 를 만들어 command 와 mapping 시킨다.
	static const CString tempCmdList[14] = { L"goto", L"filter", L"mixing", L"waiting", L"pumping up", L"pumping sup", L"pumping down", L"pumping sdown", L"ready", L"home", L"magnet on", L"magnet off", L"heating", L"pcr", };

	// 비정상적인 파일임을 알림
	if (convertedProtocol.size() == 0)
		return L"Compile Error: 비정상인 파일입니다.";

	// 모든 Protocol line 을 읽는다.
	for (int i = 0; i < convertedProtocol.size(); ++i)
	{
		int offset = 0;
		CString line = convertedProtocol[i].Trim();
		CString cmd = line.Tokenize(L" ", offset).Trim();

		if (cmd.Compare(L"pumping") == 0)
		{
			CString subCmd = line.Tokenize(L" ", offset).Trim();


			if (subCmd.Compare(L"up") == 0 || subCmd.Compare(L"sup") == 0 || subCmd.Compare(L"down") == 0 || subCmd.Compare(L"sdown") == 0) // 기존 if( subCmd.Compare(L"up") == 0 || subCmd.Compare(L"down") == 0 ) 170106_2 KSY 
				cmd += L" " + subCmd;
			else
			{
				compileMessage.Format(L"%s\nLine %d : Invalid argument value", compileMessage, i + 1);
				continue;
			}
		}

		// Command 가 없는 경우와 주석 문자가 처음 시작되는 경우 무시
		if (cmd.IsEmpty())
			continue;

		else if (cmd.GetAt(0) == '%')
			continue;

		// 값을 저장할 구조체 초기화
		ProtocolBinary bin = { -1, -1 };

		for (int j = 0; j < ProtocolCmd::MAX + 1; ++j)
		{
			// 커맨드에 매개변수(args) 가 없는 경우 처리
			// 아래 명시된 프로토콜 커맨드는 매개변수가 없다.
			if ((j == ProtocolCmd::MAGNET_ON) || (j == ProtocolCmd::MAGNET_OFF) || (j == ProtocolCmd::PCR) ||
				(j == ProtocolCmd::HOME) || (j == ProtocolCmd::READY))
			{
				if (line.Compare(tempCmdList[j]) == 0)
					bin.cmd = j;
			}

			// 커맨드 매개변수(args) 가 있는 경우 처리
			// pumping 은 위에서 처리,
			// GO, FILTER, MIX, WAIT, HEATING 에 대해 처리한다.
			else if (cmd.Compare(tempCmdList[j]) == 0)
			{
				bin.cmd = j;

				CString arg = line.Tokenize(L" ", offset);

				// arg 값이 있는지 체크
				if (arg.Compare(L"") != 0)
				{
					if (arg.Compare(L"full") == 0)
						bin.arg = -1;
					else
						bin.arg = _ttoi(arg);
				}

				// 없는 경우 에러 메시지 추가
				else
				{
					compileMessage.Format(L"%s\nLine %d : Invalid argument value", compileMessage, i + 1);
				}

				break;
			}
		}

		if (bin.cmd == -1)
			compileMessage.Format(L"%s\nLine %d : Invalid command value", compileMessage, i + 1);
		else {
			protocolCount++;
		}
	}

	if (protocolCount == 0) {
		compileMessage.Format(L"%s\n\n- Magneto 프로토콜 데이터가 없습니다.", compileMessage);
	}

	// Compile error message 가 변경되지 않은 경우 성공한 경우
	if (compileMessage.Compare(L"=====Compile Error=====\n") == 0)
	{
		return Magneto::CompileMessageOk;
	}

	return compileMessage;
}

void CProtocolEditorDialog::OnBnClickedButtonProtocolFilterFam()
{
	currentProtocol.useFam = !currentProtocol.useFam;
	GetDlgItem(IDC_EDIT_FILTER_FAM)->EnableWindow(currentProtocol.useFam);
	GetDlgItem(IDC_EDIT_FILTER_FAM_CT)->EnableWindow(currentProtocol.useFam);

	if (currentProtocol.useFam) {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_FAM, famImg);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_FAM, offImg);
	}
}


void CProtocolEditorDialog::OnBnClickedButtonProtocolFilterHex()
{
	currentProtocol.useHex = !currentProtocol.useHex;
	GetDlgItem(IDC_EDIT_FILTER_HEX)->EnableWindow(currentProtocol.useHex);
	GetDlgItem(IDC_EDIT_FILTER_HEX_CT)->EnableWindow(currentProtocol.useHex);

	if (currentProtocol.useHex) {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_HEX, hexImg);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_HEX, offImg);
	}
}


void CProtocolEditorDialog::OnBnClickedButtonProtocolFilterRox()
{
	currentProtocol.useRox = !currentProtocol.useRox;
	GetDlgItem(IDC_EDIT_FILTER_ROX)->EnableWindow(currentProtocol.useRox);
	GetDlgItem(IDC_EDIT_FILTER_ROX_CT)->EnableWindow(currentProtocol.useRox);

	if (currentProtocol.useRox) {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_ROX, roxImg);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_ROX, offImg);
	}
}


void CProtocolEditorDialog::OnBnClickedButtonProtocolFilterCy5()
{
	currentProtocol.useCY5 = !currentProtocol.useCY5;
	GetDlgItem(IDC_EDIT_FILTER_CY5)->EnableWindow(currentProtocol.useCY5);
	GetDlgItem(IDC_EDIT_FILTER_CY5_CT)->EnableWindow(currentProtocol.useCY5);

	if (currentProtocol.useCY5) {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_CY5, cy5Img);
	}
	else {
		SET_BUTTON_IMAGE(IDC_BUTTON_PROTOCOL_FILTER_CY5, offImg);
	}
}


BOOL CProtocolEditorDialog::PreTranslateMessage(MSG* pMsg)
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


void CProtocolEditorDialog::OnBnClickedButtonProtocolSave()
{
	// Load saved protocols
	vector<Protocol> protocols;
	vector<MagnetoProtocol> magnetoProtocols;

	bool res = FileManager::loadProtocols(protocols);
	bool res2 = FileManager::loadMagnetoProtocols(magnetoProtocols);
	CString protocolName;
	CString labelFam, labelHex, labelRox, labelCY5;

	GetDlgItemText(IDC_EDIT_PROTOCOL_NAME, protocolName);

	// Getting the filter labels & ct
	GetDlgItemText(IDC_EDIT_FILTER_FAM, labelFam);
	GetDlgItemText(IDC_EDIT_FILTER_HEX, labelHex);
	GetDlgItemText(IDC_EDIT_FILTER_ROX, labelRox);
	GetDlgItemText(IDC_EDIT_FILTER_CY5, labelCY5);

	// don't check protocol name if the editing mode.
	if (!isEdit) {
		if (protocolName.IsEmpty()) {
			AfxMessageBox(L"Please input the protocol name.");
			return;
		}

		// When the saved data is exist, check the protocol name.
		if (res) {
			// Check protocol name
			for (int i = 0; i < protocols.size(); ++i) {
				if (protocolName.Compare(protocols[i].protocolName) == 0) {
					AfxMessageBox(L"This protocol name is already exist.");
					return;
				}
			}
		}
	}

	// Check the filter status, need to select 1 filter at least.
	if (!(currentProtocol.useFam || currentProtocol.useHex || currentProtocol.useRox || currentProtocol.useCY5)) {
		AfxMessageBox(L"Please select the filter at least 1.");
		return;
	}

	// Check the filter label
	if (currentProtocol.useFam) {
		if (labelFam.IsEmpty()) {
			AfxMessageBox(L"Please set the FAM filter label.");
			return;
		}
		
		// Check the range
		if (!GetDlgItem(IDC_EDIT_FILTER_FAM_CT)->UpdateData()) {
			return;
		}
	}

	if (currentProtocol.useHex) {
		if (labelHex.IsEmpty()) {
			AfxMessageBox(L"Please set the HEX filter label.");
			return;
		}

		// Check the range
		if (!GetDlgItem(IDC_EDIT_FILTER_HEX_CT)->UpdateData()) {
			return;
		}
	}

	if (currentProtocol.useRox) {
		if (labelRox.IsEmpty()) {
			AfxMessageBox(L"Please set the ROX filter label.");
			return;
		}

		// Check the range
		if (!GetDlgItem(IDC_EDIT_FILTER_ROX_CT)->UpdateData()) {
			return;
		}
	}

	if (currentProtocol.useCY5) {
		if (labelCY5.IsEmpty()) {
			AfxMessageBox(L"Please set the CY5 filter label.");
			return;
		}

		// Check the range
		if (!GetDlgItem(IDC_EDIT_FILTER_CY5_CT)->UpdateData()) {
			return;
		}
	}
	
	// For check the range of CT values
	if (!UpdateData()) {
		return;
	}

	currentProtocol.ctFam = this->ctFam;
	currentProtocol.ctHex = this->ctHex;
	currentProtocol.ctRox = this->ctRox;
	currentProtocol.ctCY5 = this->ctCY5;

	// save the labels
	currentProtocol.labelFam = labelFam;
	currentProtocol.labelHex = labelHex;
	currentProtocol.labelRox = labelRox;
	currentProtocol.labelCY5 = labelCY5;

	// Check magneto protocol validation
	
	CString magnetoProtocol;
	GetDlgItemText(IDC_EDIT_MAGNETO_PROTOCOL, magnetoProtocol);
	CString checkResult = checkProtocol(magnetoProtocol);

	if (checkResult.Compare(Magneto::CompileMessageOk) != 0) {
		AfxMessageBox(checkResult);
		return;
	}
	
	CString message;

	// Save new protocol
	// Load the grid data and apply the protocol class
	for (int row = 1; row < protocolTable.GetRowCount(); ++row) {
		CString label = protocolTable.GetItemText(row, 0);
		CString temp = protocolTable.GetItemText(row, 1);
		CString time = protocolTable.GetItemText(row, 2);
		
		if (label.Compare(L"GOTO") == 0) {
			// GOTO
			if (time.IsEmpty()) {
				message.Format(L"%d of row's loop data is empty.", row);
				AfxMessageBox(message);
				return;
			}
			else {
				int res = _ttoi(time);

				if (res == 0) {
					message.Format(L"%d of row's loop data is invalid.", row);
					AfxMessageBox(message);
					return;
				}
			}
		}
		else if (label.Compare(L"SHOT") == 0) {
			// No need to check
		}
		else {
			// Label
			if (temp.IsEmpty()) {
				message.Format(L"%d of row's temp data is empty.", row);
				AfxMessageBox(message);
				return;
			}
			else {
				int res = _ttoi(temp);

				if (!(res >= 4 && res <= 104)) {
					message.Format(L"%d of row's temp data is invalid(4~104).", row);
					AfxMessageBox(message);
					return;
				}
			}

			if (time.IsEmpty()) {
				message.Format(L"%d of row's time data is empty.", row);
				AfxMessageBox(message);
				return;
			}
			else {
				int res = _ttoi(time);

				// _ttoi function return the 0 value if the string is not number.
				if (time.Compare(L"0") != 0) {
					if (!(res >= 0 && res <= 3000)) {
						message.Format(L"%d of row's time data is invalid(1~3000).", row);
						AfxMessageBox(message);
						return;
					}
				}
			}
		}

		// Update new data
		currentProtocol.actionList[row - 1].Temp = _ttof(temp);
		currentProtocol.actionList[row - 1].Time = _ttof(time);
	}

	// Save protocol into file
	currentProtocol.protocolName = protocolName;
	currentMagnetoProtocol.protocolName = protocolName;
	currentMagnetoProtocol.protocolData = magnetoProtocol;
	if (isEdit) {
		// Change the protocol by new data
		int idx = -1;
		for (int i = 0; i < protocols.size(); ++i) {
			if (protocolName.Compare(protocols[i].protocolName) == 0) {
				idx = i;
				break;
			}
		}

		if (idx == -1) {
			AfxMessageBox(L"Failed! can't find previous protocol name.");
			return;
		}

		protocols[idx] = currentProtocol;
		magnetoProtocols[idx] = currentMagnetoProtocol;
	}
	else {
		protocols.push_back(currentProtocol);
		magnetoProtocols.push_back(currentMagnetoProtocol);
	}
	res = FileManager::saveProtocols(protocols);
	res2 = FileManager::saveMagnetoProtocols(magnetoProtocols);

	if (res && res2) {
		AfxMessageBox(L"Success!");
		OnOK();
	}
	else {
		AfxMessageBox(L"Failed! Unknown error!");
	}
}


void CProtocolEditorDialog::OnBnClickedButtonProtocolCancel()
{
	OnCancel();
}


void CProtocolEditorDialog::OnBnClickedButtonAddProtocol()
{
	UpdateData();

	// Check previous table data
	int currentRowCount = protocolTable.GetRowCount();
	
	CString label, temp = L"95", time = L"60";

	// Label type, just adding the label
	if (protocolType == 0) {
		currentLabel++;
		label.Format(L"%d", currentLabel);
	}
	else if (protocolType == 1) {	// GOTO
		int targetLabel = protocolTable.GetFocusCell().row;

		if (targetLabel == -1) {
			AfxMessageBox(L"Please select target label.");
			return;
		}
		
		// Check the valid target
		CString selectedLabel = protocolTable.GetItemText(protocolTable.GetFocusCell().row, 0);

		if (selectedLabel.Compare(L"GOTO") == 0 || selectedLabel.Compare(L"SHOT") == 0) {
			AfxMessageBox(L"Please select the number label.");
			return;
		}

		// Check the already exist GOTO label
		for (int i = protocolTable.GetFocusCell().row; i < currentRowCount; ++i) {
			CString checkLabel = protocolTable.GetItemText(i, 0);

			if (checkLabel.Compare(L"GOTO") == 0) {
				AfxMessageBox(L"Duplicated GOTO state. Please select the correct label.");
				return;
			}
		}

		label = L"GOTO";
		temp = selectedLabel;
		time = L"5";	// Default value
	}
	else if (protocolType == 2) {	// SHOT
		// Check the already has the shot
		for (int i = 0; i<currentProtocol.actionList.size(); ++i) {
			if (currentProtocol.actionList[i].Label.Compare(L"SHOT") == 0) {
				AfxMessageBox(L"SHOT is already exist.");
				return;
			}
		}

		label = L"SHOT";
		temp = L"";
		time = L"";
	}

	// Add the protocol
	Action action;
	action.Label = label;
	action.Temp = _ttoi(temp);
	action.Time = _ttoi(time);

	currentProtocol.actionList.push_back(action);

	// Add the new grid data
	protocolTable.SetRowCount(currentRowCount + 1);

	for (int j = 0; j < 3; ++j) {
		GV_ITEM item;
		item.mask = GVIF_TEXT | GVIF_FORMAT;
		item.row = currentRowCount;
		item.col = j;
		item.nFormat = DT_LEFT | DT_WORDBREAK;

		if (j == 0)
			item.strText.Format(label);
		else if (j == 1) 
			item.strText.Format(temp);
		
		else if (j == 2) 
			item.strText.Format(time);
		

		protocolTable.SetItem(&item);
	}

	// Add the read only option for GOTO SHOT
	if (label.Compare(L"GOTO") == 0) {
		protocolTable.SetItemState(currentRowCount, 1, protocolTable.GetItemState(currentRowCount, 1) | GVIS_READONLY);
	}
	else if (label.Compare(L"SHOT") == 0) {
		protocolTable.SetItemState(currentRowCount, 1, protocolTable.GetItemState(currentRowCount, 1) | GVIS_READONLY);
		protocolTable.SetItemState(currentRowCount, 2, protocolTable.GetItemState(currentRowCount, 2) | GVIS_READONLY);
	}
}

static const int PROTOCOL_TABLE_COLUMN_WIDTHS[3] = { 133, 133, 132 };

void CProtocolEditorDialog::initProtocolTable() {
	protocolTable.SetListMode(true);

	protocolTable.DeleteAllItems();

	protocolTable.SetSingleRowSelection();
	protocolTable.SetSingleColSelection();
	protocolTable.SetColumnCount(3);
	protocolTable.SetFixedRowCount(1);
	protocolTable.SetFixedColumnCount(1);
	protocolTable.SetEditable(true);
	protocolTable.SetColumnResize(false);

	// 초기 gridControl 의 table 값들을 설정해준다.
	DWORD dwTextStyle = DT_RIGHT | DT_VCENTER | DT_SINGLELINE;

	for (int col = 0; col < protocolTable.GetColumnCount(); col++) {
		GV_ITEM item;
		item.mask = GVIF_TEXT | GVIF_FORMAT;
		item.row = 0;
		item.col = col;

		if (col >= 0) {
			item.nFormat = DT_LEFT | DT_WORDBREAK;
			item.strText = PROTOCOL_TABLE_COLUMNS[col];
		}

		protocolTable.SetItem(&item);
		protocolTable.SetColumnWidth(col, PROTOCOL_TABLE_COLUMN_WIDTHS[col]);
	}

	protocolTable.SetRowCount(currentProtocol.actionList.size()+1);

	// Add the default item if there is no item exist.
	for (int i = 0; i < currentProtocol.actionList.size(); ++i) {
		for (int j = 0; j < 3; ++j) {
			GV_ITEM item;
			item.mask = GVIF_TEXT | GVIF_FORMAT;
			item.row = i + 1;
			item.col = j;
			item.nFormat = DT_LEFT | DT_WORDBREAK;

			if (j == 0) {
				// if the label is number
				if (!(currentProtocol.actionList[i].Label.Compare(L"GOTO") == 0 || currentProtocol.actionList[i].Label.Compare(L"SHOT") == 0)) {
					currentLabel = _ttoi(currentProtocol.actionList[i].Label);
				}
				item.strText.Format(currentProtocol.actionList[i].Label);
			}
			else if (j == 1) {
				if (currentProtocol.actionList[i].Label.Compare(L"SHOT") == 0) {
					item.strText = L"";
				}
				else {
					item.strText.Format(L"%.f", currentProtocol.actionList[i].Temp);
				}
			}
			else if (j == 2) {
				if (currentProtocol.actionList[i].Label.Compare(L"SHOT") == 0) {
					item.strText = L"";
				}
				else {
					item.strText.Format(L"%.f", currentProtocol.actionList[i].Time);
				}
			}

			protocolTable.SetItem(&item);
		}
	}
}