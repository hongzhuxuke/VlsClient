#ifndef _VHLL_SERVICE_INCLUDE_H__
#define _VHLL_SERVICE_INCLUDE_H__
class SocketServer;
extern SocketServer* gLocalServer;
extern int    gLocalServerPort;
extern bool StartVhallClient(const char *uname, 
   const char *address, 
   const char *token,
   const char *streamName, 
   bool isHideLogo,

   const char *userId,
   const char *role,
   const char *webninartype,
   const char *accesstoken,
   const char *scheduler);
extern bool StopVhallClient();

std::string srs_string_replace(std::string str, std::string old_str, std::string new_str);
void ReplaceString(std::wstring& str, const std::wstring& src, const std::wstring& des);
#endif