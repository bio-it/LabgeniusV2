// PasswordDialog.cpp: 구현 파일
//

#include "stdafx.h"
#include "PasswordDialog.h"
#include "afxdialogex.h"
#include "resource.h"

// CPasswordDialog 대화 상자

IMPLEMENT_DYNAMIC(CPasswordDialog, CDialogEx)

CPasswordDialog::CPasswordDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_PASSWORD, pParent)
{

}

CPasswordDialog::~CPasswordDialog()
{
}

void CPasswordDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPasswordDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_PASSWORD_OK, &CPasswordDialog::OnBnClickedButtonPasswordOk)
	ON_BN_CLICKED(IDC_BUTTON_PASSWORD_CANCEL, &CPasswordDialog::OnBnClickedButtonPasswordCancel)
END_MESSAGE_MAP()


// CPasswordDialog 메시지 처리기


void CPasswordDialog::OnBnClickedButtonPasswordOk()
{
	// Check the password is correct.
	CString password;
	GetDlgItemText(IDC_EDIT_PASSWORD_INPUT, password);

	if (password.Compare(L"biomedux") == 0) {
		OnOK();
	}
	else {
		AfxMessageBox(L"Incorrect password!");
	}
}


void CPasswordDialog::OnBnClickedButtonPasswordCancel()
{
	OnCancel();
}
