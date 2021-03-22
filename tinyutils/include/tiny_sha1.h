#ifndef TINY_SHA1_H
#define	TINY_SHA1_H

#include "tiny_string.h"
namespace tiny
{
	typedef struct {
		uint32_t state[5];
		uint32_t count[2];
		uint8_t buffer[64];
	} SHA_CTX;
	class SHA1
	{
	public:
		SHA1();
		SHA1(const SHA1&) = delete;
		SHA1(SHA1&&) = delete;
		~SHA1() = default;
	public:
		SHA1& operator = (const SHA1&) = delete;
		SHA1& operator = (SHA1&&) = delete;
	public:
		SHA1& Reset();
		SHA1& Update(const unsigned char* data, size_t len);
		SHA1& Update(const std::string& data);
		SHA1& Update(const char* data, size_t len);
		SHA1& Final();
		void Digest(unsigned char digest[20]);
		const char* Digest() const { return (const char*)digest_; }
		std::string ToHex(bool is_upper = true) const;
		std::string ToString() const;
	public:
		static std::string sha1(const std::string& data);
		static void HMAC(const unsigned char* key, size_t keylen,
			const unsigned char* data, size_t datalen,
			unsigned char out[20]);
		static void HMAC(const char* key, size_t keylen,
			const char* data, size_t datalen,
			char out[20]);
		static void HMAC(const std::string& key, const std::string& data, char out[20]);
		static std::string HMAC(const std::string& key, const std::string& data);
	private:
		SHA_CTX ctx_;
		unsigned char digest_[20];
	};
}
#endif // !TINY_SHA1_H
