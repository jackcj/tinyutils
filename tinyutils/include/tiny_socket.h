#ifndef TINY_SOCKET_H
#define	TINY_SOCKET_H

#include <string>

#include "tiny_byte_order.h"

#ifdef UNI_POSIX
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>
#include <netdb.h>
#elif UNI_WIN
#include <WinSock2.h>
#include <WS2tcpip.h>

#ifndef socklen_t
#define	socklen_t int
#endif // socklen_t

#endif // UNI_POSIX

#ifndef UNI_WIN
#ifndef closesocket
#define	closesocket		close
#endif // !closesocket
#ifndef SOCKET_ERROR
#define	SOCKET_ERROR		-1
#endif // !SOCKET_ERROR

#endif // !UNI_WIN

#include <fcntl.h>
extern std::ostream& operator<<(std::ostream& out, const sockaddr_storage& ss);
extern std::ostream& operator<<(std::ostream& out, const sockaddr* sa);
struct addrinfo;
namespace tiny
{
	/*
	class IPAddr
*/
	class IPAddr
	{
	public:
		IPAddr()
			: port_(0)
		{

		}
		IPAddr(const IPAddr& ia)
			: port_(ia.port_)
			, addr_(ia.addr_)
		{

		}
		IPAddr(IPAddr&& ia) noexcept
			: port_(0)
		{
			std::swap(ia.addr_, ia.addr_);
			std::swap(port_, ia.port_);
		}
		IPAddr(const std::string& ip, unsigned short port)
			: port_(0)
		{
			addr_ = ip;
			port_ = port;
		}
		IPAddr(const char* ip, unsigned int port)
			: port_(port)
		{
			set_ip(ip);
		}
		~IPAddr() = default;
	public:
		IPAddr& operator = (const IPAddr& ia)
		{
			addr_ = ia.addr_;
			port_ = ia.port_;
			return *this;
		}
		IPAddr& operator = (IPAddr&& ia) noexcept
		{
			std::swap(ia.addr_, ia.addr_);
			std::swap(port_, ia.port_);
			return *this;
		}
		bool operator ! () const
		{
			return (addr_.empty() && (port_ == 0));
		}
		bool operator == (const IPAddr& ia) const
		{
			if (port_ != ia.port_)
			{
				return false;
			}
			return (addr_ == ia.addr_);
		}
		bool operator < (const IPAddr& ia)
		{
			if (port_ == ia.port_)
			{
				return (addr_ < ia.addr_);
			}
			return (port_ < ia.port_);
		}
	public:
		bool empty() const { return (addr_.empty() && (port_ == 0)); }
		const std::string& ip() const { return addr_; }
		const unsigned short& port() const { return port_; }
		std::string& ip() { return addr_; }
		unsigned short& port() { return port_; }
		void set_ip(const std::string& ip) { addr_ = ip; }
		void set_ip(const char* ip) { addr_.clear();  if (!ip) return; addr_ = ip; }
		void set_port(unsigned short port) { port_ = port; }
		void clear() { addr_.clear(); port_ = 0; }
		void reset(const std::string& ip, unsigned short port) { addr_ = ip; port_ = port; }
		void reset(const char* ip, unsigned short port) { set_ip(ip); port_ = port; }
	private:
		unsigned short port_;
		std::string addr_;
	};
    /*
        class sockaddr_info_t
    */
    class sockaddr_info_t
    {
    public:
        sockaddr_info_t(int type = SOCK_STREAM);
        sockaddr_info_t(int domain, int type, int protocol, const char* addr = nullptr);
        ~sockaddr_info_t();
        sockaddr_info_t(const sockaddr_info_t&) = delete;
        sockaddr_info_t(sockaddr_info_t&&) = delete;
    public:
        bool operator ! () const { return (!info_); }
        operator struct addrinfo* () { return info_; }
        struct addrinfo* operator -> () { return info_; }
        sockaddr_info_t& operator = (const sockaddr_info_t&) = delete;
        sockaddr_info_t& operator = (sockaddr_info_t&&) = delete;
    public:
        void reset(int domain, int type, int protocol, const char* addr = nullptr);
        void reset();
    private:
        struct addrinfo* info_;
    };
	/*
	class sockaddr_t
*/
	class sockaddr_t
	{
	public:
		sockaddr_t() { memset(&addr, 0, sizeof(addr)); }
		sockaddr_t(const std::string& ip, unsigned int port)
		{
			memset(&addr, 0, sizeof(addr));
			set_addr(ip.c_str(), port);
		}
		sockaddr_t(const char* ip, unsigned int port)
		{
			memset(&addr, 0, sizeof(addr));
			set_addr(ip, port);
		}
		sockaddr_t(const sockaddr_t& sa)
		{
			memset(&addr, 0, sizeof(addr));
			memcpy(&addr, &(sa.addr), sizeof(addr));
		}
		sockaddr_t(const struct sockaddr* sa)
		{
			memset(&addr, 0, sizeof(addr));
			set_sockaddr(sa);
		}
		sockaddr_t(const struct sockaddr& sa)
		{
			memset(&addr, 0, sizeof(addr));
			set_sockaddr(sa);
		}
		~sockaddr_t() = default;
	public:
		sockaddr_t& operator = (const sockaddr_t& sa)
		{
			memset(&addr, 0, sizeof(addr));
			memcpy(&addr, &(sa.addr), sizeof(addr));
			return *this;
		}
		sockaddr_t& operator = (const struct sockaddr* sa)
		{
			memset(&addr, 0, sizeof(addr));
			set_sockaddr(sa);
			return *this;
		}
		sockaddr_t& operator = (const struct sockaddr& sa)
		{
			memset(&addr, 0, sizeof(addr));
			set_sockaddr(sa);
			return *this;
		}
		operator struct sockaddr* ()
		{
			return &(addr.sa);
		}
		bool operator ! () const
		{
			return ((is_blank_ip()) && (get_port() <= 0));
		}

		bool operator == (const sockaddr_t& sa) const
		{
			return (Compare(sa) == 0);
		}
		bool operator != (const sockaddr_t& sa) const
		{
			return (Compare(sa) != 0);
		}
		bool operator < (const sockaddr_t& sa) const
		{
			return (Compare(sa) < 0);
		}
		bool operator > (const sockaddr_t& sa) const
		{
			return (Compare(sa) > 0);
		}
		bool operator <= (const sockaddr_t& sa) const
		{
			return (Compare(sa) <= 0);
		}
		bool operator >= (const sockaddr_t& sa) const
		{
			return (Compare(sa) >= 0);
		}

	public:
		int Compare(const sockaddr_t& sa) const
		{
			return (memcmp(&addr, &(sa.addr), sizeof(addr)));
		}
		int get_family() const { return addr.sa.sa_family; }
		void set_family(int f) { addr.sa.sa_family = f; }
		bool is_v4() const { return (addr.sa.sa_family == AF_INET); }
		bool is_v6() const { return (addr.sa.sa_family == AF_INET6); }
		sockaddr_in& in4_addr() { return addr.sin; }
		const sockaddr_in& in4_addr()const { return addr.sin; }
		sockaddr_in6& in6_addr() { return addr.sin6; }
		const sockaddr_in6& in6_addr() const { return addr.sin6; }
		const sockaddr* get_sockaddr() const { return &(addr.sa); }
		size_t get_sockaddr_len() const
		{
			size_t ls = 0;
			switch (addr.sa.sa_family)
			{
			case AF_INET:
				ls = sizeof(addr.sin);
				break;
			case AF_INET6:
				ls = sizeof(addr.sin6);
				break;
			default:
				ls = sizeof(addr);
				break;
			}
			return ls;
		}
		bool set_sockaddr(const struct sockaddr& sa)
		{
			return set_sockaddr(&sa);
		}
		sockaddr_storage get_sockaddr_storage() const
		{
			sockaddr_storage ss;
			get_sockaddr_storage(ss);
			return ss;
		}
		void get_sockaddr_storage(sockaddr_storage& ss) const
		{
			size_t as = sizeof(addr);
			memcpy(&ss, &addr, as);
			memset((char*)&ss + as, 0, sizeof(ss) - as);
		}
		bool is_ip() const {
			switch (addr.sa.sa_family) {
			case AF_INET:
			case AF_INET6:
				return true;
			default:
				return false;
			}
		}

		bool is_blank_ip() const {
			switch (addr.sa.sa_family) {
			case AF_INET:
				return addr.sin.sin_addr.s_addr == INADDR_ANY;
			case AF_INET6:
				return memcmp(&addr.sin6.sin6_addr, &in6addr_any, sizeof(in6addr_any)) == 0;
			default:
				return true;
			}
		}
		void clear() { memset(&addr, 0, sizeof(addr)); }
		bool set_sockaddr(const struct sockaddr* sa);
		void set_in4_quad(int pos, int val);
		void set_port(int port);
		void set_ip(const char* ip);
		void set_addr(const char* ip, unsigned short port);
		void set_addr(const IPAddr& ia);
		int get_port() const;
		IPAddr Address() const;
	private:
		union {
			sockaddr sa;
			sockaddr_in sin;
			sockaddr_in6 sin6;
		} addr;
	};
    /*
        class socket_t
    */
    class socket_t
    {
    public:
        socket_t()
            : sock(-1)
        {

        }
        socket_t(int fd)
            : sock(fd)
        {

        }
        socket_t(int domain, int type, int protocol = 0)
            : sock(-1)
        {
            sock = socket(domain, type, protocol);
        }

        socket_t(socket_t&& s) noexcept
            : sock(-1)
        {
            std::swap(sock, s.sock);
        }
        socket_t(const socket_t&) = delete;
        ~socket_t()
        {
            if (sock == -1)
            {
                return;
            }
#ifdef UNI_WIN
            closesocket(sock);
#else
            close(sock);
#endif // UNI_WIN

        }
    public:
        bool operator ! () const { return (sock < 0); }
        socket_t& operator = (const socket_t&) = delete;
        socket_t& operator = (socket_t&& s) noexcept
        {
            std::swap(sock, s.sock);
            return *this;
        }
        operator int() { return sock; }
    public:
        bool valid() const { return (sock != -1); }
        int release() { int fd = sock; sock = -1; return fd; }
		void reset(int s = -1) 
		{
			if (sock != -1)
			{
				closesocket(sock);
			}
			sock = s;
		}
    private:
        int sock;
    };
}

extern std::ostream& operator<<(std::ostream& out, const tiny::sockaddr_t& addr);
namespace std {
	template<> struct hash<tiny::IPAddr>
	{
		size_t operator()(const tiny::IPAddr& m) const noexcept
		{
			return (std::hash<unsigned short>()(m.port())) ^ (std::hash<std::string>()(m.ip()) << 1);
		}
	};
} // namespace std




#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	///type: tcp/udp
	int create_socketpair(int fds[2],int type);
	int pipe_cloexec(int fds[2]);

	int socket_cloexec(int domain, int type, int protocol);
	int socket_setnonblock(int fd);
	int accept_cloexec(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
	int create_tcpserver_socket(tiny::sockaddr_t& addr, int backlog, bool reuseaddr, bool nonblock);
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif // !TINY_SOCKET_H
