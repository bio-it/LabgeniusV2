#pragma once


// CProtocolEditorDialog 대화 상자
#include ".\gridctrl_src\gridctrl.h"

class CProtocolEditorDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CProtocolEditorDialog)

private:
	CBitmap offImg;
	CBitmap famImg;
	CBitmap hexImg;
	CBitmap roxImg;
	CBitmap cy5Img;

	Protocol currentProtocol;
	MagnetoProtocol currentMagnetoProtocol;

	int protocolType;

	CGridCtrl protocolTable;
	int currentLabel;

	float ctFam;
	float ctHex;
	float ctRox;
	float ctCY5;

	bool isEdit;
	void initProtocolTable();
	CString checkProtocol(CString& protocol);

public:
	CProtocolEditorDialog(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CProtocolEditorDialog();

	// setting the mode
	void setProtocol(Protocol& protocol);

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROTOCOL_EDITOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonProtocolFilterFam();
	afx_msg void OnBnClickedButtonProtocolFilterHex();
	afx_msg void OnBnClickedButtonProtocolFilterRox();
	afx_msg void OnBnClickedButtonProtocolFilterCy5();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonProtocolSave();
	afx_msg void OnBnClickedButtonProtocolCancel();
	afx_msg void OnBnClickedButtonAddProtocol();
};
