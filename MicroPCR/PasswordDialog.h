#pragma once


// CPasswordDialog 대화 상자

class CPasswordDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CPasswordDialog)

public:
	CPasswordDialog(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CPasswordDialog();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PASSWORD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonPasswordOk();
	afx_msg void OnBnClickedButtonPasswordCancel();
};
