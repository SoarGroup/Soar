; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CTResults1
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "TestMaster.h"
LastPage=0

ClassCount=6
Class1=CTestMasterApp
Class2=CTestMasterDoc
Class3=CTestMasterView
Class4=CMainFrame

ResourceCount=3
Resource1=IDD_RESULTS1
Class5=CAboutDlg
Resource2=IDD_ABOUTBOX
Class6=CTResults1
Resource3=IDR_MAINFRAME

[CLS:CTestMasterApp]
Type=0
HeaderFile=TestMaster.h
ImplementationFile=TestMaster.cpp
Filter=N

[CLS:CTestMasterDoc]
Type=0
HeaderFile=TestMasterDoc.h
ImplementationFile=TestMasterDoc.cpp
Filter=N

[CLS:CTestMasterView]
Type=0
HeaderFile=TestMasterView.h
ImplementationFile=TestMasterView.cpp
Filter=C
BaseClass=CEditView
VirtualFilter=VWC
LastObject=IDM_TEST_ALL


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrame.h
ImplementationFile=MainFrame.cpp
Filter=T
LastObject=IDM_TEST_TEST3
BaseClass=CFrameWnd
VirtualFilter=fWC




[CLS:CAboutDlg]
Type=0
HeaderFile=TestMaster.cpp
ImplementationFile=TestMaster.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_PRINT
Command6=ID_FILE_PRINT_PREVIEW
Command7=ID_FILE_PRINT_SETUP
Command8=ID_FILE_MRU_FILE1
Command9=ID_APP_EXIT
Command10=ID_EDIT_UNDO
Command11=ID_EDIT_CUT
Command12=ID_EDIT_COPY
Command13=ID_EDIT_PASTE
Command14=ID_VIEW_TOOLBAR
Command15=ID_VIEW_STATUS_BAR
Command16=IDM_TEST_ALL
Command17=IDM_TEST_TEST1
Command18=IDM_TEST_TEST2
Command19=IDM_TEST_TEST3
Command20=ID_APP_ABOUT
CommandCount=20

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
CommandCount=8

[DLG:IDD_RESULTS1]
Type=1
Class=CTResults1
ControlCount=32
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_WINDIFF1_RUNTIME1,button,1342242816
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_WINDIFF1_RUNTIME2,button,1342242816
Control9=IDC_WINDIFF1_TOOL1,button,1342242816
Control10=IDC_RES1_1,static,1342308352
Control11=IDC_RES1_2,static,1342308352
Control12=IDC_RES1_3,static,1342308352
Control13=IDC_STATIC,button,1342177287
Control14=IDC_WINDIFF2_RUNTIME1,button,1342242816
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_WINDIFF2_RUNTIME2,button,1342242816
Control19=IDC_WINDIFF2_TOOL1,button,1342242816
Control20=IDC_RES2_1,static,1342308352
Control21=IDC_RES2_2,static,1342308352
Control22=IDC_RES2_3,static,1342308352
Control23=IDC_STATIC,button,1342177287
Control24=IDC_WINDIFF3_RUNTIME1,button,1342242816
Control25=IDC_STATIC,static,1342308352
Control26=IDC_STATIC,static,1342308352
Control27=IDC_STATIC,static,1342308352
Control28=IDC_WINDIFF3_RUNTIME2,button,1342242816
Control29=IDC_WINDIFF3_TOOL1,button,1342242816
Control30=IDC_RES3_1,static,1342308352
Control31=IDC_RES3_2,static,1342308352
Control32=IDC_RES3_3,static,1342308352

[CLS:CTResults1]
Type=0
HeaderFile=Results1.h
ImplementationFile=Results1.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_WINDIFF3_TOOL1

