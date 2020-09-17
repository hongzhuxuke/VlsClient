#define EXE_VER "2.0.0.19"
#define fps  25


[Setup]
AppPublisher=vhall.com
AppPublisherURL=http://www.vhall.com/
AppSupportURL=http://www.vhall.com/
AppUpdatesURL=http://www.vhall.com/

AppName=  {cm:vname}
DefaultDirName={pf}\VhallLive
AppVerName=VhallLive {#EXE_VER}
DefaultGroupName={cm:MyGroupName}
OutputDir=.\setup
ArchitecturesInstallIn64BitMode = x64
PrivilegesRequired=admin
ArchitecturesAllowed = x86 x64
OutputBaseFilename=VhallLiveSetup_{#EXE_VER}
Compression=lzma/ultra
SolidCompression=yes
VersionInfoVersion={#EXE_VER}
;ShowLanguageDialog=no
UsePreviousLanguage=no
[Code]
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
  unins:= ExpandConstant(('{app}\')) + processName;
  if( FileExists(unins)) then
    begin
      taskkillParam:=  '/IM ' +  processName;
      Exec('taskkill', taskkillParam, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
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
//Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";
//Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}";
[Components]
;Name: "readme"; Description: "Readme File"; Types: full

[Files]
//Source: "..\Debug\dxsetup\dxwebsetup.exe"; DestDir: "{app}\dxsetup"; AfterInstall: checkDX10(ExpandConstant('{app}\dxsetup\dxwebsetup.exe')); Flags: ignoreversion
Source: "..\Debug\dxsetup\dxwebsetup.exe"; DestDir: "{app}\dxsetup"; Flags: ignoreversion; BeforeInstall:UnInstallWinService('VhallService.exe')
//Source: "..\Debug\config.ini"; DestDir: "{app}";BeforeInstall:killprocess('VhallLive.exe');  Flags: onlyifdoesntexist uninsneveruninstall
Source: "..\Debug\config.ini"; DestDir: "{app}";BeforeInstall:killvbp();  Flags: ignoreversion
Source: "..\Debug\VhallLiveBG.jpg"; DestDir: "{app}";BeforeInstall:killvbp();  Flags: ignoreversion
Source: "..\Debug\vhalllogo.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Debug\screen.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Debug\disconnected.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Debug\SecurityTool.exe"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Debug\cameraLoading.png"; DestDir: "{app}";  Flags: ignoreversion
Source: "..\Debug\cameraLoadingFailed.png"; DestDir: "{app}";  Flags: ignoreversion

//Source: "..\Debug\CustomStream.txt"; DestDir: "{app}";  Flags: ignoreversion
                                                 
Source: "..\Debug\AudioEngine.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\DecklinkDevice.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Debug\libvlc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\libvlccore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\libx264-148.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\MediaCore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\MediaReader.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\OBS.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\OBSApi.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\VhallService.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\VhallLive.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\MainUI.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\CommonToolKit.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\OBSControl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\dshowcapture.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Debug\GraphicsPlugins\GraphicsD3D10.dll"; DestDir: "{app}\GraphicsPlugins"; Flags: ignoreversion
Source: "..\Debug\GraphicsPlugins\GraphicsD3D11.dll"; DestDir: "{app}\GraphicsPlugins"; Flags: ignoreversion

Source: "..\Debug\msvcp120d.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\msvcr120d.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Debug\msvcp120.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\msvcr120.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Debug\msvcp100.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\msvcr100.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Debug\D3DCompiler_43.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\d3dx10_43.dll"; DestDir: "{app}"; Flags: ignoreversion
                                       
;QT release
;Source: "..\Release\icudt53.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\icuin53.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "..\Release\icuuc53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5Cored.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5Guid.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5Networkd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5Widgetsd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5Multimediad.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5MultimediaWidgetsd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5OpenGLd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5Positioningd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5PrintSupportd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5Qmld.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5Quickd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5QuickWidgetsd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5Sensorsd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5Sqld.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5WebChanneld.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5WebKitd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\Qt5WebKitWidgetsd.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Debug\CustomStreamURL.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\icuin53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\icuuc53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\icudt53.dll"; DestDir: "{app}"; Flags: ignoreversion


Source: "..\Debug\shaders\*.*"; DestDir: "{app}\shaders"; Flags: ignoreversion
Source: "..\Debug\Qtplugins\bearer\*.*"; DestDir: "{app}\Qtplugins\bearer"; Flags: ignoreversion
Source: "..\Debug\Qtplugins\imageformats\*.*"; DestDir: "{app}\Qtplugins\imageformats"; Flags: ignoreversion
Source: "..\Debug\Qtplugins\platforms\*.*"; DestDir: "{app}\Qtplugins\platforms"; Flags: ignoreversion
Source: "..\Debug\locale\*.*"; DestDir: "{app}\locale"; Flags: ignoreversion
Source: "..\Debug\plugins\*.*"; DestDir: "{app}\plugins\"; Flags: ignoreversion recursesubdirs createallsubdirs;
Source: "..\Debug\lua\*.*"; DestDir: "{app}\lua\"; Flags: ignoreversion recursesubdirs createallsubdirs;
Source: "..\Debug\axvlc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\npvlc.dll"; DestDir: "{app}"; Flags: ignoreversion


Source: "..\Debug\DShowPlugin.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\VhallRightExtraWidget.dll"; DestDir: "{app}"; Flags: ignoreversionSource: "..\Debug\VHMonitorCapture.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Debug\websockets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\InteractionSDK.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion



Source: "..\Debug\avcodec-57.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\avdevice-57.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\avfilter-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\avformat-57.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\avutil-55.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\postproc-54.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\swresample-2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\swscale-4.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Debug\swscale-4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\libpng16d.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\zlibd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Debug\zlib.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\VhallLive\project\vhall.ico"; DestDir: "{app}"; Flags: ignoreversion
[Icons]//Name: "{group}\Vhall {cm:voffline}"; Filename: "{app}\Vhall_{cm:voffline}.exe";IconFilename:"{app}\EventMaker.ico";  Parameters: "/offline"//Name: "{group}\{cm:vname}"; Filename: "{app}\vbp.exe"; Parameters: "-m console"//Name: "{group}\uninstall {cm:vname}"; Filename: "{app}\vbp.exe";Parameters: "-m service -u uninstall"

Name: "{group}\微吼直播助手"; Filename: "{app}\VhallLive.exe"; IconFilename:"{app}\vhall.ico"
Name: "{group}\{cm:UninstallProgram,微吼直播助手}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\微吼直播助手"; Filename: "{app}\VhallLive.exe"; IconFilename:"{app}\vhall.ico"

[Registry]
//delete vpb bootstart
Root: HKLM; Subkey: "Software\\Microsoft\\Windows\\CurrentVersion\\Run"; ValueType: string; ValueName: "vbp"; ValueData:  "";Flags:uninsdeletevalue
Root: HKLM; Subkey: "Software\\Microsoft\\Windows\\CurrentVersion\\Run"; ValueType: string; ValueName: "VhallService"; ValueData:  "{app}\VhallService.exe -i";Flags:uninsdeletevalue
Root: HKLM; Subkey: "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"; ValueType: string; ValueName: "UninstallString"; ValueData:  "{app}\VhallLive";Flags:uninsdeletevalue
Root: HKCR; Subkey: "VHALLLive"; ValueType: string;ValueName: "URL Protocol"; ValueData:  "{app}\VhallLive";Flags:uninsdeletevalue 
Root: HKCR; Subkey: "VHALLLive\shell\open\command";  ValueType: string; ValueData:  "{app}\VhallLive %1";Flags:uninsdeletevalue 

[InstallDelete]
Type: files; Name: "{app}\vbp.log";BeforeInstall:killvbp()
//Type: files; Name: "{app}\VhallService.log";BeforeInstall:UnInstallWinService('VhallService.exe')
[UninstallRun]
//Filename: "taskkill";Parameters: "/IM  VhallLive.exe "; Flags: waituntilterminated
Filename: "{app}\VhallService.exe ";Parameters: "-d"; Flags: waituntilterminated runhidden


[Run]
Filename: "{app}\dxsetup\dxwebsetup.exe"; WorkingDir: "{app}\dxsetup"; Parameters: ""; Check: detectDirectX(ExpandConstant('{app}\dxsetup\dxwebsetup.exe')); Flags: waituntilterminated;
Filename: "{app}\VhallService.exe"; Parameters: "-i"; Description: "{cm:vname}"; Flags: nowait runhidden
Filename: "{app}\VhallLive.exe"; Parameters: "-r restart"; Description: "{cm:vname}"; Flags: nowait runhidden

