#include "tiny_md5.h"

#include "tiny_byte_order.h"
namespace tiny
{
#undef BYTE_ORDER /* 1 = big-endian, -1 = little-endian, 0 = unknown */
#if defined(ARCH_IS_BIG_ENDIAN)
#define BYTE_ORDER (ARCH_IS_BIG_ENDIAN ? 1 : -1)
#else
#define BYTE_ORDER (0)
#endif

#define T_MASK ((md5_word_t)~0)
#define T1 /* 0xd76aa478 */ (T_MASK ^ 0x28955b87)
#define T2 /* 0xe8c7b756 */ (T_MASK ^ 0x173848a9)
#define T3 (0x242070db)
#define T4 /* 0xc1bdceee */ (T_MASK ^ 0x3e423111)
#define T5 /* 0xf57c0faf */ (T_MASK ^ 0x0a83f050)
#define T6 (0x4787c62a)
#define T7 /* 0xa8304613 */ (T_MASK ^ 0x57cfb9ec)
#define T8 /* 0xfd469501 */ (T_MASK ^ 0x02b96afe)
#define T9 (0x698098d8)
#define T10 /* 0x8b44f7af */ (T_MASK ^ 0x74bb0850)
#define T11 /* 0xffff5bb1 */ (T_MASK ^ 0x0000a44e)
#define T12 /* 0x895cd7be */ (T_MASK ^ 0x76a32841)
#define T13 (0x6b901122)
#define T14 /* 0xfd987193 */ (T_MASK ^ 0x02678e6c)
#define T15 /* 0xa679438e */ (T_MASK ^ 0x5986bc71)
#define T16 (0x49b40821)
#define T17 /* 0xf61e2562 */ (T_MASK ^ 0x09e1da9d)
#define T18 /* 0xc040b340 */ (T_MASK ^ 0x3fbf4cbf)
#define T19 (0x265e5a51)
#define T20 /* 0xe9b6c7aa */ (T_MASK ^ 0x16493855)
#define T21 /* 0xd62f105d */ (T_MASK ^ 0x29d0efa2)
#define T22 (0x02441453)
#define T23 /* 0xd8a1e681 */ (T_MASK ^ 0x275e197e)
#define T24 /* 0xe7d3fbc8 */ (T_MASK ^ 0x182c0437)
#define T25 (0x21e1cde6)
#define T26 /* 0xc33707d6 */ (T_MASK ^ 0x3cc8f829)
#define T27 /* 0xf4d50d87 */ (T_MASK ^ 0x0b2af278)
#define T28 (0x455a14ed)
#define T29 /* 0xa9e3e905 */ (T_MASK ^ 0x561c16fa)
#define T30 /* 0xfcefa3f8 */ (T_MASK ^ 0x03105c07)
#define T31 (0x676f02d9)
#define T32 /* 0x8d2a4c8a */ (T_MASK ^ 0x72d5b375)
#define T33 /* 0xfffa3942 */ (T_MASK ^ 0x0005c6bd)
#define T34 /* 0x8771f681 */ (T_MASK ^ 0x788e097e)
#define T35 (0x6d9d6122)
#define T36 /* 0xfde5380c */ (T_MASK ^ 0x021ac7f3)
#define T37 /* 0xa4beea44 */ (T_MASK ^ 0x5b4115bb)
#define T38 (0x4bdecfa9)
#define T39 /* 0xf6bb4b60 */ (T_MASK ^ 0x0944b49f)
#define T40 /* 0xbebfbc70 */ (T_MASK ^ 0x4140438f)
#define T41 (0x289b7ec6)
#define T42 /* 0xeaa127fa */ (T_MASK ^ 0x155ed805)
#define T43 /* 0xd4ef3085 */ (T_MASK ^ 0x2b10cf7a)
#define T44 (0x04881d05)
#define T45 /* 0xd9d4d039 */ (T_MASK ^ 0x262b2fc6)
#define T46 /* 0xe6db99e5 */ (T_MASK ^ 0x1924661a)
#define T47 (0x1fa27cf8)
#define T48 /* 0xc4ac5665 */ (T_MASK ^ 0x3b53a99a)
#define T49 /* 0xf4292244 */ (T_MASK ^ 0x0bd6ddbb)
#define T50 (0x432aff97)
#define T51 /* 0xab9423a7 */ (T_MASK ^ 0x546bdc58)
#define T52 /* 0xfc93a039 */ (T_MASK ^ 0x036c5fc6)
#define T53 (0x655b59c3)
#define T54 /* 0x8f0ccc92 */ (T_MASK ^ 0x70f3336d)
#define T55 /* 0xffeff47d */ (T_MASK ^ 0x00100b82)
#define T56 /* 0x85845dd1 */ (T_MASK ^ 0x7a7ba22e)
#define T57 (0x6fa87e4f)
#define T58 /* 0xfe2ce6e0 */ (T_MASK ^ 0x01d3191f)
#define T59 /* 0xa3014314 */ (T_MASK ^ 0x5cfebceb)
#define T60 (0x4e0811a1)
#define T61 /* 0xf7537e82 */ (T_MASK ^ 0x08ac817d)
#define T62 /* 0xbd3af235 */ (T_MASK ^ 0x42c50dca)
#define T63 (0x2ad7d2bb)
#define T64 /* 0xeb86d391 */ (T_MASK ^ 0x14792c6e)

	static void md5_process(md5_state_t* pms, const md5_byte_t* data /*[64]*/)
	{
		md5_word_t a = pms->abcd[0], b = pms->abcd[1], c = pms->abcd[2],
			d = pms->abcd[3];
		md5_word_t t;
#if BYTE_ORDER > 0
		/* Define storage only for big-endian CPUs. */
		md5_word_t X[16];
#else
		/* Define storage for little-endian or both types of CPUs. */
		md5_word_t xbuf[16];
		const md5_word_t* X;
#endif

		{
#if BYTE_ORDER == 0
			/*
			 * Determine dynamically whether this is a big-endian or
			 * little-endian machine, since we can use a more efficient
			 * algorithm on the latter.
			 */
			static const int w = 1;

			if (*((const md5_byte_t*)&w)) /* dynamic little-endian */
#endif
#if BYTE_ORDER <= 0 /* little-endian */
			{
				/*
				 * On little-endian machines, we can process properly aligned
				 * data without copying it.
				 */
				if (!((data - (const md5_byte_t*)0) & 3)) {
					/* data are properly aligned, a direct assignment is possible */
					/* cast through a (void *) should avoid a compiler warning,
					   see
					   https://github.com/bel2125/civetweb/issues/94#issuecomment-98112861
					   */
					X = (const md5_word_t*)(const void*)data;
				}
				else {
					/* not aligned */
					memcpy(xbuf, data, 64);
					X = xbuf;
				}
			}
#endif
#if BYTE_ORDER == 0
			else /* dynamic big-endian */
#endif
#if BYTE_ORDER >= 0 /* big-endian */
			{
				/*
				 * On big-endian machines, we must arrange the bytes in the
				 * right order.
				 */
				const md5_byte_t* xp = data;
				int i;

#if BYTE_ORDER == 0
				X = xbuf; /* (dynamic only) */
#else
#define xbuf X /* (static only) */
#endif
				for (i = 0; i < 16; ++i, xp += 4)
					xbuf[i] = (md5_word_t)(xp[0]) + (md5_word_t)(xp[1] << 8)
					+ (md5_word_t)(xp[2] << 16)
					+ (md5_word_t)(xp[3] << 24);
			}
#endif
		}

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

		/* Round 1. */
		/* Let [abcd k s i] denote the operation
		   a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define SET(a, b, c, d, k, s, Ti)                                              \
	t = a + F(b, c, d) + X[k] + Ti;                                            \
	a = ROTATE_LEFT(t, s) + b

		   /* Do the following 16 operations. */
		SET(a, b, c, d, 0, 7, T1);
		SET(d, a, b, c, 1, 12, T2);
		SET(c, d, a, b, 2, 17, T3);
		SET(b, c, d, a, 3, 22, T4);
		SET(a, b, c, d, 4, 7, T5);
		SET(d, a, b, c, 5, 12, T6);
		SET(c, d, a, b, 6, 17, T7);
		SET(b, c, d, a, 7, 22, T8);
		SET(a, b, c, d, 8, 7, T9);
		SET(d, a, b, c, 9, 12, T10);
		SET(c, d, a, b, 10, 17, T11);
		SET(b, c, d, a, 11, 22, T12);
		SET(a, b, c, d, 12, 7, T13);
		SET(d, a, b, c, 13, 12, T14);
		SET(c, d, a, b, 14, 17, T15);
		SET(b, c, d, a, 15, 22, T16);
#undef SET

		/* Round 2. */
		/* Let [abcd k s i] denote the operation
			 a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s). */
#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define SET(a, b, c, d, k, s, Ti)                                              \
	t = a + G(b, c, d) + X[k] + Ti;                                            \
	a = ROTATE_LEFT(t, s) + b

			 /* Do the following 16 operations. */
		SET(a, b, c, d, 1, 5, T17);
		SET(d, a, b, c, 6, 9, T18);
		SET(c, d, a, b, 11, 14, T19);
		SET(b, c, d, a, 0, 20, T20);
		SET(a, b, c, d, 5, 5, T21);
		SET(d, a, b, c, 10, 9, T22);
		SET(c, d, a, b, 15, 14, T23);
		SET(b, c, d, a, 4, 20, T24);
		SET(a, b, c, d, 9, 5, T25);
		SET(d, a, b, c, 14, 9, T26);
		SET(c, d, a, b, 3, 14, T27);
		SET(b, c, d, a, 8, 20, T28);
		SET(a, b, c, d, 13, 5, T29);
		SET(d, a, b, c, 2, 9, T30);
		SET(c, d, a, b, 7, 14, T31);
		SET(b, c, d, a, 12, 20, T32);
#undef SET

		/* Round 3. */
		/* Let [abcd k s t] denote the operation
			 a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s). */
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define SET(a, b, c, d, k, s, Ti)                                              \
	t = a + H(b, c, d) + X[k] + Ti;                                            \
	a = ROTATE_LEFT(t, s) + b

			 /* Do the following 16 operations. */
		SET(a, b, c, d, 5, 4, T33);
		SET(d, a, b, c, 8, 11, T34);
		SET(c, d, a, b, 11, 16, T35);
		SET(b, c, d, a, 14, 23, T36);
		SET(a, b, c, d, 1, 4, T37);
		SET(d, a, b, c, 4, 11, T38);
		SET(c, d, a, b, 7, 16, T39);
		SET(b, c, d, a, 10, 23, T40);
		SET(a, b, c, d, 13, 4, T41);
		SET(d, a, b, c, 0, 11, T42);
		SET(c, d, a, b, 3, 16, T43);
		SET(b, c, d, a, 6, 23, T44);
		SET(a, b, c, d, 9, 4, T45);
		SET(d, a, b, c, 12, 11, T46);
		SET(c, d, a, b, 15, 16, T47);
		SET(b, c, d, a, 2, 23, T48);
#undef SET

		/* Round 4. */
		/* Let [abcd k s t] denote the operation
			 a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s). */
#define I(x, y, z) ((y) ^ ((x) | ~(z)))
#define SET(a, b, c, d, k, s, Ti)                                              \
	t = a + I(b, c, d) + X[k] + Ti;                                            \
	a = ROTATE_LEFT(t, s) + b

			 /* Do the following 16 operations. */
		SET(a, b, c, d, 0, 6, T49);
		SET(d, a, b, c, 7, 10, T50);
		SET(c, d, a, b, 14, 15, T51);
		SET(b, c, d, a, 5, 21, T52);
		SET(a, b, c, d, 12, 6, T53);
		SET(d, a, b, c, 3, 10, T54);
		SET(c, d, a, b, 10, 15, T55);
		SET(b, c, d, a, 1, 21, T56);
		SET(a, b, c, d, 8, 6, T57);
		SET(d, a, b, c, 15, 10, T58);
		SET(c, d, a, b, 6, 15, T59);
		SET(b, c, d, a, 13, 21, T60);
		SET(a, b, c, d, 4, 6, T61);
		SET(d, a, b, c, 11, 10, T62);
		SET(c, d, a, b, 2, 15, T63);
		SET(b, c, d, a, 9, 21, T64);
#undef SET

		/* Then perform the following additions. (That is increment each
		   of the four registers by the value it had before this block
		   was started.) */
		pms->abcd[0] += a;
		pms->abcd[1] += b;
		pms->abcd[2] += c;
		pms->abcd[3] += d;
	}

	/*
		class MD5
	*/
	MD5::MD5()
	{
		Reset();
	}
	MD5& MD5::Reset()
	{
		memset(&ctx_, 0, sizeof(ctx_));
		memset(digest_, 0, sizeof(digest_));

		md5_state_t* pms = &ctx_;
		pms->count[0] = pms->count[1] = 0;
		pms->abcd[0] = 0x67452301;
		pms->abcd[1] = /*0xefcdab89*/ T_MASK ^ 0x10325476;
		pms->abcd[2] = /*0x98badcfe*/ T_MASK ^ 0x67452301;
		pms->abcd[3] = 0x10325476;

		return *this;
	}
	MD5& MD5::Update(const unsigned char* data, size_t nbytes)
	{
		md5_state_t* pms = &ctx_;

		const md5_byte_t* p = data;
		size_t left = nbytes;
		size_t offset = (pms->count[0] >> 3) & 63;
		md5_word_t nbits = (md5_word_t)(nbytes << 3);

		if (nbytes <= 0)
		{
			return *this;
		}

		/* Update the message length. */
		pms->count[1] += (md5_word_t)(nbytes >> 29);
		pms->count[0] += nbits;
		if (pms->count[0] < nbits)
			pms->count[1]++;

		/* Process an initial partial block. */
		if (offset) {
			size_t copy = (offset + nbytes > 64 ? 64 - offset : nbytes);

			memcpy(pms->buf + offset, p, copy);
			if (offset + copy < 64)
			{
				return *this;
			}
			p += copy;
			left -= copy;
			md5_process(pms, pms->buf);
		}

		/* Process full blocks. */
		for (; left >= 64; p += 64, left -= 64)
			md5_process(pms, p);

		/* Process a final partial block. */
		if (left)
			memcpy(pms->buf, p, left);

		return *this;
	}
	MD5& MD5::Update(const std::string& data)
	{
		if (data.empty())
		{
			return *this;
		}
		return Update((const unsigned char*)data.c_str(), data.size());
	}
	MD5& MD5::Update(const char* data, size_t len)
	{
		if (!data || (len == 0))
		{
			return *this;
		}
		return Update((const unsigned char*)data, len);
	}
	MD5& MD5::Final()
	{
		static const md5_byte_t pad[64] = { 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								   0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								   0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								   0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								   0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		md5_byte_t data[8];
		int i;

		/* Save the length before padding. */
		for (i = 0; i < 8; ++i)
			data[i] = (md5_byte_t)(ctx_.count[i >> 2] >> ((i & 3) << 3));
		/* Pad to 56 bytes mod 64. */
		Update(pad, ((55 - (ctx_.count[0] >> 3)) & 63) + 1);
		/* Append the length. */
		Update(data, 8);
		for (i = 0; i < 16; ++i)
			digest_[i] = (md5_byte_t)(ctx_.abcd[i >> 2] >> ((i & 3) << 3));
		return *this;
	}
	void MD5::Digest(unsigned char digest[16])
	{
		//memset(&digest, 0, sizeof(digest));
		memcpy(digest, digest_, sizeof(digest_));
	}
	std::string MD5::ToHex(bool is_upper) const
	{
		return StringHelper::ToHex((const char*)digest_, sizeof(digest_), is_upper);
	}
	std::string MD5::ToString() const
	{
		return std::string((const char*)digest_, sizeof(digest_));
	}
	std::string MD5::md5(const std::string& data)
	{
		MD5 m;
		return m.Update(data).Final().ToString();
	}
}