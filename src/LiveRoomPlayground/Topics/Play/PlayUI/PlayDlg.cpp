
// PlayDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "PlayDlg.h"
#include "afxdialogex.h"

#include "Play/PlayUI/resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPlayDlg 对话框


CPlayDlg::CPlayDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PLAY_DIALOG, pParent)
{
}

CPlayDlg * CPlayDlg::CreateDlgInstance(CWnd * pParent)
{
    CPlayDlg * p = new CPlayDlg(pParent);
    p->Create(IDD_PLAY_DIALOG, pParent);
    return p;
}

void CPlayDlg::OnInitSDKButtonEvent()
{

    play_config_dlg_.InitDlgData();
    real_data_dlg_.InitShowRealData();
    base_config_dlg_.InitDefaultConfig();

    play_demo_.SetStateUpdateObserver(std::bind(&CPlayDlg::OnPlayDemoStatus, this, std::placeholders::_1));
    play_demo_.SetRealDataRefreshObserver(std::bind(&CPlayDlg::OnPlayRealDataRefresh, this, std::placeholders::_1));

    AddLogToList("开始初始化sdk");
    play_demo_.InitSDK(init_sdk_config_dlg_.GetConfig());
}

void CPlayDlg::OnLoginRoomButtonEvent()
{
    AddLogToList("开始登录房间");
    play_demo_.LoginRoom(login_room_config_dlg_.GetConfig());
}

void CPlayDlg::OnPlayButtonEvent()
{
    AddLogToList("开始拉流");

    PlayConfig play_config = play_config_dlg_.GetConfig();
    play_config.view_hwnd = GetDlgItem(IDC_STATIC_PLAY_TOPIC_VIDEO)->GetSafeHwnd();
    play_demo_.StartPlay(play_config);
}

void CPlayDlg::OnBaseFunctionButtonEvent(bool show)
{
    if (show)
    {
        CRect rect;
        GetDlgItem(IDC_STATIC_PLAY_TOPIC_CONFIG_PANNEL)->GetWindowRect(rect);
        ScreenToClient(&rect);
        base_config_dlg_.MoveWindow(rect.left + rect.Width() / 2, rect.top, rect.Width() / 2, rect.Height());
        base_config_dlg_.ShowWindow(SW_SHOW);
    }
    else {
        base_config_dlg_.ShowWindow(SW_HIDE);
    }
}

void CPlayDlg::OnRealDataBaseFunctionButtonEvent(bool show)
{
    if (show)
    {
        CRect rect;
        GetDlgItem(IDC_STATIC_PLAY_TOPIC_CONFIG_PANNEL)->GetWindowRect(rect);
        ScreenToClient(&rect);
        base_config_dlg_.MoveWindow(rect.left + rect.Width() / 2, rect.top, rect.Width() / 2, rect.Height());
        base_config_dlg_.ShowWindow(SW_SHOW);
    }
    else {
        base_config_dlg_.ShowWindow(SW_HIDE);
    }
}

void CPlayDlg::PrintToView(const char * log_str)
{
    AddLogToList(log_str);
}

void CPlayDlg::OnViewModeChaned(int view_mode_index)
{
    ZGConfigHelperInstance()->SetPlayViewMode((ZEGO::LIVEROOM::ZegoVideoViewMode)view_mode_index, play_config_dlg_.GetConfig().stream_id);
}

void CPlayDlg::OnPlayMuted(bool enable_mute)
{
    ZGConfigHelperInstance()->EnableSpeaker(!enable_mute);
}

void CPlayDlg::OnSetPlayVolume(int vol)
{
    ZGConfigHelperInstance()->SetPlayVolume(vol);
}


void CPlayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_PLAY_TOPIC_LOG, log_list_);
}

void CPlayDlg::OnOK()
{
}

void CPlayDlg::OnCancel()
{
}

BEGIN_MESSAGE_MAP(CPlayDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_CREATE()
    ON_MESSAGE(UI_CALLBACK_MSG, &CPlayDlg::OnUICallbackMsg)
    ON_STN_CLICKED(IDC_STATIC_PLAY_TOPIC_VIDEO, &CPlayDlg::OnStnClickedStaticPlayTopicVideo)
END_MESSAGE_MAP()


// CPlayDlg 消息处理程序

BOOL CPlayDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPlayDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


void CPlayDlg::ShowLoginRoomConfig()
{
    init_sdk_config_dlg_.ShowWindow(SW_HIDE);

    CRect rect;
    GetDlgItem(IDC_STATIC_PLAY_TOPIC_CONFIG_PANNEL)->GetWindowRect(rect);
    ScreenToClient(&rect);
    login_room_config_dlg_.MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
    login_room_config_dlg_.ShowWindow(SW_SHOW);
}

void CPlayDlg::ShowPlayConfig()
{
    login_room_config_dlg_.ShowWindow(SW_HIDE);

    CRect rect;
    GetDlgItem(IDC_STATIC_PLAY_TOPIC_CONFIG_PANNEL)->GetWindowRect(rect);
    ScreenToClient(&rect);
    play_config_dlg_.MoveWindow(rect.left, rect.top, rect.Width() / 2, rect.Height());
    play_config_dlg_.ShowWindow(SW_SHOW);
}

void CPlayDlg::ShowRealDataDlg()
{
    play_config_dlg_.ShowWindow(SW_HIDE);
    base_config_dlg_.ShowWindow(SW_HIDE);

    CRect rect;
    GetDlgItem(IDC_STATIC_PLAY_TOPIC_CONFIG_PANNEL)->GetWindowRect(rect);
    ScreenToClient(&rect);
    real_data_dlg_.MoveWindow(rect.left, rect.top, rect.Width() / 2, rect.Height());
    real_data_dlg_.ShowWindow(SW_SHOW);
}

void CPlayDlg::ShowBaseConfig()
{
    play_config_dlg_.ShowWindow(SW_HIDE);

    CRect rect;
    GetDlgItem(IDC_STATIC_PLAY_TOPIC_CONFIG_PANNEL)->GetWindowRect(rect);
    ScreenToClient(&rect);
    base_config_dlg_.MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
    base_config_dlg_.ShowWindow(SW_SHOW);
}

void CPlayDlg::InitDlg()
{
    log_list_.ResetContent();
    SendDlgItemMessage(IDC_LIST_PLAY_TOPIC_LOG, LB_SETHORIZONTALEXTENT, (WPARAM)0, 0);
    HideAllConfigDlg();

    CRect rect;
    GetDlgItem(IDC_STATIC_PLAY_TOPIC_CONFIG_PANNEL)->GetWindowRect(rect);
    ScreenToClient(&rect);
    init_sdk_config_dlg_.MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
    init_sdk_config_dlg_.ShowWindow(SW_SHOW);

    play_demo_.SetViewLogCallBack(this);
    ZGConfigHelperInstance()->SetViewLogCallBack(this);

    play_success_ = false;

}

void CPlayDlg::HideAllConfigDlg()
{
    init_sdk_config_dlg_.ShowWindow(SW_HIDE);
    login_room_config_dlg_.ShowWindow(SW_HIDE);
    play_config_dlg_.ShowWindow(SW_HIDE);
    base_config_dlg_.ShowWindow(SW_HIDE);
    real_data_dlg_.ShowWindow(SW_HIDE);
}

int CPlayDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialogEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  在此添加您专用的创建代码

    init_sdk_config_dlg_.Create(IDD_PLAY_INITSDK_CONFIG_DLG, this);
    init_sdk_config_dlg_.SetEventCallBack(this);

    login_room_config_dlg_.Create(IDD_PLAY_LOGINROOM_CONFIG_DLG, this);
    login_room_config_dlg_.SetEventCallBack(this);

    play_config_dlg_.Create(IDD_PLAY_CONFIG_DLG, this);
    play_config_dlg_.SetEventCallBack(this);

    base_config_dlg_.Create(IDD_PLAY_BASE_FUN_DLG, this);
    base_config_dlg_.SetEventCallBack(this);

    real_data_dlg_.Create(IDD_PLAY_REAL_DATA_DLG, this);
    real_data_dlg_.SetEventCallBack(this);

    return 0;
}

afx_msg LRESULT CPlayDlg::OnUICallbackMsg(WPARAM wParam, LPARAM lParam)
{
    PostUIData * pdata = (PostUIData *)wParam;
    if (pdata != nullptr && pdata->cb_in_ui_fun)
    {
        pdata->cb_in_ui_fun();
    }
    return 0;
}

void CPlayDlg::SetHScroll()
{
    CDC* dc = GetDC();
    SIZE s;
    int index;
    CString str;
    long temp;
    for (index = 0; index < log_list_.GetCount(); index++)
    {
        log_list_.GetText(index, str);
        s = dc->GetTextExtent(str, str.GetLength() + 1);
        // 获取字符串的像素大小
        // 如果新的字符串宽度大于先前的水平滚动条宽度，则重新设置滚动条宽度
        // 得到滚动条的宽度
        temp = (long)SendDlgItemMessage(IDC_LIST_PLAY_TOPIC_LOG, LB_GETHORIZONTALEXTENT, 0, 0);
        if (s.cx > temp)
        {
            SendDlgItemMessage(IDC_LIST_PLAY_TOPIC_LOG, LB_SETHORIZONTALEXTENT, (WPARAM)s.cx, 0);
        }
    }
    ReleaseDC(dc);
}

void CPlayDlg::AddLogToList(std::string str_log)
{
    PostUIData * pdata = CreateUIData();
    pdata->cb_in_ui_fun = [str_log, pdata, this]()->void
    {
        CStringA str;
        str.Format("%s", str_log.c_str());
        log_list_.AddString(CString(str));
        SetHScroll();

        DestroyUIData(pdata);
    };

    PostMsgDataToUI(pdata);

}

void CPlayDlg::OnPlayDemoStatus(ZGPlayStatus status)
{
    PostUIData * pdata = CreateUIData();
    pdata->cb_in_ui_fun = [status, pdata, this]()->void
    {
        switch (status)
        {
        case kZGPlayStatus_Init_SDK_OK:
            AddLogToList("初始化sdk成功");
            ShowLoginRoomConfig();
            break;

        case kZGPlayStatus_Login_OK:
            AddLogToList("登录房间成功");
            ShowPlayConfig();

            break;
        case kZGPlayStatus_Playing:
            AddLogToList("拉流成功");
            ShowRealDataDlg();
            play_success_ = true;
            break;
        default:
            break;
        }

        DestroyUIData(pdata);
    };

    PostMsgDataToUI(pdata);
}

void CPlayDlg::OnPlayRealDataRefresh(const PlayRealDataInfo & real_data)
{
    real_data_dlg_.UpdateRealData(real_data);
}


void CPlayDlg::OnStnClickedStaticPlayTopicVideo()
{
    if(!play_success_)
        return;

    static CRect last_rect;
    if (m_bFullSceen)//恢复
    {
        m_pWndSave->ShowWindow(SW_SHOW);
        SetParent(m_pWndSave);
        ModifyStyle(WS_POPUP, WS_CHILD);
        SetWindowPlacement(&m_wpmSave);//还原
        m_bFullSceen = FALSE;

        ::MoveWindow(GetDlgItem(IDC_STATIC_PLAY_TOPIC_VIDEO)->GetSafeHwnd(), last_rect.left, last_rect.top, last_rect.right - last_rect.left, last_rect.bottom - last_rect.top, FALSE);

        real_data_dlg_.ShowWindow(SW_SHOW);
        GetDlgItem(IDC_LIST_PLAY_TOPIC_LOG)->ShowWindow(SW_SHOW);
    }
    else//全屏
    {
        GetDlgItem(IDC_STATIC_PLAY_TOPIC_VIDEO)->GetClientRect(&last_rect);
        

        int cx = ::GetSystemMetrics(SM_CXSCREEN);
        int cy = ::GetSystemMetrics(SM_CYSCREEN);//得到屏幕大小
        GetWindowPlacement(&m_wpmSave);//保存屏幕位置
        ModifyStyle(WS_CHILD, WS_POPUP);//修改窗口的风格
        m_pWndSave = SetParent(NULL);//设置父窗口为NULL,返回原父窗口
        m_pWndSave->ShowWindow(SW_HIDE);//原父窗口 隐藏
        MoveWindow(0, 0, cx, cy);//移动窗口
        ::MoveWindow(GetDlgItem(IDC_STATIC_PLAY_TOPIC_VIDEO)->GetSafeHwnd(), 0, 0, cx, cy, FALSE);
        m_bFullSceen = TRUE;

        real_data_dlg_.ShowWindow(SW_HIDE);
        GetDlgItem(IDC_LIST_PLAY_TOPIC_LOG)->ShowWindow(SW_HIDE);
    }
}
