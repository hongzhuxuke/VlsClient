#include "rtmp_sys.h"
#include "log.h"
#include <windows.h>
#include <time.h>
#include <list>
#include <map>
#include <queue>
#include "publisher_multitcp_control.h"
#include "multitcpnetwork.h"
#include "multitcp.h"
#include "rtmp_inner.h"
//--------------------------[RTMP]-------------------------
int
MultiConnect0(RTMP *r, struct sockaddr * service)
{
	int ret = 1;
	IpAddr client, server;
	client.addr = (struct sockaddr*)(&r->m_bindIP.addr);
	client.addr_len = r->m_bindIP.addrLen;

	server.addr = service;
	server.addr_len = sizeof(struct sockaddr);


	RTMP_Log(RTMP_LOGINFO, "MultiTcpConnect");
	MultiTcpRTMPSocketBuf *mrsb = NULL;
	if (r->Link.timeout){
		mrsb = new MultiTcpRTMPSocketBuf(r->m_multi_conn_num, r->m_multi_conn_buf_size, r->Link.timeout * 1000);
	}
	else {
		mrsb = new MultiTcpRTMPSocketBuf(r->m_multi_conn_num, r->m_multi_conn_buf_size);
	}

	//mrsb->SetMaxSendBufSize(r->m_multi_conn_buf_size);
	//used for debug and log;
   if (r->Link.playpath.av_val!= NULL) {
      mrsb->mStreamID = r->Link.playpath.av_val;
   }

	if (!mrsb->Connect(&client, &server)){
		delete mrsb;
		return -1;
	}

	mrsb->CreateRecvThread();
	r->m_sb.SocketBufHandle = mrsb;
	return ret;
}

int MultiWriteN(RTMP *r, const char *buffer, int n)
{
	const char *ptr = buffer;


	while (n > 0)
	{
		int nBytes;

		if (r->m_bCustomSend && r->m_customSendFunc)
			nBytes = r->m_customSendFunc(&r->m_sb, ptr, n, r->m_customSendParam);
		else
			nBytes = RTMPSockBuf_Send(&r->m_sb, ptr, n);

		if (nBytes < 0)
		{
			int sockerr = GetSockError();
			RTMP_Log(RTMP_LOGERROR, "%s, RTMP send error %d (%d bytes)", __FUNCTION__,
				sockerr, n);

			if (sockerr == EINTR && !RTMP_ctrlC)
				continue;

			RTMP_Close(r);
			n = 1;
			break;
		}

		if (nBytes == 0)
			break;

		n -= nBytes;
		ptr += nBytes;
	}


	return n == 0;
}

int
MultiIsConnected(RTMP *r) {

	int ret = 0;
	if (r != NULL && r->m_sb.SocketBufHandle != NULL) //TODO check if need to check mrsb or just use r->m_sb.SocketBufHandle
	{
		MultiTcpRTMPSocketBuf *mrsb = (MultiTcpRTMPSocketBuf *)r->m_sb.SocketBufHandle;
		if (mrsb)
		{
			ret = mrsb->IsConnected();
		}
	}
	return ret;
}

int
MultiReadN(RTMP *r, char *buffer, int n)
{
	int nOriginalSize = n;
	int avail;
	char *ptr;
	//MultiTcpRTMPSocketBuf *mrsb=(MultiTcpRTMPSocketBuf *)r->m_sb.SocketBufHandle;

	ptr = buffer;
	while (n > 0)
	{
		int nRead = 0;
		if ((nRead = MultiRecv(&r->m_sb, ptr, n)) < 0) {
			RTMP_Log(RTMP_LOGERROR, "MultiRecv failed nRead=%d", nRead);
			RTMP_Log(RTMP_LOGDEBUG, "%s, RTMP socket closed by peer", __FUNCTION__);
			/*goto again; */
			RTMP_Close(r);
			break;
		}

		r->m_nBytesIn += nRead;

		if (r->m_bSendCounter
			&& r->m_nBytesIn > (r->m_nBytesInSent + r->m_nClientBW / 10)){
			if (!SendBytesReceived(r)){
				return FALSE;
			}
		}
		n -= nRead;
		ptr += nRead;
	}

	return nOriginalSize - n;
}

int
MultiSockBuf_Send(RTMPSockBuf *sb, const char *buf, int len) {

	int ret = -1;
	if (sb)
	{
		if (sb->SocketBufHandle)
		{
			MultiTcpRTMPSocketBuf *mrsb = (MultiTcpRTMPSocketBuf *)sb->SocketBufHandle;
			ret = mrsb->Write(buf, len);
			//RTMP_Log(RTMP_LOGINFO, "MultiTcpWrite write %d",ret);
		}
		else
		{
			RTMP_Log(RTMP_LOGERROR, "MultiTcpWrite RTMPSockBuf handle NULL");
		}
	}
	else
	{
		RTMP_Log(RTMP_LOGERROR, "MultiTcpWrite RTMPSockBuf NULL");
	}
	return ret;
}

int MultiRecv(RTMPSockBuf *sb, char *buf, int len)
{
	int ret = -1;
	if (sb)
	{
		if (sb->SocketBufHandle)
		{
			MultiTcpRTMPSocketBuf *mrsb = (MultiTcpRTMPSocketBuf *)sb->SocketBufHandle;
			ret = mrsb->Recv(buf, len);
			//RTMP_Log(RTMP_LOGINFO, "MultiRecv Recv %d",ret);
		}
		else
		{
			RTMP_Log(RTMP_LOGERROR, "MultiRecv RTMPSockBuf handle NULL");
		}
	}
	else
	{
		RTMP_Log(RTMP_LOGERROR, "MultiRecv RTMPSockBuf NULL");
	}
	return ret;
}

int MultiRecvBufferSize(RTMPSockBuf *sb)
{
	if (sb)
	{
		if (sb->SocketBufHandle)
		{
			MultiTcpRTMPSocketBuf *mrsb = (MultiTcpRTMPSocketBuf *)sb->SocketBufHandle;
			return mrsb->RecvBufferSize();
		}
	}

	return 0;
}

//int MultiWaitForRead(RTMPSockBuf *sb)
//{
//	MultiTcpRTMPSocketBuf *mrsb = (MultiTcpRTMPSocketBuf *)sb->SocketBufHandle;
//	if (mrsb)
//	{
//		mrsb->ReadForData();
//	}
//	return true;
//}

int MultiSockBuf_Close(RTMPSockBuf *sb)
{
	MultiDisconnectSock(sb);
	return 0;
}
int MultiRecvDiscardBytes(RTMP *r)
{
	MultiTcpRTMPSocketBuf *sb = (MultiTcpRTMPSocketBuf*)r->m_sb.SocketBufHandle;
	if (sb){
		return sb->ClearRecvBuf();
	}
	else {
		return 0;
	}
}

void MultiDisconnectSock(RTMPSockBuf *sb)
{
	//MultitcpDisconnect(&r->m_sb);

	if (sb && sb->SocketBufHandle != NULL)
	{
		MultiTcpRTMPSocketBuf *mrsb = (MultiTcpRTMPSocketBuf *)sb->SocketBufHandle;
		if (mrsb)
		{
			delete mrsb;
			mrsb = NULL;
			sb->SocketBufHandle = NULL;
		}
	}
}