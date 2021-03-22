#ifndef TINY_STRING_H
#define	TINY_STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define STRINGIZE_NO_EXPANSION(x)				#x
#define STRINGIZE(x)							STRINGIZE_NO_EXPANSION(x)

#if defined(__GNUC__)

// optimize for the common case, which is very small copies
static inline void* maybe_inline_memcpy(void* dest, const void* src, size_t l,
	size_t inline_len)
	__attribute__((always_inline));

void* maybe_inline_memcpy(void* dest, const void* src, size_t l,
	size_t inline_len)
{
	if (l > inline_len) {
		return memcpy(dest, src, l);
	}
	switch (l) {
	case 8:
		return __builtin_memcpy(dest, src, 8);
	case 4:
		return __builtin_memcpy(dest, src, 4);
	case 3:
		return __builtin_memcpy(dest, src, 3);
	case 2:
		return __builtin_memcpy(dest, src, 2);
	case 1:
		return __builtin_memcpy(dest, src, 1);
	default:
		int cursor = 0;
		while (l >= sizeof(uint64_t)) {
			__builtin_memcpy((char*)dest + cursor, (char*)src + cursor,
				sizeof(uint64_t));
			cursor += sizeof(uint64_t);
			l -= sizeof(uint64_t);
		}
		while (l >= sizeof(uint32_t)) {
			__builtin_memcpy((char*)dest + cursor, (char*)src + cursor,
				sizeof(uint32_t));
			cursor += sizeof(uint32_t);
			l -= sizeof(uint32_t);
		}
		while (l > 0) {
			*((char*)dest + cursor) = *((char*)src + cursor);
			cursor++;
			l--;
		}
	}
	return dest;
}

#else

#define maybe_inline_memcpy(d, s, l, x) memcpy(d, s, l)

#endif


#if defined(__GNUC__) && defined(__x86_64__)

namespace tiny {
	typedef unsigned uint128_t __attribute__((mode(TI)));
}
using tiny::uint128_t;

static inline bool mem_is_zero(const char* data, size_t len)
__attribute__((always_inline));

bool mem_is_zero(const char* data, size_t len)
{
	// we do have XMM registers in x86-64, so if we need to check at least
	// 16 bytes, make use of them
	if (len / sizeof(uint128_t) > 0) {
		// align data pointer to 16 bytes, otherwise it'll segfault due to bug
		// in (at least some) GCC versions (using MOVAPS instead of MOVUPS).
		// check up to 15 first bytes while at it.
		while (((unsigned long long)data) & 15) {
			if (*(uint8_t*)data != 0) {
				return false;
			}
			data += sizeof(uint8_t);
			--len;
		}

		const char* data_start = data;
		const char* max128 = data + (len / sizeof(uint128_t)) * sizeof(uint128_t);

		while (data < max128) {
			if (*(uint128_t*)data != 0) {
				return false;
			}
			data += sizeof(uint128_t);
		}
		len -= (data - data_start);
	}

	const char* max = data + len;
	const char* max32 = data + (len / sizeof(uint32_t)) * sizeof(uint32_t);
	while (data < max32) {
		if (*(uint32_t*)data != 0) {
			return false;
		}
		data += sizeof(uint32_t);
	}
	while (data < max) {
		if (*(uint8_t*)data != 0) {
			return false;
		}
		data += sizeof(uint8_t);
	}
	return true;
}

#else  // gcc and x86_64

static inline bool mem_is_zero(const char* data, size_t len) {
	const char* end = data + len;
	const char* end64 = data + (len / sizeof(uint64_t)) * sizeof(uint64_t);

	while (data < end64) {
		if (*(uint64_t*)data != 0) {
			return false;
		}
		data += sizeof(uint64_t);
	}

	while (data < end) {
		if (*data != 0) {
			return false;
		}
		++data;
	}
	return true;
}
#endif //__GNUC__

#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>
#include <string>




namespace tiny
{

	/*
		class StringHelper
	*/
	class StringHelper
	{
	public:
		static std::string RandString(size_t len = 16);
		static std::string UUID(bool is_upper = false);
		static std::string ToHex(const char* data, size_t len, bool is_upper = false);
		static std::string ToHex(const std::string& data, bool is_upper = false);
		static std::string ToHex(const int i);
		static void Trim(std::string& data);
		static void Trim(std::string& data, char c);
		static bool StartWith(const std::string& src, const std::string& dst);
		static bool EndWith(const std::string& src, const std::string& dst);
		static std::string ValueOf(size_t d);
		static std::string ValueOf(int d);
		static std::string ValueOf(long d);
		static std::string ValueOf(double d, int bit = 0);
		static std::string ValueBytes(double bytes, int bit = 0);
		static std::string ValueKBytes(double kbytes, int bit = 0);
		static std::vector<std::string> Split(std::string& src, char flag);
		static size_t Split(const std::string& src, std::vector<std::string>& results, char flag);
		static void Reverse(std::string& src);
		static std::string UrlDecode(const char* src, size_t src_len, bool keep_space = true);
		static std::string UrlDecode(const std::string& src, bool keep_space = true) { return UrlDecode(src.c_str(), src.size(), keep_space); }
		static std::string UrlEncode(const char* src, size_t len, bool use_upper, bool space_as_plus = false);
		static std::string UrlEncode(const std::string& src, bool use_upper, bool space_as_plus = false) { return UrlEncode(src.c_str(), src.size(), use_upper, space_as_plus); }

		static std::string Base64Encode(const std::string &strString);
		static std::string Base64Decode(const std::string &strString);
	};


	/*
	class stringview
	*/
	class stringview
	{
	public:
		using traits_type = std::char_traits<char>;
		using value_type = char;
		using pointer = char*;
		using const_pointer = const char*;
		using reference = char&;
		using const_reference = const char&;
		using const_iterator = const_pointer;
		using iterator = const_iterator;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using reverse_iterator = const_reverse_iterator;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		static constexpr size_type npos = size_type(-1);
	public:
		stringview()
			: ptr_(nullptr)
			, len_(0)
		{

		}
		stringview(const char* ptr)
			: ptr_(ptr)
			, len_((!ptr) ? 0 : strlen(ptr))
		{

		}
		stringview(const char* ptr, size_t len)
			: ptr_(ptr)
			, len_(len)
		{

		}
		stringview(const stringview& sv)
			: ptr_(sv.ptr_)
			, len_(sv.len_)
		{

		}
		stringview(const std::string& str)
			: ptr_(str.c_str())
			, len_(str.size())
		{

		}
	public:
		char operator [] (size_t index) const
		{
			if (index >= len_)
			{
				return '\0';
			}
			return ptr_[index];
		}
		bool operator == (const stringview& sv) const
		{
			if (len_ != sv.len_)
			{
				return false;
			}
			if (len_ == 0)
			{
				return true;
			}
			return (memcmp(ptr_, sv.ptr_, len_) == 0);
		}
		bool operator != (const stringview& sv) const
		{
			return !(operator ==(sv));
		}
		bool operator == (const std::string& sv) const
		{
			return operator == (stringview(sv.c_str(), sv.size()));
		}
		bool operator == (const char* data) const
		{
			return operator == (stringview(data));
		}
		bool operator ! () const { return (len_ == 0); }
		operator bool() const { return (len_ > 0); }
		stringview& operator = (const stringview& sv)
		{
			ptr_ = sv.ptr_;
			len_ = sv.len_;
			return *this;
		}
		stringview& operator = (const std::string& str)
		{
			ptr_ = str.c_str();
			len_ = str.size();
			return *this;
		}
		stringview& operator = (const char* data)
		{
			ptr_ = data;
			len_ = ((!data) ? 0 : strlen(data));
			return *this;
		}
	public:
		const_iterator begin() const noexcept { return ptr_; }
		const_iterator cbegin() const noexcept { return ptr_; }
		const_iterator end() const noexcept { return ptr_ + len_; }
		const_iterator cend() const noexcept { return ptr_ + len_; }

		size_type size() const noexcept { return len_; }
		size_type length() const noexcept { return len_; }
		size_type max_size() const noexcept { return len_; }
	public:
		void clear() { ptr_ = nullptr; len_ = 0; }
		const char* c_str() const { return ptr_; }
		const char* data() const { return ptr_; }
		bool empty() const { return (len_ == 0); }
		stringview substr(size_type pos, size_type len = npos)
		{
			if (pos >= len_)
			{
				return stringview();
			}
			size_type nl = len_ - pos;
			if (nl > len)
			{
				nl = len;
			}
			return stringview((ptr_ + pos), nl);
		}
		size_type find(char c)
		{
			size_type pos = 0;
			while (pos < len_)
			{
				if (ptr_[pos] == c)
				{
					return pos;
				}
				++pos;
			}
			return npos;
		}
		size_type find(const stringview& sv)
		{
			if (sv.size() > len_)
			{
				return npos;
			}
			size_type pos = 0;
			while (pos <= (len_ - sv.size()))
			{
				if (0 == (memcmp(ptr_, sv.ptr_, sv.size())))
				{
					return pos;
				}
				++pos;
			}
			return pos;
		}
		size_type find(const char* data)
		{
			return find(stringview(data));
		}
		size_type find(const std::string& data)
		{
			return find(stringview(data));
		}
		std::string to_string() const
		{
			if (!ptr_ || len_ == 0)
			{
				return std::string();
			}
			return std::string(ptr_, len_);
		}
		stringview& trim()
		{
			while (len_ > 0)
			{
				if (isspace(front()))
				{
					pop_front();
				}
				else
				{
					break;
				}
			}
			while (len_ > 0)
			{
				if (isspace(back()))
				{
					pop_back();
				}
				else
				{
					break;
				}
			}
			return *this;
		}
		char at(size_t index) const
		{
			if (index >= len_)
			{
				return '\0';
			}
			return ptr_[index];
		}
		char front() const
		{
			if (len_ == 0)
			{
				return '\0';
			}
			return ptr_[0];
		}
		char back() const
		{
			if (len_ == 0)
			{
				return '\0';
			}
			return ptr_[len_ - 1];
		}
		void pop_front(size_t len = 1)
		{
			if (len_ == 0)
			{
				return;
			}

			if (len >= len_)
			{
				ptr_ = nullptr;
				len_ = 0;
				return;
			}
			ptr_ = (ptr_ + len);
			len_ -= len;
		}
		void pop_back(size_t len = 1)
		{
			if (len_ == 0 || (len >= len_))
			{
				ptr_ = nullptr;
				len_ = 0;
				return;
			}
			len_ -= len;
		}
		bool equals(const stringview& dst, bool nocase = false) const
		{
			if (size() != dst.size())
			{
				return false;
			}
			if (size() == 0)
			{
				return true;
			}
			if (!nocase)
			{
				return (memcmp(ptr_, dst.ptr_, len_) == 0);
			}
			
			for (size_t i = 0; i < len_; ++i)
			{
				char s1 = at(i);
				char s2 = dst.at(i);
				if (s1 >= 'A' && s1 <= 'Z')
				{
					s1 += 32;
				}
				if (s2 >= 'A' && s2 <= 'Z')
				{
					s2 += 32;
				}
				if (s1 != s2)
				{
					return false;
				}
			}
			return true;
		}
	private:
		const char* ptr_;
		size_t len_;
	};

	/*
		class StringBuilder
	*/
	class StringBuilder
	{
		StringBuilder() = delete;
		StringBuilder(const StringBuilder& ssb) = delete;
		StringBuilder(StringBuilder&& ssb) = delete;
	public:
		StringBuilder(char* buff, size_t len);
		~StringBuilder() = default;
	public:
		StringBuilder& AppendFormat(const char* fmt, ...);
		StringBuilder& Format(const char* fmt, ...);
		StringBuilder& Append(const char* str, size_t length);
	public:
		StringBuilder& operator<<(const char* str);
		StringBuilder& operator<<(char ch);
		StringBuilder& operator<<(const std::string& str);
		StringBuilder& operator<<(int i);
		StringBuilder& operator<<(unsigned i);
		StringBuilder& operator<<(long i);                // NOLINT
		StringBuilder& operator<<(long long i);           // NOLINT
		StringBuilder& operator<<(unsigned long i);       // NOLINT
		StringBuilder& operator<<(unsigned long long i);  // NOLINT
		StringBuilder& operator<<(float f);
		StringBuilder& operator<<(double f);
		StringBuilder& operator<<(long double f);
		const char* c_str() const { return bufptr_; }
		size_t size() const { return size_; }
		std::string str() const { return std::string(bufptr_, size_); }
	private:
		char* bufptr_;
		size_t size_;
		size_t capacity_;
	};


	/*
	*		class NumberString
	*/
	class NumberString
	{
	public:
		NumberString() = default;
		NumberString(const NumberString&) = default;
		NumberString(NumberString&&) = default;
		NumberString(const char* s) : str(s) {}
		NumberString(const std::string& s) : str(s) {}
		NumberString(const stringview& s) : str(s) {}
		~NumberString() = default;
	public:
		int asInt(int n = 0);
		unsigned asUnsigned(unsigned n = 0);
		long asLong(long n = 0);
		long long asLongLong(long long n = 0);
		unsigned long asUnsignedLong(unsigned long n = 0);
		unsigned long long asUnsignedLongLong(unsigned long long n = 0);
		float asFloat(float n = 0.0);
		double asDouble(double n = 0.0);
		long double asLongDouble(long double n = 0.0);
	private:
		stringview str;
	};

	template<std::size_t SIZE>
	class StackStringBuf : public std::basic_streambuf<char>
	{
	public:
		StackStringBuf()
			: vec(SIZE, 0)
		{
			setp(vec.data(), vec.data() + vec.size());
		}
		StackStringBuf(const StackStringBuf&) = delete;
		StackStringBuf& operator=(const StackStringBuf&) = delete;
		StackStringBuf(StackStringBuf&& o) = delete;
		StackStringBuf& operator=(StackStringBuf&& o) = delete;
		~StackStringBuf() override = default;

		void clear()
		{
			vec.resize(SIZE);
			setp(vec.data(), vec.data() + SIZE);
		}

		stringview strv() const
		{
			return stringview(pbase(), pptr() - pbase());
		}

	protected:
		std::streamsize xsputn(const char* s, std::streamsize n)
		{
			std::streamsize capacity = epptr() - pptr();
			std::streamsize left = n;
			if (capacity >= left) {
				maybe_inline_memcpy(pptr(), s, left, 32);
				pbump(left);
			}
			else {
				maybe_inline_memcpy(pptr(), s, capacity, 64);
				s += capacity;
				left -= capacity;
				vec.insert(vec.end(), s, s + left);
				setp(vec.data(), vec.data() + vec.size());
				pbump(vec.size());
			}
			return n;
		}

		int overflow(int c)
		{
			if (traits_type::not_eof(c)) {
				char str = traits_type::to_char_type(c);
				vec.push_back(str);
				return c;
			}
			else {
				return traits_type::eof();
			}
		}

	private:

		std::vector<char> vec;
	};


	template<std::size_t SIZE>
	class StackStringStream : public std::basic_ostream<char>
	{
	public:
		StackStringStream() : basic_ostream<char>(&ssb), default_fmtflags(flags()) {}
		StackStringStream(const StackStringStream& o) = delete;
		StackStringStream& operator=(const StackStringStream& o) = delete;
		StackStringStream(StackStringStream&& o) = delete;
		StackStringStream& operator=(StackStringStream&& o) = delete;
		~StackStringStream() override = default;

		void reset() {
			clear(); /* reset state flags */
			flags(default_fmtflags); /* reset fmtflags to constructor defaults */
			ssb.clear();
		}

		stringview strv() const {
			return ssb.strv();
		}

	private:
		StackStringBuf<SIZE> ssb;
		fmtflags const default_fmtflags;
	};

	template<typename T, typename... Ts>
	inline std::unique_ptr<T> make_unique(Ts&&... params)
	{
		return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
	}
	class CachedStackStringStream {
	public:
		using sss = StackStringStream<4096>;
		using osptr = std::unique_ptr<sss>;

		CachedStackStringStream() {
			if (cache.destructed || cache.c.empty()) {
				osp = make_unique<sss>();
			}
			else {
				osp = std::move(cache.c.back());
				cache.c.pop_back();
				osp->reset();
			}
		}
		CachedStackStringStream(const CachedStackStringStream&) = delete;
		CachedStackStringStream& operator=(const CachedStackStringStream&) = delete;
		CachedStackStringStream(CachedStackStringStream&&) = delete;
		CachedStackStringStream& operator=(CachedStackStringStream&&) = delete;
		~CachedStackStringStream() {
			if (!cache.destructed && cache.c.size() < max_elems) {
				cache.c.emplace_back(std::move(osp));
			}
		}

		sss& operator*() {
			return *osp;
		}
		sss const& operator*() const {
			return *osp;
		}
		sss* operator->() {
			return osp.get();
		}
		sss const* operator->() const {
			return osp.get();
		}

		sss const* get() const {
			return osp.get();
		}
		sss* get() {
			return osp.get();
		}

	private:
		static constexpr std::size_t max_elems = 8;

		/* The thread_local cache may be destructed before other static structures.
		 * If those destructors try to create a CachedStackStringStream (e.g. for
		 * logging) and access this cache, that access will be undefined. So note if
		 * the cache has been destructed and check before use.
		 */
		struct Cache {
			using container = std::vector<osptr>;

			Cache() {}
			~Cache() { destructed = true; }

			container c;
			bool destructed = false;
		};

		static thread_local Cache cache;
		osptr osp;
	};
}

inline std::ostream& operator << (std::ostream& out, const tiny::stringview& sv)
{
	if (!sv)
	{
		return out;
	}
	return out.write(sv.data(), sv.size());
}
#endif // !TINY_STRING_H
