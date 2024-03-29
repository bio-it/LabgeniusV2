#pragma once


// SetupDialog 대화 상자
#include ".\gridctrl_src\gridctrl.h"

class SetupDialog : public CDialogEx
{
	DECLARE_DYNAMIC(SetupDialog)
private:
	CGridCtrl pidTable;
	CListBox protocolList;

	void initConstants();
	void initPidTable();
	void loadProtocolList();

	int maxCycles;
	int compensation;
	float integralMax;
	float displayDelta;
	float flRelativeMax;

	vector<PID> pids;
	vector<Protocol> protocols;

public:
	SetupDialog(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~SetupDialog();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SETUP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSaveChanges();
	afx_msg void OnBnClickedButtonAddProtocol();
	afx_msg void OnBnClickedButtonEditProtocol();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
