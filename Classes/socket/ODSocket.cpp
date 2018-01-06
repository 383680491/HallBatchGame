#include "ODSocket.h"
#include <stdio.h>
 

#include "cocos/platform/CCPlatformConfig.h"
 
 
ODSocket::ODSocket(SOCKET sock)
{ 
	m_sock = sock;
}

ODSocket::~ODSocket()
{
	Close();
}

int ODSocket::Init()
{
#ifdef WIN32
	/*
	http://msdn.microsoft.com/zh-cn/vstudio/ms741563(en-us,VS.85).aspx

	typedef struct WSAData { 
	WORD wVersion;								//winsock version
	WORD wHighVersion;							//The highest version of the Windows Sockets specification that the Ws2_32.dll can support
	char szDescription[WSADESCRIPTION_LEN+1]; 
	char szSystemStatus[WSASYSSTATUS_LEN+1]; 
	unsigned short iMaxSockets; 
	unsigned short iMaxUdpDg; 
	char FAR * lpVendorInfo; 
	}WSADATA, *LPWSADATA; 
	*/
	WSADATA wsaData;
	//#define MAKEWORD(a,b) ((WORD) (((BYTE) (a)) | ((WORD) ((BYTE) (b))) << 8)) 
	WORD version = MAKEWORD(2, 0);
	int ret = WSAStartup(version, &wsaData);//win sock start up
	if ( ret ) {
		//		cerr << "Initilize winsock error !" << endl;
		return -1;
	}
#endif

	return 0;
}
//this is just for windows
int ODSocket::Clean()
{
#ifdef WIN32
	return (WSACleanup());
#endif
	return 0;
}

ODSocket& ODSocket::operator = (SOCKET s)
{
	m_sock = s;
	return (*this);
}

ODSocket::operator SOCKET ()
{
	return m_sock;
}
//create a socket object win/lin is the same
// af:
bool ODSocket::Create(int af, int type, int protocol)
{ 
	m_sock = socket(af, type, protocol);
	u_long mode = 1;
#ifdef WIN32
 	ioctlsocket(m_sock,FIONBIO,&mode);
#else
	fcntl(m_sock, F_SETFL, O_NONBLOCK); 
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
		int set = 1;
		setsockopt(m_sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
	#endif
#endif
	if ( m_sock == INVALID_SOCKET ) {
		return false;
	}
	return true;
} 
 
bool ODSocket::Connect(std::string  ip, unsigned short port)
{  
	struct sockaddr_in addr_v4;
	addr_v4.sin_family = AF_INET;
	addr_v4.sin_addr.s_addr = inet_addr(ip.c_str());
	addr_v4.sin_port = htons(port);
	int arrdlen = sizeof(addr_v4);
	sockaddr *svraddr = (struct sockaddr*)& addr_v4; 
	int ret = connect(m_sock, svraddr, arrdlen); 
	if ( ret == SOCKET_ERROR ) {
		return false;
	}
	return true;
}
bool ODSocket::Connect(sockaddr * ai_addr, size_t ai_addrlen)
{
	if (::connect(m_sock, ai_addr, ai_addrlen) < 0) { 
		log("can't connect errorno=%d,error=%s", errno,strerror(errno)); 
		return false;
	} 
	return true;
}
bool ODSocket::isConnected()
{
	struct timeval timeout;
	fd_set fd_write;

	int ret;
	FD_ZERO(&fd_write);
	FD_SET(m_sock, &fd_write);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	ret = select( m_sock + 1, 0, &fd_write, 0, &timeout);

	if (ret < 0)
	{
		return false;
	}
	if (FD_ISSET(m_sock, &fd_write))
	{
		return true;
	}
	return false;
} 
int ODSocket::Send(const char* buf, int len, int flags)
{
	int bytes;
	int count = 0;

	while ( count < len ) {

		bytes = send(m_sock, buf + count, len - count, flags);
		if ( bytes == -1 || bytes == 0 )
			return -1;
		count += bytes;
	} 

	return count;
}

int ODSocket::Recv(char* buf, int len, int flags)
{
	return (recv(m_sock, buf, len, flags));
}

int ODSocket::Close()
{
#ifdef WIN32
	return (closesocket(m_sock));
#else
	return (close(m_sock));
#endif
}

int ODSocket::GetError()
{
#ifdef WIN32
	return (WSAGetLastError());
#else
	return (errno);
#endif
}
 

 
