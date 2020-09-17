#define EXE_VER "2.5.0.41"
#define VER_NAME "no_logo"
#define fps  25

[setup]
;发布者
AppPublisher=vhall.com
AppPublisherURL=http://www.vhall.com/
AppSupportURL=http://www.vhall.com/
AppUpdatesURL=http://www.vhall.com/
SetupIconFile=sdgh.ico
AppName=  {cm:vname}
DefaultDirName={userdocs}\VhallLive2.0
AppVerName=VhallLive {#EXE_VER}
DefaultGroupName={cm:MyGroupName}
OutputDir=.\setup
ArchitecturesInstallIn64BitMode = x64
PrivilegesRequired=admin
ArchitecturesAllowed = x86 x64
OutputBaseFilename=VhallLiveSetup_{#EXE_VER}_{#VER_NAME}
Compression=lzma/ultra
SolidCompression=yes
VersionInfoVersion={#EXE_VER}
ShowLanguageDialog=no
UsePreviousLanguage=no

DisableReadyPage=yes
DisableProgramGroupPage=yes
//DisableTaskPage=yes
DirExistsWarning=no
DisableDirPage=yes

;[Messages]
;SetupAppTitle= AppName 安装向导
;SetupWindowTitle= AppName 安装向导
;en.BeveledLabel=Life is not dress rehearsal, living every day!
;chs.BeveledLabel=人生没有彩排，每天都是现场直播！

[Files]

Source: compiler:IsTask.dll; Flags: dontcopy noencryption

Source: tmpHideLogo\*; DestDir: {tmp}; Flags: dontcopy solidbreak ; Attribs: hidden system
Source: ".\tmpHideLogo\botva2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\tmpHideLogo\InnoCallback.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\ISTask.dll"; DestDir: "{app}"; Flags: ignoreversion

;Source: "..\Release\dxsetup\dxwebsetup.exe"; DestDir: "{app}\dxsetup"; AfterInstall: checkDX10(ExpandConstant('{app}\dxsetup\dxwebsetup.exe')); Flags: ignoreversion
Source: "..\Release\dxsetup\dxwebsetup.exe"; DestDir: "{app}\dxsetup"; Flags: ignoreversion; BeforeInstall:UnInstallWinService('VhallService.exe')
Source: "..\Release\dxsetup\WinPcap_4_1_3.exe"; DestDir: "{app}\dxsetup"; Flags: ignoreversion;
;Source: "..\Release\config.ini"; DestDir: "{app}";BeforeInstall:killprocess('VhallLive.exe');  Flags: onlyifdoesntexist uninsneveruninstall
Source: "..\Release\config.ini"; DestDir: "{userdocs}\VhallHelper";  Flags: onlyifdoesntexist uninsneveruninstall
;BeforeInstall:killprocess('VhallLive.exe');
;Source: "..\Release\audiodevice.ini"; DestDir: "{userdocs}\VhallHelper";BeforeInstall:killprocess('VhallLive.exe');  Flags: onlyifdoesntexist uninsneveruninstall
;Source: "..\Release\audio.ini"; DestDir: "{userdocs}\VhallHelper";BeforeInstall:killprocess('VhallLive.exe');  Flags: onlyifdoesntexist uninsneveruninstall
Source: "..\Release\vhalllogo.png"; DestDir: "{app}";BeforeInstall:makeDir();  Flags: ignoreversion
;Source: "..\Release\config.ini"; DestDir: "{app}";BeforeInstall:killvbp();  Flags: ignoreversion
Source: "..\Release\VhallLiveBG.jpg"; DestDir: "{app}";BeforeInstall:killvbp();  Flags: ignoreversion
//Source: "..\Release\vhalllogo.png"; DestDir: "{app}";  Flags: ignoreversion

Source: "..\Release\screen.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\disconnected.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\SecurityTool.exe"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\cameraLoading.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\cameraLoadingFailed.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\boleyou.ttf"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\msc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\yuv.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\tls.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\no_logo_image\login_logo.png"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\no_logo_image\splash.png"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\no_logo_image\versionBack.png"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\no_logo_image\VhallLiveBG.jpg"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\no_logo_image\vhalllogo.png"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\no_logo_image\vhallTitleLogo.png"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\no_logo_image\vhprocessbarback.png"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\no_logo_image\vhall_tool_config.ini";  DestDir: "{userdocs}\VhallHelper"; Flags: ignoreversion



;Source: "..\Release\CustomStream.txt"; DestDir: "{app}";  Flags: ignoreversion

Source: "..\Release\AudioEngine.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\DecklinkDevice.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\libvlc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libvlccore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libx264-157.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\MediaCore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\MediaReader.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\OBS.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\OBSApi.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\VhallService.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\VhallLive.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\MainUI.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\CommonToolKit.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\OBSControl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\dshowcapture.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\vhall_webrtc_sdk.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\VhallWebRtcSDK.dll"; DestDir: "{app}"; Flags: ignoreversion
 
Source: "..\Release\GraphicsPlugins\GraphicsD3D10.dll"; DestDir: "{app}\GraphicsPlugins"; Flags: ignoreversion
Source: "..\Release\GraphicsPlugins\GraphicsD3D11.dll"; DestDir: "{app}\GraphicsPlugins"; Flags: ignoreversion

Source: "..\Release\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-runtime-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\msvcp120.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\msvcr120.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\msvcp100.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\msvcr100.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\msvcp120d.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\msvcr120d.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\msvcp100d.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\msvcr100d.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\D3Dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\d3dx10_43.dll"; DestDir: "{app}"; Flags: ignoreversion

;QT release
;Source: "..\Release\icudt53.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\icuin53.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\icuuc53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libEGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libGLESV2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Positioning.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5PrintSupport.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Qml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Quick.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Sql.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5WebChannel.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5WebEngineWidgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5WebEngineCore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\QtWebEngineProcess.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5QuickWidgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\pepflashplayer.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5SerialPort.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Multimedia.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5MultimediaWidgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5OpenGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5WinExtras.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5WebEngine.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\VhallNetWork.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\VHWebSocket.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\nghttp2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libssl-1_1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libcrypto-1_1.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\CustomStreamURL.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\icuin53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\icuuc53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\icudt53.dll"; DestDir: "{app}"; Flags: ignoreversion


Source: "..\Release\shaders\*.*"; DestDir: "{app}\shaders"; Flags: ignoreversion
Source: "..\Release\resources\*.*"; DestDir: "{app}\resources"; Flags: ignoreversion
Source: "..\Release\translations\*.*"; DestDir: "{app}\translations"; Flags: ignoreversion
Source: "..\Release\Qtplugins\bearer\*.*"; DestDir: "{app}\bearer"; Flags: ignoreversion
Source: "..\Release\Qtplugins\imageformats\*.*"; DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "..\Release\Qtplugins\platforms\*.*"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "..\Release\Qtplugins\sqldrivers\*.*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion
Source: "..\Release\Qtplugins\iconengines\*.*"; DestDir: "{app}\iconengines"; Flags: ignoreversion
Source: "..\Release\Qtplugins\printsupport\*.*"; DestDir: "{app}\printsupport"; Flags: ignoreversion
Source: "..\Release\Qtplugins\position\*.*"; DestDir: "{app}\position"; Flags: ignoreversion
Source: "..\Release\translations\qtwebengine_locales\*.*"; DestDir: "{app}\translations\qtwebengine_locales"; Flags: ignoreversion

Source: "..\Release\locale\*.*"; DestDir: "{app}\locale"; Flags: ignoreversion
Source: "..\Release\plugins\*.*"; DestDir: "{app}\plugins\"; Flags: ignoreversion recursesubdirs createallsubdirs;


Source: "..\Release\lua\*.*"; DestDir: "{app}\lua\"; Flags: ignoreversion recursesubdirs createallsubdirs;
Source: "..\Release\axvlc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\npvlc.dll"; DestDir: "{app}"; Flags: ignoreversion


Source: "..\Release\DShowPlugin.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\VhallRightExtraWidget.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\VHMonitorCapture.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\websockets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\InteractionSDK.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion



Source: "../Release/VhallAudioEngine.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "../Release/VhallDecklinkDevice.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "../Release/Vhalldshowcapture.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "../Release/VhallMediaReader.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "../Release/VhallOBS.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "../Release/VhallOBSApi.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "../Release/VhallVHMonitorCapture.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "../Release/VhallDShowPlugin.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "../Release/vhall_ns_process.dll"; DestDir: "{app}"; Flags: ignoreversion


Source: "..\Release\avcodec-58.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avdevice-58.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avfilter-7.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avformat-58.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avutil-56.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\postproc-55.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\swresample-3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\swscale-5.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\swscale-5.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libpng16.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\zlib.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\VhallLive\project\sdgh.ico"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\ExternalCapture.bmp"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\InterActivitySDK.dll"; DestDir: "{app}"; Flags: ignoreversion

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Source: "..\Release\AdvVideoDev.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\avcodec-54bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\avformat-54bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\avutil-51bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\bugreport.exe"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\dbghelp.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\iLiveSDK.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\IntelDec.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\IntelEnc.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\IntelUtil.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\libtim.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\MediaEngine.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\MediaFileEx.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\qavsdk.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\QQAudioHook.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\QQAudioHookService.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\SkinBeauty.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\swresample-0bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\swscale-2bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\TcVpxDec.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\TcVpxEnc.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\tinyxml.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\TRAE.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\UDT.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\vld_x86.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\VP8.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\xPlatform.dll"; DestDir: "{app}"; Flags: ignoreversion
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;Source: "..\Release\InterActivitySDKNew.dll"; DestDir: "{app}"; Flags: ignoreversion

;Source: "..\Release\AdvVideoDev.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\avcodec-54bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\avformat-54bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\avutil-51bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\drDll.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\ilivefiltersdk.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\iLiveSDK.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\IntelDec.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\IntelEnc.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\IntelUtil.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\libtim.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\MediaEngine.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\MediaFileEx.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\qavsdk.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\QQAudioHook.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\QQAudioHookService.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\SkinBeauty.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\swresample-0bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\swscale-2bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\TcVpxDec.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\TcVpxEnc.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\tinyxml.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\TRAE.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\UDT.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\VP8.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\xPlatform.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\yuv.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\cmd.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\PluginWebServer.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\api-ms-win-crt-convert-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-environment-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-filesystem-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-heap-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-locale-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-math-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-multibyte-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-runtime-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-stdio-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-string-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-time-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-crt-utility-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\ucrtbase.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\api-ms-win-crt-runtime-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion


Source: "..\Release\api-ms-win-core-console-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-datetime-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-debug-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-errorhandling-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-file-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-file-l1-2-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-file-l2-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-handle-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-heap-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-interlocked-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-libraryloader-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-localization-l1-2-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-memory-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-namedpipe-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-processenvironment-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-processthreads-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-processthreads-l1-1-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-profile-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-rtlsupport-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-string-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-synch-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-synch-l1-2-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-sysinfo-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-timezone-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\api-ms-win-core-util-l1-1-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\VHPaasSDK.dll"; DestDir: "{app}"; Flags: ignoreversion

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[Icons]
;Name: "{group}\Vhall {cm:voffline}"; Filename: "{app}\Vhall_{cm:voffline}.exe";IconFilename:"{app}\EventMaker.ico";  Parameters: "/offline"
;Name: "{group}\{cm:vname}"; Filename: "{app}\vbp.exe"; Parameters: "-m console"
;Name: "{group}\uninstall {cm:vname}"; Filename: "{app}\vbp.exe";Parameters: "-m service -u uninstall"

Name: "{group}\{cm:showName}"; Filename: "{app}\VhallLive.exe"; IconFilename:"{app}\sdgh.ico"
Name: "{group}\{cm:UninstallProgram,cm:showName}"; Filename: "{uninstallexe}"
;Name: "{commondesktop}\微吼直播"; Filename: "{app}\VhallLive.exe"; IconFilename:"{app}\sdgh.ico"; Tasks: DesktopIcon
Name: "{commondesktop}\{cm:showName}"; Filename: "{app}\VhallLive.exe"; IconFilename:"{app}\sdgh.ico"; Tasks: DesktopIcon

[Registry]
;delete vpb bootstart
Root: HKLM; Subkey: "Software\\Microsoft\\Windows\\CurrentVersion\\Run"; ValueType: string; ValueName: "vbp"; ValueData:  "";Flags:uninsdeletevalue
Root: HKLM; Subkey: "Software\\Microsoft\\Windows\\CurrentVersion\\Run"; ValueType: string; ValueName: "VhallService"; ValueData:  "{app}\VhallService.exe -i";Flags:uninsdeletevalue
Root: HKLM; Subkey: "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"; ValueType: string; ValueName: "UninstallString"; ValueData:  "{app}\VhallLive";Flags:uninsdeletevalue
Root: HKCR; Subkey: "VHALLLive"; ValueType: string;ValueName: "URL Protocol"; ValueData:  "{app}\VhallLive";Flags:uninsdeletevalue
Root: HKCR; Subkey: "VHALLLive\shell\open\command";  ValueType: string; ValueData:  "{app}\VhallLive %1";Flags:uninsdeletevalue

[CustomMessages]
chs.vname=直播助手
en.regist=Register
en.registerInfo=if using all function please register Vhall accounts.
en.exitVhall=Please Exit Vhall then press Install  Button.
chs.regist=免费注册
chs.registerInfo=.

en.MyGroupName=Vhall Live Streaming
chs.MyGroupName=直播助手

en.vname=VhallLive
chs.exitVhall=请先退出VhallLive再进行安装.

en.wrongOS=Please use the operating system which is Windows 7 or higher version! The setup program will exit!
chs.wrongOS=请使用WIN7及以上的操作系统！无法运行程序，本安装程序即将退出！
en.detectNoDX=Your machine don't have the necessary DirectX components! Will you install it right now?
chs.detectNoDX=系统检测到您没有安装必需的DirectX组件，是否立刻安装？
en.DXWarning=DirectX is not installed. Vhall Live Streaming may not be able to run normally!
chs.DXWarning=没有安装DirectX组件，直播助手可能无法正常使用！

en.Installing=Installation process
chs.Installing=正在安装，已完成

en.shoutcuts=Generating shortcuts
chs.shoutcuts=生成快捷方式

en.showName=VHALL LIVE
chs.showName=直播助手

[InstallDelete]
Type: files; Name: "{app}\vbp.log";BeforeInstall:killvbp()
;Type: files; Name: "{app}\VhallService.log";BeforeInstall:UnInstallWinService('VhallService.exe')
[UninstallRun]
//Filename: "taskkill";Parameters: "/IM  VhallLive.exe "; Flags: waituntilterminated
Filename: "{app}\VhallService.exe ";Parameters: "-d"; Flags: waituntilterminated runhidden

[Languages]
Name: "chs"; MessagesFile: "compiler:Languages\ChineseSimp.isl"
Name: "en"; MessagesFile: "compiler:Languages\English.isl"
[Run]
Filename: "{app}\dxsetup\dxwebsetup.exe"; WorkingDir: "{app}\dxsetup"; Parameters: ""; Check: detectDirectX(ExpandConstant('{app}\dxsetup\dxwebsetup.exe')); Flags: waituntilterminated;
Filename: "{app}\dxsetup\WinPcap_4_1_3.exe"; WorkingDir: "{app}\dxsetup"; Parameters: ""; Check: detectWinCap(ExpandConstant('{app}\dxsetup\WinPcap_4_1_3.exe')); Flags: waituntilterminated;
Filename: "{app}\VhallService.exe"; Parameters: "-i"; Description: "{cm:vname}"; Flags: nowait runhidden
;Filename: "{app}\VhallLive.exe"; Parameters: "-r restart"; Description: "{cm:vname}"; Flags: nowait runhidden

[Tasks]
Name: "DesktopIcon"; Description: "TaskDescription"

[code]
#include  'dll.iss'

var
  label2:TLabel;      //label1,labShortCut
  pathEditbkg,BGimg:longint;

  MinBtn,CancelBtn,btn_Browser,btn_setup,btn5,btnImSetup,btnBack,btnImBack:HWND;     //licenseBtn,

  c_btn,bShortCut:boolean;    //xypd,
  pathEdit:tedit;
  progressbgImgbk,progressbgImg,PBOldProc:longint;  //licenseImg,
  Timer1: TTimer;
  saveMyDirRadio,saveCustomDirRadio,ShortcutCheck:HWND;       //licenseCheck
  //licenseRich:TRichEditViewer;
  iCurPage:integer;
  imInstalling,imshoutcuts:Longint;     //imaRead,
  b1,b2,DHIMG1:Longint;
  js1,js2:integer;

procedure makeLogDir( param: String);
var
command:String;
ResultCode: Integer;
begin
  if(DirExists(param) = false) then
    begin
      command:= '/c mkdir ' +  param;
      Exec(ExpandConstant('{cmd}'), command,'', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;
end;

//procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
//begin
//    if CurUninstallStep = usDone then
//    begin
       //删除 {app} 文件夹及其中所有文件
//    DelTree(ExpandConstant('{app}'), True, True, True);
//    end;
//end;

procedure makeDir();
var
rootPath:String;
path:String;
taskkillParam:String;
ResultCode: Integer;
begin

  rootPath:= ExpandConstant(('{userdocs}'));

  path:= rootPath + '\VhallHelper\vhlog';
  makeLogDir(path);

  path:= rootPath + '\VhallHelper\crashDumps';
  makeLogDir(path);

  path:= rootPath + '\VhallHelper\logs';
  makeLogDir(path);

  path:= rootPath + '\VhallHelper\pluginData';
  makeLogDir(path);

  path:= rootPath + '\VhallHelper\profiles';
  makeLogDir(path);

  path:= rootPath + '\VhallHelper\sceneCollection';
  makeLogDir(path);

  path:= rootPath + '\VhallHelper\services';
  makeLogDir(path);

  path:= rootPath + '\VhallHelper\updates';
  makeLogDir(path);

  path:= rootPath + '\VhallHelper\dshowlog';
  makeLogDir(path);

  ResultCode:= 0;
end;

Function StartMenu(): Boolean;
begin
  //MsgBox('StartMenu()',mbConfirmation, MB_OK);
  Result :=BtngetChecked(saveCustomDirRadio)
end;

Function desktop(): Boolean;
begin
  //MsgBox('desktop()',mbConfirmation, MB_OK);
  Result :=BtngetChecked(saveMyDirRadio)
end;

procedure CancelBtnOnClick(hBtn:HWND); //取消按钮点击事件
begin
 if iCurPage = wpFinished then //安装完成
 begin
    WizardForm.NextButton.Click;
 end;

 WizardForm.CancelButton.Click;
end;

procedure MinBtnOnClick(hBtn:HWND);  //最小化按钮点击事件
begin
SendMessage(WizardForm.Handle,WM_SYSCOMMAND,61472,0);
end;


procedure btn5click(hBtn:HWND); //自定义安装 按钮点击事件
begin
  if c_btn then   //路径选择
  begin
  pathEdit.show;

  //BtnSetVisibility(licenseBtn,false)
  //BtnSetVisibility(licenseCheck,false)
  BtnSetVisibility(ShortcutCheck,true)
  
  //labShortCut.Show;
  BtnSetVisibility(btn_Browser,true)
  BtnSetVisibility(btnImSetup,true)
  BtnSetVisibility(btnImBack,true)
  BtnSetVisibility(btn5,false)
  BtnSetVisibility(btn_setup,false)
  //ImgSetVisibility(imaRead,false)
  ImgSetVisibility(imInstalling,false)
  
  //WizardForm.Height:=360
  BGimg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\bigbg.png'),0,0,560,360,true,true);
  imshoutcuts:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\shoutcuts.png'),40,311,74,16,true,true);
  ImgSetVisibility(imshoutcuts,true)
  
  c_btn := false
  end else   //快速安装
  begin

  pathEdit.Hide;
  BtnSetVisibility(btn_Browser,false)
  //WizardForm.Height:=360
  BGimg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\bg_license.png'),0,0,560,360,true,true);
  c_btn := true

  //BtnSetVisibility(licenseBtn,true)
  //BtnSetVisibility(licenseCheck,true)
  BtnSetVisibility(ShortcutCheck,false)
  //labShortCut.Hide;
  BtnSetVisibility(btnImSetup,false)
  BtnSetVisibility(btnImBack,false)
  BtnSetVisibility(btn5,true)
  BtnSetVisibility(btn_setup,true)
  BtnSetVisibility(imInstalling,true)
  BtnSetVisibility(imshoutcuts,false)
  //ImgRelease(imaRead)
  //imaRead:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\imaRead.png'),40,328,64,16,true,true);
  //ImgSetVisibility(imaRead,true)


  end;
  ImgApplyChanges(WizardForm.Handle)
end;

procedure btn_Browserclick(hBtn:HWND); // 浏览 按钮点击 事件
begin
//MsgBox('procedure btn_Browserclick()',mbConfirmation, MB_OK);
  ImgSetVisibility(imInstalling,false)
  ImgSetVisibility(imshoutcuts,true)
  WizardForm.DirBrowseButton.Click;
  pathEdit.text := WizardForm.DirEdit.text;
end;

procedure pathEditChange(Sender: TObject); //安装路径改变事件
begin
//MsgBox('procedure pathEditChange()',mbConfirmation, MB_OK);
  WizardForm.DirEdit.text:=pathEdit.Text ;
end;

procedure check_licenseclick(hBtn:HWND);      // license 复选框 点击事件
begin
    //if BtnGetChecked(licenseCheck)=true then

   // begin
       BtnSetEnabled(btn_setup,true)
       BtnSetEnabled(btnImSetup,true)
    //end
    //else
    //begin

      // BtnSetEnabled(btn_setup,false)
      // BtnSetEnabled(btnImSetup,false)
    //end
end;

procedure check_ShortcutCheck(hBtn:HWND);      // 生成快捷方式 点击事件
begin
  bShortCut:=BtnGetChecked(ShortcutCheck)
//  MsgBox('procedure check_ShortcutCheck():'+inttostr(bShortCut),mbConfirmation, MB_OK);
end;

procedure killprocess(processName: String);
var
unins:String;
taskkillParam:String;
ResultCode: Integer;
begin
 //MsgBox('RunTask 判断', mbInformation, MB_OK);
  if RunTask(processName, false) then
      begin
        WizardForm.NextButton.Click;
        //MsgBox('processName', mbInformation, MB_OK);
        unins:= '/c taskkill /f /t /im ' + processName;
        Exec(ExpandConstant('{cmd}'), unins,'', SW_HIDE, ewWaitUntilTerminated, ResultCode);
        ResultCode:= 0;
      end
  else
  
  //MsgBox('RunTask else', mbInformation, MB_OK);
  ResultCode:= 0;
  
end;


procedure btn_setupclick(hBtn:HWND);//立即安装 按钮点击事件
begin
  if iCurPage=wpWelcome then //欢迎页面/安装过程
  begin
  //MsgBox('procedure btn_setupclick()',mbConfirmation, MB_OK);
  BtnSetVisibility(btn_setup,false);
  pathEdit.Hide;
  BtnSetVisibility(btn_Browser,false)
  c_btn := true
  BtnSetVisibility(btnImBack,false);
  BtnSetVisibility(btnImSetup,false);
  //BtnSetVisibility(licenseBtn,false);
  //BtnSetVisibility(licenseCheck,false);
  BtnSetVisibility(ShortcutCheck,false);
  //labShortCut.Hide;
  ImgSetVisibility(imshoutcuts,false)
  BtnSetVisibility(btn5,false)


  BGimg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\loading_pic.png'),0,0,560,360,true,true);

  progressbgImgbk:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\loadingbk.png'),20,317,520,5,True,True);
  progressbgImg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\loading.png'),0,317,0,0,True,True);


  b1:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\loading_pic1.png'),0,0,560,250,true,true);    //加载图片必备的
  b2:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\loading_pic2.png'),0,0,560,250,true,true);

  ImgSetVisibility(b1,false)     //出现动画时 第一张显示的图片   下面几张禁用的
  ImgSetVisibility(b2,false)


  BtnSetVisibility(btn_setup,false);
  Timer1.Interval:=50
  //label1:= TLabel.Create(WizardForm);
  //with label1 do
  //begin
    //Parent := WizardForm;
    //Caption := '正在安装，已完成';
    //Caption := cm:Installing
    //Caption := ExpandConstant('{cm:Installing}')
    //Font.Name:='宋体'
    //Font.Size:=10
    //Font.Color:=$00BBBBBB
    //Transparent := TRUE;
    //SetBounds(20 ,288,130,17)
   //end;
    label2:= TLabel.Create(WizardForm);
    with label2 do
      begin
        Parent := WizardForm;
        Caption := '0%';
        Font.Name:='宋体'
        Font.Size:=12
        Transparent := TRUE;
        Font.Color:=$005956fc
        SetBounds(124 ,286,400,17)
      end;

   imInstalling:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\Installing.png'),20 ,288,98,16,true,true);
   killprocess('VhallLive.exe');
  end;

  if iCurPage=wpFinished  then //安装完成页面
  begin
    WizardForm.Release;
  end;
  WizardForm.NextButton.Click;
end;

function PBProc(h:hWnd;Msg,wParam,lParam:Longint):Longint; //
var
  pr,i1,i2 : Extended;
  w : integer;
begin
  Result:=CallWindowProc(PBOldProc,h,Msg,wParam,lParam);
  if (Msg=$402) and (WizardForm.ProgressGauge.Position>WizardForm.ProgressGauge.Min) then
  begin
    i1:=WizardForm.ProgressGauge.Position-WizardForm.ProgressGauge.Min;
    i2:=WizardForm.ProgressGauge.Max-WizardForm.ProgressGauge.Min;
    pr:=i1*100/i2;
    label2.Caption:= inttostr((WizardForm.ProgressGauge.Position-WizardForm.ProgressGauge.Min)*100/(WizardForm.ProgressGauge.Max-WizardForm.ProgressGauge.Min))+'%';

    w:=Round(520*pr/100);
    ImgSetPosition(progressbgImg,20,317,w,5);
    //ImgSetVisiblePart(progressbgImg,0,0,w,15);
    //ImgApplyChanges(WizardForm.Handle);
  end;
end;



procedure WizardMouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  ReleaseCapture
  SendMessage(WizardForm.Handle, $0112, $F012, 0)
end;

procedure Timer1Timer(Sender: TObject);   //核心代码 时钟控制动画 需要inno5 增强版      代码.mad8834671
begin
  js2:=js2+10; //控制图片透明度
  if  (js2>200) then      //控制图片透明度 计数归零
  begin
    js2:=0
    js1:=js1+1;
  end;

 if js2<200 then
 begin
    //控制显示图片
  if js1=1 then
  begin
    ImgSetVisibility(b1,false)
    ImgSetVisibility(b2,true)
  end;
  if js1=2 then
  begin
    ImgSetVisibility(b1,true)
    ImgSetVisibility(b2,false)
  end;
  if js1>2 then
  begin
    ImgSetVisibility(b1,false)
    ImgSetVisibility(b2,true)
    js1:=1
  end;
end;


ImgApplyChanges(WizardForm.Handle)
end;

procedure locklicenseclick(hBtn:HWND);
var
  ErrorCode: Integer;
begin
  ShellExec('open', 'http://e.vhall.com/home/vhallapi/serviceterms', '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode);
end;

procedure InitializeWizard();   //使用这个事件函数启动时改变向导或向导页。你不能在它触发之后使用 InitializeSetup 事件函数，向导窗体不退出。
begin
//MsgBox('procedure InitializeWizard()',mbConfirmation, MB_OK);
  WizardForm.OuterNotebook.hide;
  WizardForm.Bevel.Hide;
  WizardForm.BorderStyle:=bsnone;
  WizardForm.Position:=poDesktopCenter;
  WizardForm.Width:=560;
  WizardForm.Height:=360;
  WizardForm.Color:=clWhite ;
  //xypd:=true;

  WizardForm.NextButton.Hide;
  WizardForm.BackButton.Hide;
  WizardForm.CancelButton.Hide;
  
  WizardForm.OnMouseDown:=@WizardMouseDown
  c_btn := true;

  ExtractTemporaryFile('bg_license.png');
  //ExtractTemporaryFile('license.rtf');
  ExtractTemporaryFile('btn_n.png');
  ExtractTemporaryFile('btn_complete.png');
  ExtractTemporaryFile('btn_setup.png');
  ExtractTemporaryFile('btnImSetup.png');

  ExtractTemporaryFile('bigbg.png');
  ExtractTemporaryFile('btn_Browser.png');
  ExtractTemporaryFile('edit_Browser.png');

  ExtractTemporaryFile('loadingbk.png');
  ExtractTemporaryFile('loading.png');

  //ExtractTemporaryFile('license.png');

  //ExtractTemporaryFile('imaRead.png');
  ExtractTemporaryFile('shoutcuts.png');
  ExtractTemporaryFile('Installing.png');
  ExtractTemporaryFile('loading_pic1.png');
  ExtractTemporaryFile('loading_pic2.png');
  //ExtractTemporaryFile('loading_pic3.png');
  //ExtractTemporaryFile('loading_pic4.png');

  ExtractTemporaryFile('checkbox.png');
  //ExtractTemporaryFile('checkboxdeep.png');

  ExtractTemporaryFile('loading_pic.png');
  ExtractTemporaryFile('finish_bg.png');
  ExtractTemporaryFile('btn_close.png');
  ExtractTemporaryFile('btn_min.png');
  ExtractTemporaryFile('back.png');

  CancelBtn:=BtnCreate(WizardForm.Handle,530,10,20,15,ExpandConstant('{tmp}\btn_close.png'),1,False)
  BtnSetEvent(CancelBtn,BtnClickEventID,WrapBtnCallback(@CancelBtnOnClick,1));

  MinBtn:=BtnCreate(WizardForm.Handle,505,10,20,15,ExpandConstant('{tmp}\btn_min.png'),1,False)
  BtnSetEvent(MinBtn,BtnClickEventID,WrapBtnCallback(@MinBtnOnClick,1));

  btn_setup:=BtnCreate(WizardForm.Handle,203,238,154,40,ExpandConstant('{tmp}\btn_setup.png'),1,False)
  BtnSetEvent(btn_setup,BtnClickEventID,WrapBtnCallback(@btn_setupclick,1));

  
  
  btnImSetup:=BtnCreate(WizardForm.Handle,460,310,80,32,ExpandConstant('{tmp}\btnImSetup.png'),1,False)
  BtnSetEvent(btnImSetup,BtnClickEventID,WrapBtnCallback(@btn_setupclick,1));
  BtnSetVisibility(btnImSetup,false)

  btnBack:=BtnCreate(WizardForm.Handle,240,310,80,32,ExpandConstant('{tmp}\back.png'),1,False)
  BtnSetEvent(btnBack,BtnClickEventID,WrapBtnCallback(@locklicenseclick,1));
  BtnSetVisibility(btnBack,false)

  btnImBack:=BtnCreate(WizardForm.Handle,370,310,80,32,ExpandConstant('{tmp}\back.png'),1,False)
  BtnSetEvent(btnImBack,BtnClickEventID,WrapBtnCallback(@btn5click,1));
  BtnSetVisibility(btnImBack,false)



  ShortcutCheck:=BtnCreate(WizardForm.Handle,20,312,16,14,ExpandConstant('{tmp}\checkbox.png'),1,true)
  BtnSetEvent(ShortcutCheck,BtnClickEventID,WrapBtnCallback(@check_ShortcutCheck,1));
  BtnSetChecked(ShortcutCheck,true)

  //labShortCut:= TLabel.Create(WizardForm);
 // with labShortCut do
  //begin
    //Parent := WizardForm;
    //Caption := '生成快捷方式';
    //Caption := ExpandConstant('{cm:shoutcuts}')
    //Font.Name:='宋体'
    //Font.Size:=10
   //Font.Color:=$00BBBBBB
    //Transparent := TRUE;
    //SetBounds(40 ,311,130,17)
  //end;

  BtnSetVisibility(ShortcutCheck,false)
  //labShortCut.Hide;

  pathEdit:= TEdit.Create(WizardForm);
  with pathEdit do
  begin
    Parent := WizardForm;
    text :=WizardForm.DirEdit.text;
    Font.Name:='宋体'
    Font.Size:=10
    BorderStyle:=bsNone;
    SetBounds(25,270,425,22)
    OnChange:=@pathEditChange;
    //Color := $00D0e2ff;    cl3dlight
    //Color := $00FFE2D0   //颜色值赋值
    TabStop :=false;
  end;
  pathEdit.Hide;

  btn_Browser:=BtnCreate(WizardForm.Handle,460,262,82,32,ExpandConstant('{tmp}\btn_Browser.png'),1,False)
  BtnSetEvent(btn_Browser,BtnClickEventID,WrapBtnCallback(@btn_Browserclick,1));
  BtnSetVisibility(btn_Browser,false)

  btn5:=BtnCreate(WizardForm.Handle,469,328,72,17,ExpandConstant('{tmp}\btn_n.png'),1,False)
  BtnSetEvent(btn5,BtnClickEventID,WrapBtnCallback(@btn5click,1));

  PBOldProc:=SetWindowLong(WizardForm.ProgressGauge.Handle,-4,PBCallBack(@PBProc,4));

  Timer1 := TTimer.Create(WizardForm);
  with Timer1 do
  begin
    OnTimer := @Timer1Timer;
  end;
   js1:=1
   js2:=0
   
  //licenseRich:= TRichEditViewer.Create(WizardForm);
  //with licenseRich do
  //begin
  //Parent := WizardForm;
  //ReadOnly:= true;
  //SCROLLBARS:= ssVertical;
  //font.Name:='宋体'
  //font.Size:=20
  //Color:=clWhite;
  //BorderStyle:= bsNone;
  //SetBounds(42,36,476,187)
  //Lines.LoadFromFile(ExpandConstant('{tmp}\license.rtf'));
  //TabStop:=false;
  //Height := 0;
  //end;

  ImgApplyChanges(WizardForm.Handle)
end;



procedure DeinitializeSetup();
begin
  gdipShutdown;
end;

procedure CurPageChanged(CurPageID: Integer);//在新向导页 (用 CurPageID 指定) 显示后调用
var
  Index: Integer;
  ResultStr: String;
  ResultCode: Integer;
begin
  //MsgBox( intToStr(CurPageID), mbInformation, MB_OK);
  //WizardForm.NextButton.Visible:=false;
  WizardForm.CancelButton.Height:=0;
  WizardForm.BackButton.Height:=0;
  WizardForm.NextButton.Height:=0;
  //WizardForm.NextButton.Hide;
  //WizardForm.BackButton.Hide;
  //WizardForm.CancelButton.Hide;


  
  if CurPageID=wpWelcome then //欢迎页面
  begin
  bShortCut:=true;
  iCurPage:=wpWelcome;
  BGimg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\bg_license.png'),0,0,560,360,true,true);
  pathEditbkg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\edit_Browser.png'),60,385,500,15,FALSE,true);

  //imaRead:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\imaRead.png'),40,328,64,16,true,true);
  imshoutcuts:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\shoutcuts.png'),40,311,74,16,true,true);
  
  //licenseBtn:=BtnCreate(WizardForm.Handle,100,327,136,17,ExpandConstant('{tmp}\license.png'),4,false) //协议条款按钮
  //BtnSetEvent(licenseBtn,BtnClickEventID,WrapBtnCallback(@locklicenseclick,1));
  //licenseCheck:=BtnCreate(WizardForm.Handle,20,330,16,14,ExpandConstant('{tmp}\checkbox.png'),1,true)
  //BtnSetEvent(licenseCheck,BtnClickEventID,WrapBtnCallback(@check_licenseclick,1));
  //BtnSetChecked(licenseCheck,true)

  //ImgSetVisibility(imaRead,true)
  ImgSetVisibility(imshoutcuts,false)
  WizardForm.Show;
  end;
  if CurPageID = wpSelectTasks then //附加任务
  begin
    iCurPage:=wpSelectTasks;
    
    //if RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\微吼直播助手_is1', 'UninstallString', ResultStr) then
    if RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\'+ExpandConstant('{cm:vname}')+'_is1', 'UninstallString', ResultStr) then
    begin
    ResultStr := RemoveQuotes(ResultStr);
    Exec(ResultStr, '/verysilent /suppressmsgboxes', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;
    
    Index := WizardForm.TasksList.Items.IndexOf('TaskDescription');
    if Index <> -1 then
    begin
      if bShortCut=true then
      begin
        WizardForm.TasksList.Checked[Index] := True;
      end
      else
      begin
        WizardForm.TasksList.Checked[Index] := False;
      end
    end;

    WizardForm.NextButton.Click;
  end;
  //if CurPageID = wpPreparing then //正在准备安装
  //begin
      //iCurPage:=wpPreparing;
      //WizardForm.NextButton.Click;
  //end;
  if CurPageID = wpInstalling then //正在安装
  begin

    iCurPage:=wpInstalling
  end;
  if CurPageID = wpFinished then //安装完成
  begin
   iCurPage:=wpFinished;
   Timer1.Interval:=0
   ImgSetVisibility(progressbgImgbk,false)
   ImgSetVisibility(progressbgImg,false)
   btn_setup:=BtnCreate(WizardForm.Handle,203,232,154,40,ExpandConstant('{tmp}\btn_complete.png'),1,False)
   
   BtnSetEvent(btn_setup,BtnClickEventID,WrapBtnCallback(@btn_setupclick,1));
   BGimg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\finish_bg.png'),0,0,560,360,true,true);
   BtnSetEnabled(CancelBtn,true)
   BtnSetVisibility(CancelBtn,true)
   BtnSetVisibility(MinBtn,false)

  ImgSetVisibility(imshoutcuts,false)
  ImgSetVisibility(imInstalling,false)

   //label1.Hide;
   label2.Hide;

  end;

  ImgApplyChanges(WizardForm.Handle)
end;

function ShouldSkipPage(PageID: Integer): Boolean;  //如果返回 True，将跳过该页；如果你返回 False，该页被显示。
begin
    if (PageID=wpSelectComponents)  then    //跳过组件安装界面
    begin
      result := true;
    end
    if(wpSelectTasks=PageId)  then     //选择附加任务
    begin
       result := true;
    end;
    if(wpPreparing=PageId) then//正在准备安装
    begin
      result := true;
    end
    if(wpInfoAfter=PageID) then    //信息
    begin
      result := true;
    end
    else
      result := false;
end;

procedure InstallCapture();
var
  axFile:String;
  tmpFile:String;
  tmp:String;
  ResultCode: Integer;
begin
    axFile:= ExpandConstant(('{app}\VhallDesktop.ax'));
    tmpFile := axFile;
    if( FileExists(tmpFile)) then
    begin
      tmp := ' -s \"' + axFile +'\"';
      Exec('regsvr32',tmp,'',SW_HIDE, ewWaitUntilTerminated,ResultCode);
    end
    else
    begin
      tmpFile := axFile + '.1';
      if( FileExists(tmpFile))  then
      begin
        tmp := ' -s \"' + axFile + '.1\"';
        Exec('regsvr32',tmp,'',SW_HIDE, ewWaitUntilTerminated,ResultCode);
      end
    end

end;

procedure killvbp();
var
unins:String;
ResultCode: Integer;
begin
  unins:= ExpandConstant(('{app}\vbp.exe'));
  if( FileExists(unins)) then
    begin
      Exec('taskkill', '/IM  vbp.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;
end;

procedure UnInstallCapture();
var
  srcFile:String;
  axFile:String;
  tmpFile:String;
  tmp:String;
  ResultCode: Integer;
begin
   axFile:= ExpandConstant(('{app}\VhallDesktop.ax'));
    tmpFile := axFile;
    if( FileExists(tmpFile)) then
    begin
      tmp := ' -u -s \"' + axFile +'\"';
      Exec('regsvr32',tmp,'',SW_HIDE, ewWaitUntilTerminated,ResultCode);
      DeleteFile(tmpFile);
    end;

    tmpFile := axFile + '.1';
    if( FileExists(tmpFile))  then
    begin
      tmp := ' -u  -s \"' + axFile + '.1\"';
      Exec('regsvr32',tmp,'',SW_HIDE, ewWaitUntilTerminated,ResultCode);
      DeleteFile(tmpFile);
    end;

     tmpFile :=  ExpandConstant(('{sys}\VhallDesktop.ax'));;
    if( FileExists(tmpFile))  then
    begin
      tmp := ' -u -s \"'+tmpFile +'\"';
      Exec('regsvr32',tmp,'',SW_HIDE, ewWaitUntilTerminated,ResultCode)
      DeleteFile(tmpFile);
    end;
end;


procedure vbpInstall( param: String);
var
unins:String;
ResultCode: Integer;
begin
  unins:= ExpandConstant(('{app}\vbp.exe'));
  if( FileExists(unins)) then
    begin
      Exec(unins, param, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;
end;

procedure UnInstallWinService(serviceFileName: String);
var
uninservice:String;
ResultCode: Integer;
begin
  uninservice:= ExpandConstant(('{app}\')) + serviceFileName;
  if( FileExists(uninservice)) then
    begin
      Exec(uninservice, ' -d', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;
end;

function InitializeSetup: Boolean;
var Isbl: boolean;
var Isstr: string;
var MyProgChecked: Boolean;
var Version:TWindowsVersion;
begin
  GetWindowsVersionEx(Version);
  if (Version.Major < 6) or ((Version.Major = 6) and (Version.Minor = 0)) then
    begin
      MsgBox(ExpandConstant('{cm:wrongOS}'),mbInformation,MB_OK);
      Result := false;
      Exit;
    end
  else
    begin
      Result := true;
    end;
    result := true;
end;

procedure checkDX10(FileName: String);
var DXresult:Longint;
ResultCode: Integer;
ErrorCode: Integer;
begin
  if (LoadDLL(ExpandConstant('{sys}\D3DX10_43.DLL'), ErrorCode) = 0) or (LoadDLL(ExpandConstant('{sys}\D3D10_1.DLL'), ErrorCode) = 0)
                 or (LoadDLL(ExpandConstant('{sys}\DXGI.DLL'), ErrorCode) = 0) or (LoadDLL(ExpandConstant('{sys}\D3DCompiler_43.dll'), ErrorCode) = 0) then
    begin
      //if MsgBox('系统检测到您没有安装必需的DirectX组件，是否立刻安装？', mbConfirmation, MB_YESNO) = idYes then
      if MsgBox(ExpandConstant('{cm:detectNoDX}'), mbConfirmation, MB_YESNO) = idYes then
        begin
          ShellExec('', FileName, '', '' , SW_SHOW, ewWaitUntilTerminated, ErrorCode);  //ewNoWait
        end
      else
        begin
          //MsgBox('没有安装DirectX组件，本程序可能无法正常使用！',mbInformation,MB_OK);
          MsgBox(ExpandConstant('{cm:DXWarning}'),mbInformation,MB_OK);
          ResultCode := 0;
        end
    end;
end;

function detectDirectX(FileName: String): Boolean;
var DXresult:Longint;
ResultCode: Integer;
ErrorCode: Integer;
begin
  if (LoadDLL(ExpandConstant('{sys}\D3DX10_43.DLL'), ErrorCode) = 0) or (LoadDLL(ExpandConstant('{sys}\D3D10_1.DLL'), ErrorCode) = 0)
                 or (LoadDLL(ExpandConstant('{sys}\DXGI.DLL'), ErrorCode) = 0) or (LoadDLL(ExpandConstant('{sys}\D3DCompiler_43.dll'), ErrorCode) = 0) then
    begin
      if MsgBox(ExpandConstant('{cm:detectNoDX}'), mbConfirmation, MB_YESNO) = idYes then
        begin
          //ShellExec('', FileName, '', '' , SW_SHOW, ewWaitUntilTerminated, ErrorCode);  //ewNoWait
          Result := true;
        end
      else
        begin
          MsgBox(ExpandConstant('{cm:DXWarning}'),mbInformation,MB_OK);
          Result := false;
        end
    end;
end;


function detectWinCap(FileName: String): Boolean;
var DXresult:Longint;
var WpcapResultCode:Boolean;
var PacketResultCode:Boolean;
var wpcapPath :String;
var packetPath :String;
var ResultCode:Boolean;
begin
   ResultCode := false;
   wpcapPath := ExpandConstant('{sys}\wpcap.dll');
   packetPath := ExpandConstant('{sys}\Packet.dll');
   WpcapResultCode := FileExists(wpcapPath);
   PacketResultCode := FileExists(packetPath);

  if (WpcapResultCode = false) or (PacketResultCode = false) then
    begin
      if MsgBox(ExpandConstant('{cm:detectNoWinpcap}'), mbConfirmation, MB_YESNO) = idYes then
        begin
          Result := true;
        end
      else
        begin
          MsgBox(ExpandConstant('{cm:WinpcapWarning}'),mbInformation,MB_OK);
          Result := false;
        end
    end;
end;

{ RedesignWizardFormBegin } // 不要删除这一行代码。
// 不要修改这一段代码，它是自动生成的。
var
  OldEvent_NextButtonClick: TNotifyEvent;

procedure _NextButtonClick(Sender: TObject); forward;

procedure RedesignWizardForm;
begin
  with WizardForm.NextButton do
  begin
    OldEvent_NextButtonClick := OnClick;
    OnClick := @_NextButtonClick;
  end;

  with WizardForm.TasksList do
  begin
    Top := ScaleY(26);
    Height := ScaleY(185);
  end;

  with WizardForm.PreparingYesRadio do
  begin
    Color := clBackground;
    ParentColor := False;
  end;

  with WizardForm.PreparingMemo do
  begin
    Top := ScaleY(121);
  end;

{ ReservationBegin }
  // 这一部分是提供给你的，你可以在这里输入一些补充代码。
{ ReservationEnd }
end;
// 不要修改这一段代码，它是自动生成的。
{ RedesignWizardFormEnd } // 不要删除这一行代码。

procedure _NextButtonClick(Sender: TObject);
begin
  OldEvent_NextButtonClick(Sender);
end;

[ISFormDesigner]
WizardForm=FF0A005457495A415244464F524D0030108502000054504630F10B5457697A617264466F726D0A57697A617264466F726D0C436C69656E74486569676874034C010B436C69656E74576964746803F101134F6E436F6E73747261696E6564526573697A6507105F4E657874427574746F6E436C69636B0D4578706C6963697457696474680301020E4578706C696369744865696768740372010D506978656C73506572496E636802600A54657874486569676874020C00F10A544E6577427574746F6E0A4E657874427574746F6E074F6E436C69636B07105F4E657874427574746F6E436C69636B0000F10C544E65774E6F7465626F6F6B0D4F757465724E6F7465626F6F6B00F110544E65774E6F7465626F6F6B5061676509496E6E65725061676500F10C544E65774E6F7465626F6F6B0D496E6E65724E6F7465626F6F6B00F110544E65774E6F7465626F6F6B506167651453656C656374436F6D706F6E656E74735061676500F10C544E6577436F6D626F426F780A5479706573436F6D626F064865696768740214000000F110544E65774E6F7465626F6F6B506167650F53656C6563745461736B735061676500F110544E6577436865636B4C697374426F78095461736B734C69737403546F70021A0648656967687403B9000B4578706C69636974546F70021A0E4578706C6963697448656967687403B900000000F110544E65774E6F7465626F6F6B506167650D507265706172696E675061676500F10F544E6577526164696F427574746F6E11507265706172696E67596573526164696F05436F6C6F72070C636C4261636B67726F756E640B506172656E74436F6C6F72080000F108544E65774D656D6F0D507265706172696E674D656D6F03546F7002790B4578706C69636974546F70027900000000000000