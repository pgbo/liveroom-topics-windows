#pragma once

#include <string>

#include "AppSupport/ZGUIOperator.h"

#include "Publish/PublishDemo.h"


class RealDataDlgEvent
{
public:
    virtual void OnRealDataBaseFunctionButtonEvent(bool show) = 0;

};

class RealDataDlg : public CDialogEx
{
	DECLARE_DYNAMIC(RealDataDlg)

public:
	RealDataDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~RealDataDlg();

    void SetEventCallBack(RealDataDlgEvent * cb);

    void UpdateRealData(const RealDataInfo & info);

    void InitShowRealData();


// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REAL_DATA_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    afx_msg LRESULT OnUICallbackMsg(WPARAM wParam, LPARAM lParam);

    RealDataDlgEvent * event_cb_ = nullptr;
    bool show_base_fun_ = false;

	DECLARE_MESSAGE_MAP()

    virtual void OnOK() override;


    virtual void OnCancel() override;

public:
    afx_msg void OnBnClickedButtonPublishTopicBaseFun2();
};
