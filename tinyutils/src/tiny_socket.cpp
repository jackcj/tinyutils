#include "tiny_socket.h"
#include "tiny_assert.h"

#ifdef UNI_WIN
#define	SockArgs		char*
#else
#define	SockArgs		void*
#endif // UNI_WIN

namespace tiny
{
	/*
		class sockaddr_t
	*/
	bool sockaddr_t::set_sockaddr(const sockaddr* sa)
	{
		if (!sa)
		{
			return false;
		}
		bool ret = false;
		memset(&addr, 0, sizeof(addr));
		switch (sa->sa_family)
		{
		case AF_INET:
			memcpy(&(addr.sin), sa, sizeof(addr.sin));
			ret = true;
			break;
		case AF_INET6:
			memcpy(&(addr.sin6), sa, sizeof(addr.sin6));
			ret = true;
			break;
		case AF_UNSPEC:
			ret = true;
			break;
		default:
			ret = false;
			break;
		}
		return ret;
	}
	void sockaddr_t::set_in4_quad(int pos, int val)
	{
		addr.sin.sin_family = AF_INET;
		unsigned char* ipq = (unsigned char*)&addr.sin.sin_addr.s_addr;
		ipq[pos] = val;
	}
	void sockaddr_t::set_port(int port)
	{
		switch (addr.sa.sa_family) {
		case AF_INET:
			addr.sin.sin_port = HostToNetwork16(port);
			break;
		case AF_INET6:
			addr.sin6.sin6_port = HostToNetwork16(port);
			break;
		default:
			tiny_abort("set port buf family unspec,maybe is blank ip");
		}
	}
	void sockaddr_t::set_ip(const char* ip)
	{
		struct in_addr a4;
		struct in6_addr a6;
		if (inet_pton(AF_INET, ip, &a4))
		{
			addr.sin.sin_addr.s_addr = a4.s_addr;
			addr.sa.sa_family = AF_INET;
		}
		else if (inet_pton(AF_INET6, ip, &a6))
		{
			addr.sa.sa_family = AF_INET6;
			memcpy(&addr.sin6.sin6_addr, &a6, sizeof(a6));
#if defined(SIN6_LEN) /* this define is required by IPv6 if used */
			addr.sin6.sin6_len = sizeof(struct sockaddr_in6);
#endif
		}
	}
	void sockaddr_t::set_addr(const char* ip, unsigned short port)
	{
		struct in_addr a4;
		struct in6_addr a6;
		if (inet_pton(AF_INET, ip, &a4))
		{
			addr.sin.sin_addr.s_addr = a4.s_addr;
			addr.sa.sa_family = AF_INET;
			addr.sin.sin_port = HostToNetwork16(port);
		}
		else if (inet_pton(AF_INET6, ip, &a6))
		{
			addr.sa.sa_family = AF_INET6;
			memcpy(&addr.sin6.sin6_addr, &a6, sizeof(a6));
			addr.sin6.sin6_port = HostToNetwork16(port);
#if defined(SIN6_LEN) /* this define is required by IPv6 if used */
			addr.sin6.sin6_len = sizeof(struct sockaddr_in6);
#endif
		}
	}
	void sockaddr_t::set_addr(const IPAddr& ia)
	{
		set_addr(ia.ip().c_str(), ia.port());
	}
	int sockaddr_t::get_port() const
	{
		switch (addr.sa.sa_family) {
		case AF_INET:
			return NetworkToHost16(addr.sin.sin_port);
			break;
		case AF_INET6:
			return NetworkToHost16(addr.sin6.sin6_port);
			break;
		}
		return 0;
	}
	IPAddr sockaddr_t::Address() const
	{
		const char* host_ip = NULL;
		char addr_buf[INET6_ADDRSTRLEN];
		switch (get_family()) {
		case AF_INET:
			host_ip = ::inet_ntop(AF_INET, (void*)&in4_addr().sin_addr,
				addr_buf, INET_ADDRSTRLEN);
			break;
		case AF_INET6:
			host_ip = ::inet_ntop(AF_INET6, (void*)&in6_addr().sin6_addr,
				addr_buf, INET6_ADDRSTRLEN);
			break;
		default:
			break;
		}
		return IPAddr(host_ip, get_port());
	}

	/*
		class sockaddr_info_t
	*/
	sockaddr_info_t::sockaddr_info_t(int domain, int type, int protocol, const char* addr)
		: info_(nullptr)
	{
		reset(domain, type, protocol, addr);
	}
	sockaddr_info_t::sockaddr_info_t(int type)
		: info_(nullptr)
	{
		reset(AF_INET, type, 0);
	}
	sockaddr_info_t::~sockaddr_info_t()
	{
		reset();
	}
	void sockaddr_info_t::reset(int domain, int type, int protocol, const char* addr)
	{
		reset();
		struct addrinfo addr_info;
		memset(&addr_info, 0, sizeof(addr_info));
		addr_info.ai_family = domain;
		addr_info.ai_socktype = type;
		addr_info.ai_protocol = protocol;
		if (!addr)
		{
			if (AF_INET6 == domain)
				addr = "0:0:0:0:0:0:0:1";
			else
				addr = "127.0.0.1";
		}
		getaddrinfo(addr, "0", &addr_info, &info_);
	}
	void sockaddr_info_t::reset()
	{
		if (info_)
		{
			freeaddrinfo(info_);
		}
		info_ = nullptr;
	}
}

std::ostream& operator<<(std::ostream& out, const sockaddr_storage& ss)
{
	char buf[NI_MAXHOST] = { 0 };
	char serv[NI_MAXSERV] = { 0 };
	size_t hostlen;

	if (ss.ss_family == AF_INET)
		hostlen = sizeof(struct sockaddr_in);
	else if (ss.ss_family == AF_INET6)
		hostlen = sizeof(struct sockaddr_in6);
	else
		hostlen = sizeof(struct sockaddr_storage);
	getnameinfo((struct sockaddr*)&ss, hostlen, buf, sizeof(buf),
		serv, sizeof(serv),
		NI_NUMERICHOST | NI_NUMERICSERV);
	if (ss.ss_family == AF_INET6)
		return out << '[' << buf << "]:" << serv;
	return out << buf << ':' << serv;
}

std::ostream& operator<<(std::ostream& out, const sockaddr* sa)
{
	char buf[NI_MAXHOST] = { 0 };
	char serv[NI_MAXSERV] = { 0 };
	size_t hostlen;

	if (sa->sa_family == AF_INET)
		hostlen = sizeof(struct sockaddr_in);
	else if (sa->sa_family == AF_INET6)
		hostlen = sizeof(struct sockaddr_in6);
	else
		hostlen = sizeof(struct sockaddr_storage);
	getnameinfo(sa, hostlen, buf, sizeof(buf),
		serv, sizeof(serv),
		NI_NUMERICHOST | NI_NUMERICSERV);
	if (sa->sa_family == AF_INET6)
		return out << '[' << buf << "]:" << serv;
	return out << buf << ':' << serv;
}
std::ostream& operator<<(std::ostream& out, const tiny::sockaddr_t& addr)
{
	out << addr.get_sockaddr();
	return out;
}



////////////////////////////////////////////////c function///////////////////////////////


int create_socketpair(int fds[2], int type)
{
#ifdef UNI_WIN
	tiny::sockaddr_info_t addr_info(AF_INET, type, 0);
	if (!addr_info)
	{
		return -1;
	}

	int opt = 1;
	if (type == SOCK_STREAM)
	{
		tiny::socket_t s1, s2;
		tiny::socket_t listener(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const SockArgs)&opt, sizeof(opt));
		if (SOCKET_ERROR == bind(listener, addr_info->ai_addr, addr_info->ai_addrlen))
		{
			return -1;
		}
		if (SOCKET_ERROR == getsockname(listener, addr_info->ai_addr, (int*)&addr_info->ai_addrlen))
		{
			return -2;
		}

		if (SOCKET_ERROR == listen(listener, 5))
		{
			return -3;
		}
		s1 = std::move(tiny::socket_t(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol));
		if (!s1)
		{
			return -4;
		}
		if (SOCKET_ERROR == connect(s1, addr_info->ai_addr, addr_info->ai_addrlen))
		{
			return -5;
		}
		s2 = std::move(tiny::socket_t(accept(listener, 0, 0)));
		if (!s2)
		{
			return -6;
		}
		fds[0] = s1.release();
		fds[1] = s2.release();
	}
	else if (type == SOCK_DGRAM)
	{
		tiny::socket_t server(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
		if (!server)
		{
			return -2;
		}

		setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const SockArgs)&opt, sizeof(opt));

		if (SOCKET_ERROR == bind(server, addr_info->ai_addr, addr_info->ai_addrlen))
		{
			return -3;
		}

		if (SOCKET_ERROR == getsockname(server, addr_info->ai_addr, (int*)&addr_info->ai_addrlen))
		{
			return -4;
		}

		tiny::socket_t client(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
		if (!client)
		{
			return -5;
		}
		tiny::sockaddr_info_t result(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
		if (!result)
		{
			return -6;
		}

		setsockopt(client, SOL_SOCKET, SO_REUSEADDR, (const SockArgs)&opt, sizeof(opt));
		if (SOCKET_ERROR == bind(client, result->ai_addr, result->ai_addrlen))
		{
			return -7;
		}

		if (SOCKET_ERROR == getsockname(client, result->ai_addr, (int*)&result->ai_addrlen))
		{
			return -8;
		}

		if (SOCKET_ERROR == connect(server, result->ai_addr, result->ai_addrlen))
		{
			return -9;
		}

		if (SOCKET_ERROR == connect(client, addr_info->ai_addr, addr_info->ai_addrlen))
		{
			return -10;
		}
		fds[0] = client.release();
		fds[1] = server.release();
	}
#else
#ifdef SOCK_CLOEXEC
	return socketpair(AF_UNIX, type | SOCK_CLOEXEC, 0, fds);
#else
	int sv[2];
	sv[0] = -1;
	sv[1] = -1;
	int rc = socketpair(AF_UNIX, type, protocol, sv);
	if (rc == -1)
	{
		return -2;
	}
	tiny::socket_t s1(sv[0]), s2(sv[1]);
	if (fcntl(sv[0], F_SETFD, FD_CLOEXEC) < 0)
	{
		return -3;
	}

	if (fcntl(sv[1], F_SETFD, FD_CLOEXEC) < 0)
	{
		return -4;
	}
	fds[0] = s1.release();
	fds[1] = s2.release();
#endif
#endif // UNI_WIN
	return 0;
}
int pipe_cloexec(int fds[2])
{
#ifdef UNI_WIN
	create_socketpair(fds, SOCK_DGRAM);
#else
#if defined(HAVE_PIPE2)
	return pipe2(fds, O_CLOEXEC);
#else
	int pipefd[2];
	pipefd[0] = -1;
	pipefd[1] = -1;
	if (pipe(pipefd) == -1)
	{
		return -2;
	}

	tiny::socket_t s1(pipefd[0]), s2(pipefd[1]);
	if (fcntl(pipefd[0], F_SETFD, FD_CLOEXEC) < 0)
	{
		return -3;
	}

	if (fcntl(pipefd[1], F_SETFD, FD_CLOEXEC) < 0)
	{
		return -4;
	}
	fds[0] = s1.release();
	fds[1] = s2.release();
#endif
#endif // UNI_WIN
	return 0;

}

int socket_setnonblock(int sd)
{
#ifndef UNI_WIN
	int flags;
	int r = 0;

	/* Set the socket nonblocking.
	 * Note that fcntl(2) for F_GETFL and F_SETFL can't be
	 * interrupted by a signal. */
	if ((flags = fcntl(sd, F_GETFL)) < 0) {
		r = errno;
		return -r;
	}
	if (fcntl(sd, F_SETFL, flags | O_NONBLOCK) < 0) {
		r = errno;
		return -r;
	}

#else
	unsigned long nonblocking = 1;
	if (SOCKET_ERROR == ioctlsocket(sd, FIONBIO, &nonblocking))
	{
		int r = errno;
		return -r;
	}
#endif // !LQ_WIN

	{
#ifndef UNI_WIN
#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
# ifdef SO_NOSIGPIPE
		int val = 1;
		int r = ::setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void*)&val, sizeof(val));
		if (r) {
			r = errno;
		}
# endif //SO_NOSIGPIPE
#else
		int val = 1;
		int r = ::setsockopt(sd, SOL_SOCKET, MSG_NOSIGNAL, (void*)&val, sizeof(val));
		if (r) {
			r = errno;
		}
#endif //!MSG_NOSIGNAL
#endif // LQ_WIN
	}

	return 0;
}

int socket_cloexec(int domain, int type, int protocol)
{
#ifdef SOCK_CLOEXEC
	return socket(domain, type | SOCK_CLOEXEC, protocol);
#else
	tiny::socket_t fd(domain, type, protocol);
	if (!fd)
	{
		return -1;
	}
#ifndef UNI_WIN
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
	{
		return -1;
	}
#endif // !LQ_WIN
	return fd.release();
#endif //SOCK_CLOEXEC
}

int accept_cloexec(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
#ifdef HAVE_ACCEPT4
	return accept4(sockfd, addr, addrlen, SOCK_CLOEXEC);
#else
	tiny::socket_t fd(accept(sockfd, addr, addrlen));
	if (!fd)
	{
		return -1;
	}
#ifndef UNI_WIN
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
	{
		return -1;
	}
#endif // !LQ_WIN
	return fd.release();
#endif
}

int create_tcpserver_socket(tiny::sockaddr_t& addr, int backlog, bool reuseaddr, bool nonblock)
{
	tiny::socket_t fd(socket_cloexec(addr.get_family(),SOCK_STREAM,0));
	if (!fd)
	{
		return -1;
	}
	if (reuseaddr)
	{
		int one = 1;
		if (0 != setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const SockArgs)&one, sizeof(int)))
		{
			return -1;
		}
	}
	if (::bind(fd, addr.get_sockaddr(), addr.get_sockaddr_len()) != 0)
	{
		return -1;
	}
	if (backlog <= 0)
	{
		backlog = 1024;
	}
	if (0 != ::listen(fd, backlog))
	{
		return -1;
	}
	if (0 != socket_setnonblock(fd))
	{
		return -1;
	}
	struct sockaddr_storage ss;
	socklen_t socklen = sizeof(ss);
	memset(&ss, 0, sizeof(ss));
	if (getsockname(fd, (struct sockaddr*)&ss, &socklen)) {
		perror("getsockname() failed");
		return -1;
	}
	addr.set_sockaddr((struct sockaddr*)&ss);
	return fd.release();
}