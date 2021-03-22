#ifndef TINY_MD5_H
#define	TINY_MD5_H

#include "tiny_string.h"
namespace tiny
{
	using md5_byte_t = unsigned char; /* 8-bit byte */
	using md5_word_t = unsigned int;  /* 32-bit word */
	typedef struct md5_state_s {
		md5_word_t count[2]; /* message length in bits, lsw first */
		md5_word_t abcd[4];  /* digest buffer */
		md5_byte_t buf[64];  /* accumulate block */
	} md5_state_t;
	class MD5
	{
	public:
		typedef struct {
			unsigned int count[2]; /* message length in bits, lsw first */
			unsigned int abcd[4];  /* digest buffer */
			unsigned char buf[64];  /* accumulate block */
		} context_t;
	public:
		MD5();
		MD5(const MD5&) = delete;
		MD5(MD5&&) = delete;
		~MD5() = default;
	public:
		MD5& operator = (const MD5&) = delete;
		MD5& operator = (MD5&&) = delete;
	public:
		MD5& Reset();
		MD5& Update(const unsigned char* data, size_t nbytes);
		MD5& Update(const std::string& data);
		MD5& Update(const char* data, size_t len);
		MD5& Final();
		void Digest(unsigned char digest[16]);
		const char* Digest() const { return (const char*)digest_; }
		std::string ToHex(bool is_upper = true) const;
		std::string ToString() const;
	public:
		static std::string md5(const std::string& data);
	private:
		md5_state_t ctx_;
		unsigned char digest_[16];
	};
}

#endif // !TINY_MD5_H
