#include "stdafx.h"
#include "VideoCommunicationDemo.h"

#include <algorithm>
#include <thread>

#include "AppSupport/ZGHelper.h"
#include "AppSupport/ZGManager.h"
#include "AppSupport/ZGConfigHelper.h"


// 190*190
const int kResolutionWidth = 190;
const int kResolutionHeight = 190;
const int kBitrate = 250000;//250kbps

VideoCommunicationDemo::VideoCommunicationDemo()
{
}

VideoCommunicationDemo::~VideoCommunicationDemo()
{
}

void VideoCommunicationDemo::SetDemoCallBack(VideoCommunicationCallBack * cb)
{
    video_communication_cb_ = cb;
}

void VideoCommunicationDemo::SetLocalVideoView(HWND view, LIVEROOM::ZegoVideoViewMode view_mode /*= LIVEROOM::ZegoVideoViewModeScaleAspectFit*/)
{
    local_video_view_ = view;
    local_view_mode = view_mode;
    
}

void VideoCommunicationDemo::SetRemoteVideoView(HWND view, ZegoUser for_user, LIVEROOM::ZegoVideoViewMode view_mode /*= LIVEROOM::ZegoVideoViewModeScaleAspectFit*/)
{
    PlayConfig config;
    if (GetPlayConfigByUser(for_user, config))
    {
        config.view_hwnd = view;
        config.view_mode = view_mode;
        UpdatePlayConfigByUser(for_user, config);

        LIVEROOM::StartPlayingStream(config.stream_id.c_str(), config.view_hwnd);
        LIVEROOM::SetViewMode(config.view_mode, config.stream_id.c_str());

    }
    else {
        PlayConfig play_config;
        play_config.view_hwnd = view;
        play_config.view_mode = view_mode;

        std::lock_guard<std::mutex> lock(play_config_map_mutex_);
        play_map_info_.insert(std::make_pair(for_user, play_config));
    }
}


void VideoCommunicationDemo::PrintLogToView(const char * format, ...)
{
    if (view_log_ != nullptr)
    {
        char log_buf[1024] = { 0 };
        va_list la;
        va_start(la, format);
        vsprintf_s(log_buf, format, la);
        va_end(la);

        view_log_->PrintToView(log_buf);
    }
}


void VideoCommunicationDemo::LoginRoom(string room_id)
{
    // 1.init sdk        初始化sdk
    // 2.login room      登录房间
    // 3.start publish   开始推流
    // 4.on stream added 收到流新增事件
    // 5.start playing   开始拉流

    room_id_ = room_id;

    bool is_test_env = true;
    PrintLogToView("使用%s环境", is_test_env ? "测试环境":"正式环境");

    LIVEROOM::SetUseTestEnv(is_test_env);

    string user_id = ZGHelper::Instance()->GetRandomUUID().c_str();
    string user_name = ZGHelper::Instance()->GetMachineName().c_str();

    PrintLogToView("设置userid = %s，username = %s", user_id.c_str(), user_name.c_str());
    LIVEROOM::SetUser(user_id.c_str(), user_name.c_str());

    // 配置观众可以创建房间，开启房间用户列表更新
    LIVEROOM::SetRoomConfig(true, true);

    LIVEROOM::SetRoomCallback(this);             // 房间回调注册
    LIVEROOM::SetLivePublisherCallback(this);    // 推流回调注册 
    LIVEROOM::SetLivePlayerCallback(this);       // 拉流回调注册
    LIVEROOM::SetDeviceStateCallback(this);      // 设备事件回调注册

    PrintLogToView("执行初始化");
    if (!ZGManagerInstance()->InitSdk())
    {
        UpdateLoginEvent(kZGVideoCommunicationLoginEventLoginFailed);
    }

}

void VideoCommunicationDemo::InitMainHwnd(HWND hwnd)
{
    main_hwnd_ = hwnd;
}

void VideoCommunicationDemo::SetLogCallBack(ILogToView *log_cb)
{
    view_log_ = log_cb;
}

void VideoCommunicationDemo::OnPlayStateUpdate(int stateCode, const char* pszStreamID)
{
    if (stateCode == 0)
    {
        PrintLogToView("拉流成功");
    }
    else {
        PrintLogToView("拉流状态更新，拉流失败，状态码 = %d",stateCode);
    }
}

void VideoCommunicationDemo::OnPlayQualityUpdate(const char* pszStreamID, ZegoPlayQuality playQuality)
{
    
}

void VideoCommunicationDemo::OnVideoSizeChanged(const char* pStreamID, int nWidth, int nHeight)
{
    
}

void VideoCommunicationDemo::OnAudioDeviceStateChanged(AV::AudioDeviceType deviceType, AV::DeviceInfo *deviceInfo, AV::DeviceState state)
{
    
}

void VideoCommunicationDemo::OnDeviceError(const char* deviceName, int errorCode)
{
    if (errorCode != 0)
    {
        string name = deviceName;
        if (name == "camera")
        {
            PrintLogToView("打开摄像头发生错误，请检查摄像头是否被占用");
        }else if (name == "microphone")
        {
            PrintLogToView("打开麦克风失败");
        }else if (name == "speaker")
        {
            PrintLogToView("打开扬声器失败");
        }
    }   
}

void VideoCommunicationDemo::LogoutRoom()
{
    LIVEROOM::StopPublishing();
    LIVEROOM::LogoutRoom();
    UpdateLoginEvent(kZGVideoCommunicationLoginEventLogout);
}

void VideoCommunicationDemo::OnInitSDK(int nError)
{
    if (nError == 0)
    {
        PrintLogToView("初始化sdk成功");

        // 初始化成功，切线程然后在开始登录房间
        PostUIData * pdata = CreateUIData();
        pdata->cb_in_ui_fun = [pdata, this]()->void
        {
            PrintLogToView("开始预览");

            // 设置采集分辨率和编码分辨率
            PrintLogToView("设置采集分辨率和编码分辨率,%dx%d", kResolutionWidth, kResolutionHeight);
            ZGConfigHelperInstance()->SetPublishResolution(kResolutionWidth, kResolutionHeight);

            LIVEROOM::SetPreviewView(local_video_view_);
            LIVEROOM::StartPreview();

            PrintLogToView("开始登录，room id = %s", room_id_.c_str());

            UpdateLoginEvent(kZGVideoCommunicationLoginEventLogin);
            start_retry_login_room_timestamp_ = -1;

            if (!LIVEROOM::LoginRoom(room_id_.c_str(), COMMON::Audience))
            {
                UpdateLoginEvent(kZGVideoCommunicationLoginEventLoginFailed);
            }

            DestroyUIData(pdata);
        };

        GlobalPostMsgDataToUI(main_hwnd_, pdata);
    }
    else 
    {
        PrintLogToView("初始化失败，错误码为：%d", nError);
        UpdateLoginEvent(kZGVideoCommunicationLoginEventLoginFailed);
    }
}

void VideoCommunicationDemo::OnLoginRoom(int errorCode, const char *pszRoomID, const  COMMON::ZegoStreamInfo *pStreamInfo, unsigned int streamCount)
{
    if (errorCode == 0)
    {
        start_retry_login_room_timestamp_ = -1;

        PrintLogToView("登录成功");
        UpdateLoginEvent(kZGVideoCommunicationLoginEventLoginSuccess);
        // 登录房间成功，切线程然后在开始推流
        PostUIData * pdata = CreateUIData();
        pdata->cb_in_ui_fun = [pdata, this]()->void
        {
            string stream_id = ZGHelper::Instance()->GetRandomUUID().c_str();
            string live_title = "";
            string publish_param = "";
            AV::ZegoPublishFlag publish_flag = AV::ZEGO_JOIN_PUBLISH;// 连麦方式

            PrintLogToView("推流 id = %s", stream_id.c_str());

            PrintLogToView("开始推流");

            ZGConfigHelperInstance()->SetVideoBitrate(kBitrate);

            PrintLogToView("设置码率为:%d bps", kBitrate);

            LIVEROOM::StartPublishing(live_title.c_str(), stream_id.c_str(), publish_flag, publish_param.c_str());

            DestroyUIData(pdata);
        };

        GlobalPostMsgDataToUI(main_hwnd_, pdata);

        for (int i = 0; i < streamCount; ++i)
        {
            PrintLogToView("index:%d, szStreamId=%s, szUserId = %s", i, pStreamInfo[i].szStreamId, pStreamInfo[i].szUserId);

            ProcessStreamAdded(pStreamInfo[i].szStreamId, pStreamInfo[i].szUserId, pStreamInfo[i].szUserName, pszRoomID);
        }

    }
    else {
        PrintLogToView("登录房间失败，错误码为：%d", errorCode);
        UpdateLoginEvent(kZGVideoCommunicationLoginEventLoginFailed);
        if (auto_retry_login_room_)
        {
            std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms_));
                RetryLoginRoom();
            }).detach();
        }
    }
}

void VideoCommunicationDemo::OnLogoutRoom(int errorCode, const char *pszRoomID)
{
    PrintLogToView("已退出登录");
}

void VideoCommunicationDemo::OnKickOut(int reason, const char *pszRoomID)
{
    PrintLogToView("被踢掉线，reason = %d", reason);
    UpdateLoginEvent(kZGVideoCommunicationLoginEventKickOut);
}

void VideoCommunicationDemo::OnDisconnect(int errorCode, const char *pszRoomID)
{
    PrintLogToView("已掉线");
    UpdateLoginEvent(kZGVideoCommunicationLoginEventDisconnect);
}

void VideoCommunicationDemo::OnReconnect(int errorCode, const char *pszRoomID)
{
    PrintLogToView("已重连上服务器");
    UpdateLoginEvent(kZGVideoCommunicationLoginEventReconnect);
}

void VideoCommunicationDemo::OnTempBroken(int errorCode, const char *pszRoomID)
{
    PrintLogToView("OnTempBroken，临时掉线，SDK内部正在进行重连，错误码 = %d", errorCode);
    UpdateLoginEvent(kZGVideoCommunicationLoginEventTempBroke);
}

void VideoCommunicationDemo::OnStreamUpdated(ZegoStreamUpdateType type, COMMON::ZegoStreamInfo *pStreamInfo, unsigned int streamCount, const char *pszRoomID)
{
    PrintLogToView("OnStreamUpdated, %s, streamCount = %d, pszRoomID = %s", type == COMMON::StreamAdded ? "流新增" : "流删除", streamCount, pszRoomID);

    for (int i = 0; i < streamCount; ++i)
    {
        PrintLogToView("index:%d , szStreamId = %s, ", i, pStreamInfo[i].szStreamId, pStreamInfo[i].szUserId);
        // 流新增加
        if (type == COMMON::StreamAdded)
        {
            ProcessStreamAdded(pStreamInfo[i].szStreamId, pStreamInfo[i].szUserId, pStreamInfo[i].szUserName, pszRoomID);
        }
        // 流删除的通知
        else if (type == COMMON::StreamDeleted) {

            ProcessStreamDeleted(pStreamInfo[i].szStreamId, pStreamInfo[i].szUserId, pStreamInfo[i].szUserName, pszRoomID);
        }
    }
}

void VideoCommunicationDemo::OnStreamExtraInfoUpdated(COMMON::ZegoStreamInfo *pStreamInfo, unsigned int streamCount, const char *pszRoomID)
{
    
}

void VideoCommunicationDemo::OnCustomCommand(int errorCode, int requestSeq, const char *pszRoomID)
{
    
}

void VideoCommunicationDemo::OnRecvCustomCommand(const char *pszUserId, const char *pszUserName, const char *pszContent, const char *pszRoomID)
{
    
}

void VideoCommunicationDemo::OnPublishStateUpdate(int stateCode, const char* pszStreamID, const ZegoPublishingStreamInfo& oStreamInfo)
{
    if (stateCode == 0)
    {
        PrintLogToView("推流成功");
    }
    else {
        PrintLogToView("推流状态码更新，推流失败，状态码 = %d", stateCode);
    }
}

void VideoCommunicationDemo::OnPublishQualityUpdate(const char* pszStreamID, ZegoPublishQuality publishQuality)
{
    
}

void VideoCommunicationDemo::OnCaptureVideoSizeChanged(int nWidth, int nHeight)
{
    PrintLogToView("采集视频帧大小变化，width = %d, height = %d", nWidth, nHeight);
}

void VideoCommunicationDemo::OnCaptureVideoFirstFrame()
{
    PrintLogToView("采集视频首帧到达");
}


void VideoCommunicationDemo::UpdateLoginEvent(ZGVideoCommunicationLoginEvent event)
{
    ZGVideoCommunicationStatus status = login_status_;

    switch (status) {
    case kZGVideoCommunicationLoginStatusLogout: {
        switch (event) {
        case kZGVideoCommunicationLoginEventLogin:
            status = kZGVideoCommunicationLoginStatusStartLogin;
            break;
        case kZGVideoCommunicationLoginEventLogout:
            break;
        default:
            PrintLogToView("ZegoRoomManager error event with current status");
            break;
        }
        break;
    }
    case kZGVideoCommunicationLoginStatusStartLogin: {
        switch (event) {
        case kZGVideoCommunicationLoginEventLoginSuccess:
            status = kZGVideoCommunicationLoginStatusLogin;
            break;
        case kZGVideoCommunicationLoginEventLogout:
        case kZGVideoCommunicationLoginEventLoginFailed:
            status = kZGVideoCommunicationLoginStatusLogout;
            break;
        default:
            PrintLogToView("ZegoRoomManager error event with current status");
            break;
        }
        break;
    }
    case kZGVideoCommunicationLoginStatusLogin: {
        switch (event) {
        case kZGVideoCommunicationLoginEventTempBroke:
            status = kZGVideoCommunicationLoginStatusTempBroken;
            break;
        case kZGVideoCommunicationLoginEventLogout:
        case kZGVideoCommunicationLoginEventKickOut:
        case kZGVideoCommunicationLoginEventDisconnect:
            status = kZGVideoCommunicationLoginStatusLogout;
            break;
        default:
            PrintLogToView("ZegoRoomManager error event with current status");
            break;
        }
        break;
    }
    case kZGVideoCommunicationLoginStatusTempBroken: {
        switch (event) {
        case kZGVideoCommunicationLoginEventTempBroke:
            break;
        case kZGVideoCommunicationLoginEventReconnect:
            status = kZGVideoCommunicationLoginStatusLogin;
            break;
        case kZGVideoCommunicationLoginEventLogout:
        case kZGVideoCommunicationLoginEventKickOut:
        case kZGVideoCommunicationLoginEventDisconnect:
            status = kZGVideoCommunicationLoginStatusLogout;
            break;
        default:
            PrintLogToView("ZegoRoomManager error event with current status");
            break;
        }
        break;
    }
    }

    login_status_ = status;

    if (video_communication_cb_ != nullptr)
    {
        video_communication_cb_->OnLoginEvent(event, login_status_);
    }
}

bool VideoCommunicationDemo::RetryLoginRoom()
{
    if (start_retry_login_room_timestamp_ == -1)
    {
        start_retry_login_room_timestamp_ = ZGHelperInstance()->GetCurTimeStampMs();
    }
    else if (retry_login_room_timeout_ms_ != 0)
    {
        if (ZGHelperInstance()->GetCurTimeStampMs() - start_retry_login_room_timestamp_ > retry_login_room_timeout_ms_)
        {
            PrintLogToView("重连超时,不再重连");
            StopRetryRoomReconnect(kZGVideoCommunicationReconnectStopReasonTimeout);
            return false;
        }
    }
    PrintLogToView("业务层重连中...");

    PrintLogToView("room id = %s", room_id_.c_str());
    UpdateLoginEvent(kZGVideoCommunicationLoginEventLogin);
    BOOL ret = LIVEROOM::LoginRoom(room_id_.c_str(), COMMON::Audience);
    if (!ret)
    {
        UpdateLoginEvent(kZGVideoCommunicationLoginEventLoginFailed);
    }
    return ret;
}

void VideoCommunicationDemo::StopRetryRoomReconnect(ZGVideoCommunicationReconnectStopReason reason)
{
    if (auto_retry_login_room_)
    {
//         if (on_stop_room_reconnect_ != nullptr)
//         {
//             on_stop_room_reconnect_(reason);
//         }
    }
    auto_retry_login_room_ = false;
}

void VideoCommunicationDemo::ProcessStreamAdded(string stream_id, string user_id, string user_name, string room_id)
{
    PlayConfig config;
    ZegoUser for_user;

    for_user.user_id = user_id;
    for_user.user_name = user_name;

    if (GetPlayConfigByUser(for_user, config))
    {
        config.stream_id = stream_id;

        UpdatePlayConfigByUser(for_user, config);

        // 已经为该用户配置视图时直接拉该用户的流
        if (config.view_hwnd != nullptr)
        {
            // 切线程然后在开始拉流
            PostUIData * pdata = CreateUIData();
            pdata->cb_in_ui_fun = [pdata, config, this]()->void
            {
                LIVEROOM::StartPlayingStream(config.stream_id.c_str(), config.view_hwnd);
                LIVEROOM::SetViewMode(config.view_mode, config.stream_id.c_str());

                DestroyUIData(pdata);
            };

            GlobalPostMsgDataToUI(main_hwnd_, pdata);

        }
    }
    else {
        PlayConfig play_config;
        ZegoUser user;

        user.user_id = user_id;
        user.user_name = user_name;
        play_config.stream_id = stream_id;

        std::lock_guard<std::mutex> lock(play_config_map_mutex_);
        play_map_info_.insert(std::make_pair(user, play_config));
    }

    if (video_communication_cb_ != nullptr)
    {
        video_communication_cb_->OnUserJoin(for_user);
    }
}

void VideoCommunicationDemo::ProcessStreamDeleted(string stream_id, string user_id, string user_name, string room_id)
{
    RemovePlayConfigByStreamId(stream_id);

    ZegoUser for_user;
    for_user.user_id = user_id;
    for_user.user_name = user_name;

    if (video_communication_cb_ != nullptr)
    {
        video_communication_cb_->OnUserLeave(for_user);
    }
}

void VideoCommunicationDemo::UpdatePlayConfigByUser(ZegoUser user, const PlayConfig& config)
{
    std::lock_guard<std::mutex> lock(play_config_map_mutex_);

    std::map<ZegoUser, PlayConfig>::iterator find_iter = std::find_if(play_map_info_.begin(), play_map_info_.end(), [&](const std::pair<ZegoUser, PlayConfig> &item) {
        if (item.first.user_id == user.user_id && item.first.user_name == user.user_name)
        {
            return true;
        }
        else {
            return false;
        }
    });
    if (find_iter != play_map_info_.end())
    {
        find_iter->second = config;
    }
}

void VideoCommunicationDemo::RemovePlayConfigByStreamId(string stream_id)
{
    std::lock_guard<std::mutex> lock(play_config_map_mutex_);

    std::map<ZegoUser, PlayConfig>::iterator find_iter = std::find_if(play_map_info_.begin(), play_map_info_.end(), [&](const std::pair<ZegoUser, PlayConfig> &item) {
        if (item.second.stream_id == stream_id)
        {
            return true;
        }
        else {
            return false;
        }
    });
    if (find_iter != play_map_info_.end())
    {
        PlayConfig need_stop_play_config;
        need_stop_play_config = find_iter->second;

        // 切线程然后在停止拉流
        PostUIData * pdata = CreateUIData();
        pdata->cb_in_ui_fun = [pdata, need_stop_play_config, this]()->void
        {
            PrintLogToView("停止拉流, stream id = %s", need_stop_play_config.stream_id.c_str());
            
            LIVEROOM::StopPlayingStream(need_stop_play_config.stream_id.c_str());

            DestroyUIData(pdata);
        };

        GlobalPostMsgDataToUI(main_hwnd_, pdata);

        play_map_info_.erase(find_iter);
    }
}


bool VideoCommunicationDemo::GetPlayConfigByUser(ZegoUser user, PlayConfig& config)
{
    std::lock_guard<std::mutex> lock(play_config_map_mutex_);

    std::map<ZegoUser, PlayConfig>::iterator find_iter = std::find_if(play_map_info_.begin(), play_map_info_.end(), [&](const std::pair<ZegoUser, PlayConfig> &item) {
        if (item.first.user_id == user.user_id && item.first.user_name == user.user_name)
        {
            return true;
        }
        else {
            return false;
        }
    });
    if (find_iter != play_map_info_.end())
    {
        config = find_iter->second;
        return true;
    }
    else {
        return false;
    }
}