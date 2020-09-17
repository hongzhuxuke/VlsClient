#define EXE_VER "2.2.2.2"
#define fps  25

[Setup]
AppPublisher=vhall.com
AppPublisherURL=http://www.vhall.com/
AppSupportURL=http://www.vhall.com/
AppUpdatesURL=http://www.vhall.com/
SetupIconFile=vhall.ico
AppName=  {cm:vname}
DefaultDirName={userdocs}\VhallLive2.0
AppVerName=VhallLive {#EXE_VER}
DefaultGroupName={cm:MyGroupName}
OutputDir=.\setup
ArchitecturesInstallIn64BitMode = x64
PrivilegesRequired=none
ArchitecturesAllowed = x86 x64
OutputBaseFilename=VhallLiveSetup_{#EXE_VER}
Compression=lzma/ultra
SolidCompression=yes
VersionInfoVersion={#EXE_VER}
;ShowLanguageDialog=no
UsePreviousLanguage=no

[Code]
//安装前判断是否有进程正在运行，istask.dll文件放到innosetup安装目录
function RunTask(FileName: string; bFullpath: Boolean): Boolean;
external 'RunTask@files:ISTask.dll stdcall delayload';
function KillTask(ExeFileName: string): Integer;
external 'KillTask@files:ISTask.dll stdcall delayload';

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
    end

    tmpFile := axFile + '.1';
    if( FileExists(tmpFile))  then
    begin
      tmp := ' -u  -s \"' + axFile + '.1\"';
      Exec('regsvr32',tmp,'',SW_HIDE, ewWaitUntilTerminated,ResultCode);
      DeleteFile(tmpFile);
    end

     tmpFile :=  ExpandConstant(('{sys}\VhallDesktop.ax'));;
    if( FileExists(tmpFile))  then
    begin
      tmp := ' -u -s \"'+tmpFile +'\"';
      Exec('regsvr32',tmp,'',SW_HIDE, ewWaitUntilTerminated,ResultCode)
      DeleteFile(tmpFile);
    end
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

procedure killprocess(processName: String);
var
unins:String;
taskkillParam:String;
ResultCode: Integer;
begin
  if RunTask(processName, false) then
      begin
        unins:= '/c taskkill /f /t /im ' + processName;
        Exec(ExpandConstant('{cmd}'), unins,'', SW_HIDE, ewWaitUntilTerminated, ResultCode);
        ResultCode:= 0;
      end
  else
  ResultCode:= 0;
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
ResultStr: String;
ResultCode: Integer;
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
    end
    
  if RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\微吼直播助手_is1', 'UninstallString', ResultStr) then
    begin
    ResultStr := RemoveQuotes(ResultStr);
    Exec(ResultStr, '/silent', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
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
      if MsgBox('系统检测到您没有安装必需的DirectX组件，是否立刻安装？', mbConfirmation, MB_YESNO) = idYes then
        begin
          ShellExec('', FileName, '', '' , SW_SHOW, ewWaitUntilTerminated, ErrorCode);  //ewNoWait
        end
      else
        begin
          MsgBox('没有安装DirectX组件，本程序可能无法正常使用！',mbInformation,MB_OK);
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

[Languages]
Name: "chs"; MessagesFile: "compiler:Languages\ChineseSimp.isl"
Name: "en"; MessagesFile: "compiler:Languages\English.isl"

[CustomMessages]
chs.vname=微吼直播助手
en.regist=Register
en.registerInfo=if using all function please register Vhall accounts.
en.exitVhall=Please Exit Vhall then press Install  Button.
chs.regist=免费注册
chs.registerInfo=.

en.MyGroupName=Vhall Live Streaming
chs.MyGroupName=微吼直播助手
                                 
en.vname=VhallLive
chs.exitVhall=请先退出VhallLive再进行安装.

en.wrongOS=Please use the operating system which is Windows 7 or higher version! The setup program will exit!
chs.wrongOS=请使用WIN7及以上的操作系统！无法运行程序，本安装程序即将退出！
en.detectNoDX=Your machine don't have the necessary DirectX components! Will you install it right now?
chs.detectNoDX=系统检测到您没有安装必需的DirectX组件，是否立刻安装？
en.DXWarning=DirectX is not installed. Vhall Live Streaming may not be able to run normally!
chs.DXWarning=没有安装DirectX组件，微吼直播助手可能无法正常使用！

[Messages]
en.BeveledLabel=Life is not dress rehearsal, living every day!
chs.BeveledLabel=人生没有彩排，每天都是现场直播！

[Tasks]
;Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";
;Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}";
[Components]
;Name: "readme"; Description: "Readme File"; Types: full

[Files]
Source: compiler:IsTask.dll; Flags: dontcopy noencryption
Source: tmp\*; DestDir: {tmp}; Flags: dontcopy solidbreak ; Attribs: hidden system
Source: ".\tmp\botva2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\tmp\InnoCallback.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\ISTask.dll"; DestDir: "{app}"; Flags: ignoreversion

;Source: "..\Release\dxsetup\dxwebsetup.exe"; DestDir: "{app}\dxsetup"; AfterInstall: checkDX10(ExpandConstant('{app}\dxsetup\dxwebsetup.exe')); Flags: ignoreversion
Source: "..\Release\dxsetup\dxwebsetup.exe"; DestDir: "{app}\dxsetup"; Flags: ignoreversion; BeforeInstall:UnInstallWinService('VhallService.exe')
;Source: "..\Release\config.ini"; DestDir: "{app}";BeforeInstall:killprocess('VhallLive.exe');  Flags: onlyifdoesntexist uninsneveruninstall
Source: "..\Release\config.ini"; DestDir: "{userdocs}\VhallHelper";  Flags: onlyifdoesntexist uninsneveruninstall
;BeforeInstall:killprocess('VhallLive.exe');
;Source: "..\Release\audiodevice.ini"; DestDir: "{userdocs}\VhallHelper";BeforeInstall:killprocess('VhallLive.exe');  Flags: onlyifdoesntexist uninsneveruninstall
;Source: "..\Release\audio.ini"; DestDir: "{userdocs}\VhallHelper";BeforeInstall:killprocess('VhallLive.exe');  Flags: onlyifdoesntexist uninsneveruninstall
Source: "..\Release\vhalllogo.png"; DestDir: "{app}";BeforeInstall:makeDir();  Flags: ignoreversion
;Source: "..\Release\config.ini"; DestDir: "{app}";BeforeInstall:killvbp();  Flags: ignoreversion
Source: "..\Release\VhallLiveBG.jpg"; DestDir: "{app}";BeforeInstall:killvbp();  Flags: ignoreversion
//Source: "..\Release\vhalllogo.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\vhall_tool_config.ini";  DestDir: "{userdocs}\VhallHelper"; Flags: ignoreversion
Source: "..\Release\screen.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\splash.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\VhallLiveBGNoVhall.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\vhalllogoNoVhall.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\disconnected.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\SecurityTool.exe"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\cameraLoading.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\cameraLoadingFailed.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Release\boleyou.ttf"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\msc.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\CustomStream.txt"; DestDir: "{app}";  Flags: ignoreversion

Source: "..\Release\AudioEngine.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\DecklinkDevice.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\libvlc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libvlccore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libx264-148.dll"; DestDir: "{app}"; Flags: ignoreversion
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

Source: "..\Release\GraphicsPlugins\GraphicsD3D10.dll"; DestDir: "{app}\GraphicsPlugins"; Flags: ignoreversion
Source: "..\Release\GraphicsPlugins\GraphicsD3D11.dll"; DestDir: "{app}\GraphicsPlugins"; Flags: ignoreversion

Source: "..\Release\msvcp120.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\msvcr120.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\msvcp100.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\msvcr100.dll"; DestDir: "{app}"; Flags: ignoreversion

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
Source: "..\Release\Qt5Multimedia.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5MultimediaWidgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5OpenGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Positioning.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5PrintSupport.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Qml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Quick.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5Sql.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5WebChannel.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5WebEngineWidgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5WebEngineCore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5WebEngine.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\Qt5WinExtras.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\QtWebEngineProcess.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\pepflashplayer.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\CustomStreamURL.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\icuin53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\icuuc53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\icudt53.dll"; DestDir: "{app}"; Flags: ignoreversion


Source: "..\Release\shaders\*.*"; DestDir: "{app}\shaders"; Flags: ignoreversion
Source: "..\Release\resources\*.*"; DestDir: "{app}\resources"; Flags: ignoreversion
Source: "..\Release\translations\*.*"; DestDir: "{app}\translations"; Flags: ignoreversion
Source: "..\Release\Qtplugins\bearer\*.*"; DestDir: "{app}\Qtplugins\bearer"; Flags: ignoreversion
Source: "..\Release\Qtplugins\imageformats\*.*"; DestDir: "{app}\Qtplugins\imageformats"; Flags: ignoreversion
Source: "..\Release\Qtplugins\platforms\*.*"; DestDir: "{app}\Qtplugins\platforms"; Flags: ignoreversion
Source: "..\Release\Qtplugins\sqldrivers\*.*"; DestDir: "{app}\Qtplugins\sqldrivers"; Flags: ignoreversion
Source: "..\Release\Qtplugins\iconengines\*.*"; DestDir: "{app}\Qtplugins\iconengines"; Flags: ignoreversion
Source: "..\Release\Qtplugins\position\*.*"; DestDir: "{app}\Qtplugins\position"; Flags: ignoreversion
Source: "..\Release\translations\qtwebengine_locales\*.*"; DestDir: "{app}\translations\qtwebengine_locales"; Flags: ignoreversion

Source: "..\Release\locale\*.*"; DestDir: "{app}\locale"; Flags: ignoreversion
Source: "..\Release\plugins\*.*"; DestDir: "{app}\plugins\"; Flags: ignoreversion recursesubdirs createallsubdirs;
Source: "..\Release\lua\*.*"; DestDir: "{app}\lua\"; Flags: ignoreversion recursesubdirs createallsubdirs;
Source: "..\Release\axvlc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\npvlc.dll"; DestDir: "{app}"; Flags: ignoreversion


Source: "..\Release\DShowPlugin.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\VhallRightExtraWidget.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\VHMonitorCapture.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\websockets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\InteractionSDK.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion



Source: "..\Release\avcodec-57.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avdevice-57.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avfilter-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avformat-57.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avutil-55.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\postproc-54.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\swresample-2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\swscale-4.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\swscale-4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libpng16.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\zlib.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\VhallLive\project\vhall.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\ExternalCapture.bmp"; DestDir: "{app}"; Flags: ignoreversion
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

Source: "..\Release\InterActivitySDKNew.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\AdvVideoDev.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avcodec-54bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avformat-54bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\avutil-51bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\drDll.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\ilivefiltersdk.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\iLiveSDK.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\IntelDec.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\IntelEnc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\IntelUtil.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libtim.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\MediaEngine.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\MediaFileEx.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\qavsdk.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\QQAudioHook.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\QQAudioHookService.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\SkinBeauty.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\swresample-0bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\swscale-2bp1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\TcVpxDec.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\TcVpxEnc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\tinyxml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\TRAE.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\UDT.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\VP8.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\xPlatform.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\PluginWebServer.dll"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
;Name: "{group}\Vhall {cm:voffline}"; Filename: "{app}\Vhall_{cm:voffline}.exe";IconFilename:"{app}\EventMaker.ico";  Parameters: "/offline"
;Name: "{group}\{cm:vname}"; Filename: "{app}\vbp.exe"; Parameters: "-m console"
;Name: "{group}\uninstall {cm:vname}"; Filename: "{app}\vbp.exe";Parameters: "-m service -u uninstall"

Name: "{group}\微吼直播"; Filename: "{app}\VhallLive.exe"; IconFilename:"{app}\vhall.ico"
Name: "{group}\{cm:UninstallProgram,微吼直播}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\微吼直播"; Filename: "{app}\VhallLive.exe"; IconFilename:"{app}\vhall.ico"

[Registry]
;delete vpb bootstart
Root: HKLM; Subkey: "Software\\Microsoft\\Windows\\CurrentVersion\\Run"; ValueType: string; ValueName: "vbp"; ValueData:  "";Flags:uninsdeletevalue
Root: HKLM; Subkey: "Software\\Microsoft\\Windows\\CurrentVersion\\Run"; ValueType: string; ValueName: "VhallService"; ValueData:  "{app}\VhallService.exe -i";Flags:uninsdeletevalue
Root: HKLM; Subkey: "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"; ValueType: string; ValueName: "UninstallString"; ValueData:  "{app}\VhallLive";Flags:uninsdeletevalue
Root: HKCR; Subkey: "VHALLLive"; ValueType: string;ValueName: "URL Protocol"; ValueData:  "{app}\VhallLive";Flags:uninsdeletevalue 
Root: HKCR; Subkey: "VHALLLive\shell\open\command";  ValueType: string; ValueData:  "{app}\VhallLive %1";Flags:uninsdeletevalue 

[InstallDelete]
Type: files; Name: "{app}\vbp.log";BeforeInstall:killvbp()
;Type: files; Name: "{app}\VhallService.log";BeforeInstall:UnInstallWinService('VhallService.exe')
[UninstallRun]
;Filename: "taskkill";Parameters: "/IM  VhallLive.exe "; Flags: waituntilterminated
Filename: "{app}\VhallService.exe ";Parameters: "-d"; Flags: waituntilterminated runhidden


[Run]
Filename: "{app}\dxsetup\dxwebsetup.exe"; WorkingDir: "{app}\dxsetup"; Parameters: ""; Check: detectDirectX(ExpandConstant('{app}\dxsetup\dxwebsetup.exe')); Flags: waituntilterminated;
Filename: "{app}\VhallService.exe"; Parameters: "-i"; Description: "{cm:vname}"; Flags: nowait runhidden
;Filename: "{app}\VhallLive.exe"; Parameters: "-r restart"; Description: "{cm:vname}"; Flags: nowait runhidden

