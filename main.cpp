#include <QtGlobal>
#include <windows.h>
#include <winuser.h>
#include <vector>
#include "../rizhi/rizhi.h"
#include "../baikePub/BaikeTypeData.h"
#include <iostream>
#include <QTextCodec>
using namespace std;

vector<int> hotkeyid;//存储全部热键信息
DWORD qzdPid=0;      //保存的已经启动的“全自动”的进程id

void zhucerejian()
{
    struct mvk{QString idstr; uint mod;uint vk;};
    mvk modvk[]={
        {"toclose",0,VK_F6}
    };
    for ( mvk& mv : modvk )
    {
        int x=GlobalAddAtom(mv.idstr.toStdWString().c_str())- 0xC000;
        bool jg=RegisterHotKey(NULL,x,mv.mod,mv.vk);
        if(!jg){
            snd<<"reghoterr,err="<<GetLastError()<<","<<mv.idstr;
        }
        hotkeyid.push_back(x);
    }
}
void shifang()
{
    for(int&id:hotkeyid)
    {
        bool jg=UnregisterHotKey(NULL, id);
        if(!jg){
            snd<<"unreg err="<<GetLastError()<<",id="<<id;
        }
    }
}
BOOL WINAPI ConsoleCtrlhandler(DWORD dwCtrlType)
{
    sow<<QString("直接点击关闭按钮callConsoleCtrlhandler");
    if(dwCtrlType == CTRL_CLOSE_EVENT)
    {
        snd<<"closeEvent";
        shifang();
    }
    return TRUE;
}

void closeProcess(){
    HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, qzdPid);
    if( hProcess == NULL ){
        sow<<"OpenProcess";
    }else{
        bool jg=TerminateProcess(hProcess,0);
        if(!jg){
            int id=GetLastError();
            sow<<"guanbi shibai,err="<<id;
        }
    }
}

int main(int argc, char *argv[])
{
    bool istest=false;
    bool isTihuanguanbi=false;

    //防止重复启动，只留一个进程
    HANDLE hMutex = NULL;
    hMutex = CreateMutex(NULL, FALSE, L"gbbk");
    DWORD dwRet = GetLastError();
    if (hMutex)
    {
        if (ERROR_ALREADY_EXISTS == dwRet)
        {
            istest=true;
            CloseHandle(hMutex);  // should be closed
            return FALSE;
        }
    }
    else
    {
        sow<<" Create Mutex Error ," << dwRet;
    }
    //
    SetConsoleTitleW(istest?L"guanbi":L"关闭");//修改窗口标题
    SetConsoleCtrlHandler(ConsoleCtrlhandler, TRUE);

    //注册热键F6，用来关闭“自动修改工具”
    zhucerejian();

    //被其他进程启动时，第二个参数是“全自动”的进程id
    if(argc==2){
        qzdPid=atoi(argv[1]);
    }

    ShowWindow(GetConsoleWindow(),SW_MINIMIZE);


    MSG msg;
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
    while (GetMessage(&msg, NULL, 0, 0))
    {
        switch(msg.message)
        {
        case WM_USER+msgRebootSelf://重启自己
        case WM_USER+msgNewGbid://一个“全自动”关闭后再启动另一个；
            //一个“全自动”未关闭时又打开一个
            //msg.wParam=“全自动”当前的进程ID
            sow<<((msg.message==WM_USER+msgRebootSelf)?"rebootself":"gbzjzcqTihuanqidong");
            sow<<"newpid="<<msg.wParam<<",gbid="<<qzdPid;
            if(msg.wParam!=qzdPid){
                if(qzdPid!=0 && msg.message==WM_USER+msgNewGbid){
                    //已经打开一个“全自动”时，又打开了一个“全自动”
                    closeProcess();      //关闭之前的“全自动”
                    isTihuanguanbi=true; //记录这次替换
                }
                qzdPid=msg.wParam;
                Sleep(300);
                PostThreadMessageW(DWORD(msg.lParam),WM_USER+msgHotkey,0,0);
            }
            break;
        case WM_USER+msgCloseGbid:
            //收到此消息，表明“全自动”被人工关闭
            sow<<"rcv closeGbid,gbid="<<qzdPid;
            if(isTihuanguanbi){
                isTihuanguanbi=false;
                sow<<"Tihuan";
            }else{
                qzdPid=0;
                sow<<"notTihuan,gbid=0";
            }

            break;
        case WM_HOTKEY:
            //UINT ctrlalt=LOWORD(msg.lParam);
            //UINT anjian=HIWORD(msg.lParam);
            if(LOWORD(msg.lParam)==0 && HIWORD(msg.lParam)==VK_F6){
                closeProcess();
                sow<<"f6,gbid=0";
                qzdPid=0;
            }
            break;
        default:
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    //直接点击关闭按钮下面的不运行
    sow<<QString("按键退出结束msgloop以后");
    SetConsoleCtrlHandler(ConsoleCtrlhandler, FALSE);
    CloseHandle(hMutex);
    return 0;
}
