
// PlayDlg.h: 头文件
//

#pragma once

#include "PlayInitSDKConfigDlg.h"
#include "PlayLoginRoomConfigDlg.h"
#include "PlayConfigDlg.h"
#include "PlayBaseConfigDlg.h"
#include "PlayRealDataDlg.h"

#include "Play/PlayDemo.h"
#include "AppSupport/ZGUIOperator.h"
#include "AppSupport/ZGLog.h"

// CPlayDlg 对话框
class CPlayDlg : public CDialogEx,
    public PlayInitSDKConfigDlgEvent,
    public PlayLoginRoomConfigDlgEvent,
    public PlayConfigDlgEvent,
    public PlayBaseConfigDlgEvent,
    public PlayRealDataDlgEvent,
    public ILogToView
{
// 构造
public:

    static CPlayDlg * CreateDlgInstance(CWnd * pParent);

    void InitDlg();

    void HideAllConfigDlg();
    // 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PLAY_DIALOG };
#endif


    virtual void OnInitSDKButtonEvent() override;


    virtual void OnLoginRoomButtonEvent() override;


    virtual void OnPlayButtonEvent() override;


    virtual void OnBaseFunctionButtonEvent(bool show) override;


    virtual void OnViewModeChaned(int view_mode_index) override;


    virtual void OnPlayMuted(bool enable_mute) override;


    virtual void OnSetPlayVolume(int vol) override;


    virtual void OnRealDataBaseFunctionButtonEvent(bool show) override;


    virtual void PrintToView(const char * log_str) override;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持



    virtual void OnOK() override;


    virtual void OnCancel() override;

    // 实现
protected:
    CPlayDlg(CWnd* pParent = nullptr);	// 标准构造函数

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	
    afx_msg void OnPaint();

    void ShowLoginRoomConfig();
    void ShowPlayConfig();
    void ShowRealDataDlg();
    void ShowBaseConfig();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

    afx_msg LRESULT OnUICallbackMsg(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

    void SetHScroll();
    void AddLogToList(std::string str_log);

    void OnPlayDemoStatus(ZGPlayStatus status);

    void OnPlayRealDataRefresh(const PlayRealDataInfo & real_data);

    CListBox log_list_;

    PlayDemo play_demo_;

    PlayInitSDKConfigDlg init_sdk_config_dlg_;
    PlayLoginRoomConfigDlg login_room_config_dlg_;
    PlayConfigDlg play_config_dlg_;
    PlayBaseConfigDlg base_config_dlg_;
    PlayRealDataDlg real_data_dlg_;

    bool play_success_ = false;

    BOOL m_bFullSceen = FALSE;//全屏标志
    CWnd* m_pWndSave;//保存父窗口指针
    WINDOWPLACEMENT m_wpmSave;//保存窗口位置信息

public:
    afx_msg void OnStnClickedStaticPlayTopicVideo();
};
