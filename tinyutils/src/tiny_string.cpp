#include "tiny_string.h"


#include <stdarg.h>
#include <cstdio>
#include <cstring>

#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
namespace tiny
{
    thread_local CachedStackStringStream::Cache CachedStackStringStream::cache;

    /*
        class StringHelper
    */
    std::string StringHelper::RandString(size_t len)
    {
        if (len == 0 || len == std::string::npos)
        {
            return std::string();
        }
        std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
        while (len > str.size())
        {
            str.append("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
        }
        std::random_device rd;
        std::mt19937 generator(rd());
        std::shuffle(str.begin(), str.end(), generator);
        return str.substr(0, len);    // assumes 32 < number of characters in str   
    }
    std::string StringHelper::UUID(bool is_upper)
    {
        static std::random_device seed;
        static std::mt19937 PRNG(seed());

        std::uniform_int_distribution<uint32_t> uint_dist_long(0, UINT32_MAX);

        unsigned Node;//16 bits
        unsigned long long TimeEpoch; //52 bits
        unsigned long long RandomNumber; //60 bits

        RandomNumber = uint_dist_long(PRNG);
        RandomNumber <<= 32;
        RandomNumber |= uint_dist_long(PRNG);
        RandomNumber &= 0xFFFFFFFFFFFFFFF;

        std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch());
        TimeEpoch = ns.count() & 0xFFFFFFFFFFFFF;

        //std::uniform_int_distribution<unsigned> uint_dist_long(0, UINT16_MAX);
        Node = uint_dist_long(PRNG) & 0xFFFF;

        std::stringstream ssUUID;

        if (is_upper)
        {
            ssUUID << std::setiosflags(std::ios::uppercase) << std::hex << std::setfill('0');
        }
        else
        {
            ssUUID << std::hex << std::setfill('0');
        }


        ssUUID << std::setw(15) << RandomNumber; // 15 hex digits = 60 bit binary number
        ssUUID << std::setw(13) << TimeEpoch;
        ssUUID << std::setw(4) << Node;

        return ssUUID.str().insert(8, 1, '-').insert(13, 1, '-').insert(18, 1, '-').insert(23, 1, '-');
    }

    std::string StringHelper::ToHex(const char* data, size_t len, bool is_upper)
    {
        std::string ret;
        if (!data || (len == 0))
        {
            return ret;
        }
        static const char* hex = "0123456789abcdef";
        char tmp_ch[3] = { 0 };
        unsigned char* p = (unsigned char*)data;
        for (size_t pos = 0; pos < len; ++pos) {
            tmp_ch[0] = hex[p[pos] >> 4];
            tmp_ch[1] = hex[p[pos] & 0x0f];
            if (is_upper)
            {
                tmp_ch[0] = ((tmp_ch[0] >= 'a') ? (tmp_ch[0] - 32) : tmp_ch[0]);
                tmp_ch[1] = ((tmp_ch[1] >= 'a') ? (tmp_ch[1] - 32) : tmp_ch[1]);
            }

            ret.append(tmp_ch, 2);
        }
        return ret;
    }

    std::string StringHelper::ToHex(const std::string& data, bool is_upper)
    {
        if (data.empty())
        {
            return std::string();
        }
        return ToHex(data.c_str(), data.size(), is_upper);
    }
    std::string StringHelper::ToHex(const int i)
    {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "%x", i);
        return std::string(buffer);
    }

    void StringHelper::Trim(std::string& data)
    {
        while (!data.empty())
        {
            if (!isspace(data.front()))
            {
                break;
            }
            data.erase(0, 1);
        }

        while (!data.empty())
        {
            if (!isspace(data.back()))
            {
                break;
            }
            data.pop_back();
        }
    }
    void StringHelper::Trim(std::string& data, char c)
    {
        while (!data.empty())
        {
            if (data.front() != c)
            {
                break;
            }
            data.erase(0, 1);
        }

        while (!data.empty())
        {
            if (data.back() != c)
            {
                break;
            }
            data.pop_back();
        }
    }
    bool StringHelper::StartWith(const std::string& src, const std::string& dst)
    {
        if (src.empty() || dst.empty() || (src.size() < dst.size()))
        {
            return false;
        }
        return (0 == memcmp(src.c_str(), dst.c_str(), dst.size()));
    }
    bool StringHelper::EndWith(const std::string& src, const std::string& dst)
    {
        if (src.empty() || dst.empty() || (src.size() < dst.size()))
        {
            return false;
        }
        size_t pos = src.size() - dst.size();
        return (0 == memcmp((src.c_str()+pos), dst.c_str(), dst.size()));
    }
    std::string StringHelper::ValueOf(size_t d)
    {
        char buf[64] = { 0 };
        size_t bs = sprintf(buf, "%lu", d);
        return std::string(buf, bs);
    }
    std::string StringHelper::ValueOf(int d)
    {
        char buf[64] = { 0 };
        size_t bs = sprintf(buf, "%d", d);
        return std::string(buf, bs);
    }
    std::string StringHelper::ValueOf(long d)
    {
        char buf[64] = { 0 };
        size_t bs = sprintf(buf, "%ld", d);
        return std::string(buf, bs);
    }
    std::string StringHelper::ValueOf(double d, int bit)
    {
        char buf[64] = { 0 };
        size_t bs = 0;
        switch (bit)
        {
        case 1:
            bs = sprintf(buf, "%0.1lf", d);
            break;
        case 2:
            bs = sprintf(buf, "%0.2lf", d);
            break;
        case 3:
            bs = sprintf(buf, "%0.3lf", d);
            break;
        case 4:
            bs = sprintf(buf, "%0.4lf", d);
            break;
        case 5:
            bs = sprintf(buf, "%0.5lf", d);
            break;
        default:
            bs = sprintf(buf, "%lf", d);
            break;
        }
        return std::string(buf, bs);
    }
    std::string StringHelper::ValueBytes(double bytes, int bit)
    {
        static char byte_names[] = { 'B','K','M','G','T' };
        int index = 0;
        while (bytes > 1024.000000 && (index < 5))
        {
            bytes /= 1024.000000;
            ++index;
        }
        std::string val = ValueOf(bytes, bit);
        val.append(" ").append(&(byte_names[index]), 1);
        return val;
    }
    std::string StringHelper::ValueKBytes(double kbytes, int bit)
    {
        static char byte_names[] = { 'K','M','G','T' ,'P','E' };
        int index = 0;
        while (kbytes > 1024.000000 && (index < 5))
        {
            kbytes /= 1024.000000;
            ++index;
        }
        std::string val = ValueOf(kbytes, bit);
        val.append(" ").append(&(byte_names[index]), 1);
        return val;
    }

    std::vector<std::string> StringHelper::Split(std::string& src, char flag)
    {
        std::vector<std::string> results;
        while (!src.empty())
        {
            std::string::size_type pos = src.find_first_of(flag);
            if (pos == 0)
            {
                src.erase(0, 1);
                continue;
            }
            else if (pos == std::string::npos)
            {
                results.push_back(src);
                src.clear();
            }
            else
            {
                results.push_back(src.substr(0, pos));
                src.erase(0, pos + 1);
            }
        }
        return results;
    }

    size_t StringHelper::Split(const std::string& src, std::vector<std::string>& results, char flag)
    {
        stringview tmp(src);
        while (!tmp.empty())
        {
            std::string::size_type pos = tmp.find(flag);
            if (pos == 0)
            {
                tmp.pop_front();
                continue;
            }
            else if (pos == std::string::npos)
            {
                results.push_back(tmp.to_string());
                tmp.clear();
            }
            else
            {
                results.push_back(tmp.substr(0, pos).to_string());
                tmp.pop_front(pos + 1);
            }
        }
        return results.size();
    }
    void StringHelper::Reverse(std::string& src) {
        size_t s = src.size();
        char c;
        for (size_t i = 0, j = s - 1; i < j; ++i, --j) {
            c = src[i];
            src[i] = src[j];
            src[j] = c;
        }
    }

    std::string StringHelper::UrlEncode(const char* src, size_t len, bool use_upper, bool space_as_plus)
    {
        if (!src || len == 0)
        {
            return std::string();
        }
        static std::string need_encoded = "._-$,;~()/";
        static std::string upper_hex = "0123456789ABCDEF";
        static std::string lower_hex = "0123456789abcdef";

        const char* hex = ((use_upper == 0) ? lower_hex.c_str() : upper_hex.c_str());
        std::string encoded;
        for (size_t i = 0; i < len; i++) {
            const unsigned char c = *((const unsigned char*)src + i);
            if (isalnum(c) || (std::string::npos != need_encoded.find(src + i))) {
                encoded.append(src + i, 1);
            }
            else if (c == ' ' && (space_as_plus != 0)) {
                encoded.append("+");
            }
            else {
                encoded.append("%");
                encoded.append(&hex[c >> 4], 1);
                encoded.append(&hex[c & 15], 1);
            }
        }
        return encoded;
    }

	std::string StringHelper::Base64Encode(const std::string & strString)
	{
		int nByteSrc = strString.length();
		std::string pszSource = strString;

		int i = 0;
		for (i=0; i < nByteSrc; i++)
			if (pszSource[i] < 0 || pszSource[i] > 127)
				throw "can not encode Non-ASCII characters";

		const char *enkey = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string pszEncode(nByteSrc * 4 / 3 + 4, '\0');
		int nLoop = nByteSrc % 3 == 0 ? nByteSrc : nByteSrc - 3;
		int n = 0;
		for (i = 0; i < nLoop; i += 3)
		{
			pszEncode[n] = enkey[pszSource[i] >> 2];
			pszEncode[n + 1] = enkey[((pszSource[i] & 3) << 4) | ((pszSource[i + 1] & 0xF0) >> 4)];
			pszEncode[n + 2] = enkey[((pszSource[i + 1] & 0x0f) << 2) | ((pszSource[i + 2] & 0xc0) >> 6)];
			pszEncode[n + 3] = enkey[pszSource[i + 2] & 0x3F];
			n += 4;
		}

		switch (nByteSrc % 3)
		{
		case 0:
			pszEncode[n] = '\0';
			break;

		case 1:
			pszEncode[n] = enkey[pszSource[i] >> 2];
			pszEncode[n + 1] = enkey[((pszSource[i] & 3) << 4) | ((0 & 0xf0) >> 4)];
			pszEncode[n + 2] = '=';
			pszEncode[n + 3] = '=';
			pszEncode[n + 4] = '\0';
			break;

		case 2:
			pszEncode[n] = enkey[pszSource[i] >> 2];
			pszEncode[n + 1] = enkey[((pszSource[i] & 3) << 4) | ((pszSource[i + 1] & 0xf0) >> 4)];
			pszEncode[n + 2] = enkey[((pszSource[i + 1] & 0xf) << 2) | ((0 & 0xc0) >> 6)];
			pszEncode[n + 3] = '=';
			pszEncode[n + 4] = '\0';
			break;
		}

		return pszEncode.c_str();
	}

	std::string StringHelper::Base64Decode(const std::string & strString)
	{
		int nByteSrc = strString.length();
		std::string pszSource = strString;

		const int dekey[] = {
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			62, // '+'
			-1, -1, -1,
			63, // '/'
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
			-1, -1, -1, -1, -1, -1, -1,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
			13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
			-1, -1, -1, -1, -1, -1,
			26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
			39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
		};

		if (nByteSrc % 4 != 0)
			throw "bad base64 string";

		std::string pszDecode(nByteSrc * 3 / 4 + 4, '\0');
		int nLoop = pszSource[nByteSrc - 1] == '=' ? nByteSrc - 4 : nByteSrc;
		int b[4] = { 0 };
		int i = 0, n = 0;
		for (i = 0; i < nLoop; i += 4)
		{
			b[0] = dekey[(int)pszSource[i]];        b[1] = dekey[(int)pszSource[i + 1]];
			b[2] = dekey[(int)pszSource[i + 2]];    b[3] = dekey[(int)pszSource[i + 3]];
			if (b[0] == -1 || b[1] == -1 || b[2] == -1 || b[3] == -1)
				throw "bad base64 string";

			pszDecode[n] = (b[0] << 2) | ((b[1] & 0x30) >> 4);
			pszDecode[n + 1] = ((b[1] & 0xf) << 4) | ((b[2] & 0x3c) >> 2);
			pszDecode[n + 2] = ((b[2] & 0x3) << 6) | b[3];

			n += 3;
		}

		if (pszSource[nByteSrc - 1] == '=' && pszSource[nByteSrc - 2] == '=')
		{
			b[0] = dekey[(int)pszSource[i]];        b[1] = dekey[(int)pszSource[i + 1]];
			if (b[0] == -1 || b[1] == -1)
				throw "bad base64 string";

			pszDecode[n] = (b[0] << 2) | ((b[1] & 0x30) >> 4);
			pszDecode[n + 1] = '\0';
		}

		if (pszSource[nByteSrc - 1] == '=' && pszSource[nByteSrc - 2] != '=')
		{
			b[0] = dekey[(int)pszSource[i]];        b[1] = dekey[(int)pszSource[i + 1]];
			b[2] = dekey[(int)pszSource[i + 2]];
			if (b[0] == -1 || b[1] == -1 || b[2] == -1)
				throw "bad base64 string";

			pszDecode[n] = (b[0] << 2) | ((b[1] & 0x30) >> 4);
			pszDecode[n + 1] = ((b[1] & 0xf) << 4) | ((b[2] & 0x3c) >> 2);
			pszDecode[n + 2] = '\0';
		}

		if (pszSource[nByteSrc - 1] != '=' && pszSource[nByteSrc - 2] != '=')
			pszDecode[n] = '\0';

		return pszDecode;
	}

    std::string StringHelper::UrlDecode(const char* src, size_t src_len, bool keep_space)
    {

        std::string decoded;
        if (!src || src_len == 0)
        {
            return decoded;
        }
        char ch[2] = { 0 };
        int a, b;
#define HEXTOI(x) (isdigit(x) ? x - '0' : x - 'W')

        for (size_t i = 0; i < src_len; ++i)
        {
            if (src[i] == '%') {
                if (i < src_len - 2 && isxdigit(*(const unsigned char*)(src + i + 1)) &&
                    isxdigit(*(const unsigned char*)(src + i + 2))) {
                    a = tolower(*(const unsigned char*)(src + i + 1));
                    b = tolower(*(const unsigned char*)(src + i + 2));
                    ch[0] = (char)((HEXTOI(a) << 4) | HEXTOI(b));
                    i += 2;
                }
                else
                {
                    return std::string();
                }
            }
            else if (keep_space && src[i] == '+') {
                ch[0] = ' ';
            }
            else {
                ch[0] = src[i];
            }
            decoded.append(ch, 1);
        }
        return decoded;
    }

    /*
        class StringBuilder
    */
    StringBuilder::StringBuilder(char* buff, size_t len)
        : bufptr_(buff)
        , size_(0)
        , capacity_(len)
    {
        memset(bufptr_, 0, capacity_);
    }
    StringBuilder& StringBuilder::AppendFormat(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        const int len =
            std::vsnprintf((bufptr_ + size_), capacity_ - size_, fmt, args);
        if (len >= 0)
        {
            size_t ls = capacity_ - size_ - 1;
            size_ += ((static_cast<size_t>(len) < ls) ? len : ls);
        }
        else
        {
            bufptr_[size_] = 0;
        }
        va_end(args);
        return *this;
    }

    StringBuilder& StringBuilder::Format(const char* fmt, ...)
    {
        memset(bufptr_, 0, capacity_);
        size_ = 0;

        va_list args;
        va_start(args, fmt);
        const int len =
            std::vsnprintf((bufptr_ + size_), capacity_ - size_, fmt, args);
        if (len >= 0)
        {
            size_t ls = capacity_ - size_ - 1;
            size_ += ((static_cast<size_t>(len) < ls) ? len : ls);
        }
        else
        {
            bufptr_[size_] = 0;
        }
        va_end(args);
        return *this;
    }

    StringBuilder& StringBuilder::Append(const char* str, size_t length)
    {
        size_t ls = capacity_ - size_;
        if (ls == 0)
        {
            return *this;
        }
        ls = ((ls < length) ? ls : length);
        memcpy((bufptr_ + size_), str, ls);
        size_ += ls;
        return *this;
    }


    StringBuilder& StringBuilder::operator<<(const char* str) {
        return Append(str, strlen(str));
    }

    StringBuilder& StringBuilder::operator<<(char ch) {
        return Append(&ch, 1);
    }

    StringBuilder& StringBuilder::operator<<(const std::string& str) {
        return Append(str.c_str(), str.length());
    }

    // Numeric conversion routines.
    //
    // We use std::[v]snprintf instead of std::to_string because:
    // * std::to_string relies on the current locale for formatting purposes,
    //   and therefore concurrent calls to std::to_string from multiple threads
    //   may result in partial serialization of calls
    // * snprintf allows us to print the number directly into our buffer.
    // * avoid allocating a std::string (potential heap alloc).
    // TODO(tommi): Switch to std::to_chars in C++17.

    StringBuilder& StringBuilder::operator<<(int i) {
        return AppendFormat("%d", i);
    }

    StringBuilder& StringBuilder::operator<<(unsigned i) {
        return AppendFormat("%u", i);
    }

    StringBuilder& StringBuilder::operator<<(long i) {  // NOLINT
        return AppendFormat("%ld", i);
    }

    StringBuilder& StringBuilder::operator<<(long long i) {  // NOLINT
        return AppendFormat("%lld", i);
    }

    StringBuilder& StringBuilder::operator<<(
        unsigned long i) {  // NOLINT
        return AppendFormat("%lu", i);
    }

    StringBuilder& StringBuilder::operator<<(
        unsigned long long i) {  // NOLINT
        return AppendFormat("%llu", i);
    }

    StringBuilder& StringBuilder::operator<<(float f) {
        return AppendFormat("%g", f);
    }

    StringBuilder& StringBuilder::operator<<(double f) {
        return AppendFormat("%lg", f);
    }

    StringBuilder& StringBuilder::operator<<(long double f) {
        return AppendFormat("%Lg", f);
    }

    /*
    *   class NumberString
    */
    int NumberString::asInt(int n)
    {

        int v = n;
        if (str.empty() || (sscanf(str.c_str(), "%d", &v) != 1))
        {
            return n;
        }
        return v;
    }

    unsigned NumberString::asUnsigned(unsigned n)
    {
        unsigned v = n;
        if (str.empty() || (sscanf(str.c_str(), "%u", &v) != 1))
        {
            return n;
        }
        return v;
    }

    long NumberString::asLong(long n)
    {
        long v = n;
        if (str.empty() || (sscanf(str.c_str(), "%ld", &v) != 1))
        {
            return n;
        }
        return v;
    }

    long long NumberString::asLongLong(long long n)
    {
        long long v = n;
        if (str.empty() || (sscanf(str.c_str(), "%lld", &v) != 1))
        {
            return n;
        }
        return v;
    }

    unsigned long NumberString::asUnsignedLong(unsigned long n)
    {
        unsigned long v = n;
        if (str.empty() || (sscanf(str.c_str(), "%lu", &v) != 1))
        {
            return n;
        }
        return v;
    }

    unsigned long long NumberString::asUnsignedLongLong(unsigned long long n)
    {
        unsigned long long v = n;
        if (str.empty() || (sscanf(str.c_str(), "%llu", &v) != 1))
        {
            return n;
        }
        return v;
    }

    float NumberString::asFloat(float n)
    {
        float v = n;
        if (str.empty() || (sscanf(str.c_str(), "%g", &v) != 1))
        {
            return n;
        }
        return v;
    }

    double NumberString::asDouble(double n)
    {
        double v = n;
        if (str.empty() || (sscanf(str.c_str(), "%lg", &v) != 1))
        {
            return n;
        }
        return v;
    }

    long double NumberString::asLongDouble(long double n)
    {
        long double v = n;
        if (str.empty() || (sscanf(str.c_str(), "%Lg", &v) != 1))
        {
            return n;
        }
        return v;
    }

}