set path=%path%;"C:\Program Files (x86)\Windows Kits\8.1\bin\x86"
<<<<<<< HEAD
signtool sign /f ./wosign/vhall20160522.pfx /p 15110073312Vhall /t http://timestamp.wosign.com/ ./setup/VhallLiveSetup_2.4.9.1_azure.exe
=======
wosigncodecmd  sign /dig sha256 /tp 0c9059939df8cfee072966ea94a49aeb2fa4d6d2 /p Vhall53520909  /hide /c /tr http://timestamp.digicert.com /file ./setup/VhallLiveSetup_2.4.9.9_azure.exe
>>>>>>> origin/develop


