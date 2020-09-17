set path=%path%;"C:\Program Files (x86)\Windows Kits\8.1\bin\x86"
wosigncodecmd  sign /dig sha256 /tp 60569cb296a77794109b52308fa8b5058160345c /p vhall20200701  /hide /c /tr http://timestamp.digicert.com /file ./setup/VhallLiveSetup_3.0.2.1_normal.exe
pause


