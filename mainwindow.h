#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Windows.h>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <TlHelp32.h>
#include <ShlObj.h>
typedef  NTSTATUS (*pAdjustPriv) (ULONG Privilege,
                         BOOLEAN Enable,
                         BOOLEAN CurrentThread,
                         PBOOLEAN Enabled);
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_actionAbout_Tool_triggered();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QString selectedFileName;
    DWORD winLogonPid = 0;
    void AutoElevate();
};
#endif // MAINWINDOW_H
