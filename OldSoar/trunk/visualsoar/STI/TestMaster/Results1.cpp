// Results1.cpp : implementation file
//

#include "stdafx.h"
#include "TestMaster.h"
#include "Results1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTResults1 dialog


CTResults1::CTResults1(CWnd* pParent /*=NULL*/)
	: CDialog(CTResults1::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTResults1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTResults1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTResults1)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTResults1, CDialog)
	//{{AFX_MSG_MAP(CTResults1)
	ON_BN_CLICKED(IDC_WINDIFF1_RUNTIME1, OnWindiff1Runtime1)
	ON_BN_CLICKED(IDC_WINDIFF1_RUNTIME2, OnWindiff1Runtime2)
	ON_BN_CLICKED(IDC_WINDIFF1_TOOL1, OnWindiff1Tool1)
	ON_BN_CLICKED(IDC_WINDIFF2_RUNTIME1, OnWindiff2Runtime1)
	ON_BN_CLICKED(IDC_WINDIFF2_RUNTIME2, OnWindiff2Runtime2)
	ON_BN_CLICKED(IDC_WINDIFF2_TOOL1, OnWindiff2Tool1)
	ON_BN_CLICKED(IDC_WINDIFF3_RUNTIME1, OnWindiff3Runtime1)
	ON_BN_CLICKED(IDC_WINDIFF3_RUNTIME2, OnWindiff3Runtime2)
	ON_BN_CLICKED(IDC_WINDIFF3_TOOL1, OnWindiff3Tool1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTResults1 message handlers

// Decide if these two files are identical.
static BOOL CheckForErrors(TCHAR const* pFile)
{
	TCHAR path1[_MAX_PATH*2] ;

	GetFilePath(path1, sizeof(path1), pFile) ;

	// Put files on the stack so they're closed on leaving function
	CFile file1 ;

	BOOL ok1 = file1.Open(path1, CFile::modeRead | CFile::shareDenyWrite) ;

	// If we can't open the file, that's an error
	if (!ok1)
	{
		return TRUE ;
	}

	const long kBufferSize = 1024 ;
	TCHAR buffer1[kBufferSize] ;
	BOOL  bDone = FALSE ;

	while (!bDone)
	{
		UINT read1 = file1.Read(buffer1, kBufferSize) ;

		// If the buffer contains "Error:" then report the error
		// BUGBUG: Technically we could miss an error if it happens
		// to cross the boundary between one read buffer and another.
		// There's less than a 1% chance of this so I'm happy to take the risk.
		if (strstr(buffer1, "Error:") != NULL)
			return TRUE ;

		// We're finished with the files if we read 0 bytes last time
		// Check here--after checking read1 against read2 so we
		// know both files are finished at the same point.
		bDone = (read1 <= 0) ;
	}

	file1.Close() ;

	// No errors found
	return FALSE ;
}

// Decide if these two files are identical.
static BOOL CompareFiles(TCHAR const* pFile1, TCHAR const* pFile2)
{
	TCHAR path1[_MAX_PATH*2] ;
	TCHAR path2[_MAX_PATH*2] ;

	GetFilePath(path1, sizeof(path1), pFile1) ;
	GetFilePath(path2, sizeof(path2), pFile2) ;

	// Put files on the stack so they're closed on leaving function
	CFile file1 ;
	CFile file2 ;

	BOOL ok1 = file1.Open(path1, CFile::modeRead | CFile::shareDenyWrite) ;
	BOOL ok2 = file2.Open(path2, CFile::modeRead | CFile::shareDenyWrite) ;

	// If we can't open the files, they're different
	if (!ok1 || !ok2)
	{
		return FALSE ;
	}

	const long kBufferSize = 512 ;
	TCHAR buffer1[kBufferSize] ;
	TCHAR buffer2[kBufferSize] ;
	BOOL  bDone = FALSE ;

	while (!bDone)
	{
		UINT read1 = file1.Read(buffer1, kBufferSize) ;
		UINT read2 = file2.Read(buffer2, kBufferSize) ;

		// If the file lengths are different, the files are different
		if (read1 != read2)
			return FALSE ;

		// If the buffers are different, the files are different
		if (memcmp(buffer1, buffer2, read1) != 0)
			return FALSE ;

		// We're finished with the files if we read 0 bytes last time
		// Check here--after checking read1 against read2 so we
		// know both files are finished at the same point.
		bDone = (read1 <= 0) ;
	}

	file1.Close() ;
	file2.Close() ;

	return TRUE ;
}

static BOOL Unpack(TCHAR const* pType, UINT id, TCHAR const* pOutputFilename)
{
	// Find the resource
	HRSRC hRes = FindResource(AfxGetResourceHandle(), (LPTSTR)id, pType) ;

	if (!hRes)
		return FALSE ;

	// Load up the resource
	HGLOBAL hResource = LoadResource(AfxGetResourceHandle(), hRes) ;
	DWORD   size      = SizeofResource(AfxGetResourceHandle(), hRes) ;

	if (!hResource)
		return FALSE ;

	TCHAR outputPath[_MAX_PATH*2] ;
	GetFilePath(outputPath, sizeof(outputPath), pOutputFilename) ;

	// Read and save the resource to disk
	BYTE* pData = (BYTE*)LockResource(hResource) ;

	CFile output ;
	output.Open(outputPath, CFile::modeWrite | CFile::modeCreate | CFile::shareDenyWrite) ;
	output.Write(pData, size) ;
	output.Close() ;

	FreeResource(hResource) ;

	return TRUE ;
}

static BOOL LaunchWinDiff(TCHAR const* pFile1, TCHAR const* pFile2)
{
	TCHAR path1[_MAX_PATH*2] ;
	TCHAR path2[_MAX_PATH*2] ;
	TCHAR winDiff[_MAX_PATH*2] ;

	GetFilePath(path1, sizeof(path1), pFile1) ;
	GetFilePath(path2, sizeof(path2), pFile2) ;

	GetFilePath(winDiff, sizeof(winDiff), "WinDiff.exe") ;

	TCHAR commandLine[_MAX_PATH*6] ;
	lstrcpy(commandLine, winDiff) ;
	lstrcat(commandLine, " \"") ;
	lstrcat(commandLine, path1) ;
	lstrcat(commandLine, "\" \"") ;
	lstrcat(commandLine, path2) ;
	lstrcat(commandLine, "\"") ;

	// Try to run windiff
	UINT res = WinExec(commandLine, SW_SHOW) ;

	// If windiff fails, assume it's because windiff is missing.
	// Unpack it from our resources and try again.
	if (res <= 31)
	{
		Unpack("EXE",IDR_WINDIFF_EXE, "WinDiff.exe") ;
		Unpack("DLL",IDR_GUTILS_DLL, "GUtils.dll") ;

		// Try windiff again
		res = WinExec(commandLine, SW_SHOW) ;
	}

	return (res <= 31) ;
}

static void ShowResults(CWnd* pTextWnd, TCHAR const* pTestFile, TCHAR const* pBaseFile)
{
	BOOL bSame1  = CompareFiles(pTestFile, pBaseFile) ;
	BOOL bError1 = CheckForErrors(pTestFile) ;

	pTextWnd->SetWindowText(bError1 ? "*Error*" : (bSame1 ? "Same" : "Different")) ;
}

BOOL CTResults1::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Test 1
	ShowResults(GetDlgItem(IDC_RES1_1), "Test1_Runtime1.txt", "Base1_Runtime1.txt") ;
	ShowResults(GetDlgItem(IDC_RES1_2), "Test1_Runtime2.txt", "Base1_Runtime2.txt") ;
	ShowResults(GetDlgItem(IDC_RES1_3), "Test1_Tool1.txt", "Base1_Tool1.txt") ;

	// Test 2
	ShowResults(GetDlgItem(IDC_RES2_1), "Test2_Runtime1.txt", "Base2_Runtime1.txt") ;
	ShowResults(GetDlgItem(IDC_RES2_2), "Test2_Runtime2.txt", "Base2_Runtime2.txt") ;
	ShowResults(GetDlgItem(IDC_RES2_3), "Test2_Tool1.txt", "Base2_Tool1.txt") ;

	// Test 3
	ShowResults(GetDlgItem(IDC_RES3_1), "Test3_Runtime1.txt", "Base3_Runtime1.txt") ;
	ShowResults(GetDlgItem(IDC_RES3_2), "Test3_Runtime2.txt", "Base3_Runtime2.txt") ;
	ShowResults(GetDlgItem(IDC_RES3_3), "Test3_Tool1.txt", "Base3_Tool1.txt") ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTResults1::OnWindiff1Runtime1() 
{
	LaunchWinDiff("Test1_Runtime1.txt", "Base1_Runtime1.txt") ;
}

void CTResults1::OnWindiff1Runtime2() 
{
	LaunchWinDiff("Test1_Runtime2.txt", "Base1_Runtime2.txt") ;	
}

void CTResults1::OnWindiff1Tool1() 
{
	LaunchWinDiff("Test1_Tool1.txt", "Base1_Tool1.txt") ;	
}

void CTResults1::OnWindiff2Runtime1() 
{
	LaunchWinDiff("Test2_Runtime1.txt", "Base2_Runtime1.txt") ;
}

void CTResults1::OnWindiff2Runtime2() 
{
	LaunchWinDiff("Test2_Runtime2.txt", "Base2_Runtime2.txt") ;
}

void CTResults1::OnWindiff2Tool1() 
{
	LaunchWinDiff("Test2_Tool1.txt", "Base2_Tool1.txt") ;	
}

void CTResults1::OnWindiff3Runtime1() 
{
	LaunchWinDiff("Test3_Runtime1.txt", "Base3_Runtime1.txt") ;
}

void CTResults1::OnWindiff3Runtime2() 
{
	LaunchWinDiff("Test3_Runtime2.txt", "Base3_Runtime2.txt") ;
}

void CTResults1::OnWindiff3Tool1() 
{
	LaunchWinDiff("Test3_Tool1.txt", "Base3_Tool1.txt") ;	
}
