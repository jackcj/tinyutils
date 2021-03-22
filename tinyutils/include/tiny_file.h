#ifndef TINY_BASE_FILE_H
#define	TINY_BASE_FILE_H

#include <stdio.h>
#include <stdlib.h>

#ifdef UNI_WIN
#define	TINY_FILE_SLASH									'\\'
#define	TINY_FILE_SLASH_STR								"\\"
#else
#define	TINY_FILE_SLASH									'/'
#define	TINY_FILE_SLASH_STR								"/"
#endif // LQ_WIN


#include "tiny_string.h"

#include "tiny_time.h"
namespace tiny
{
	class File
	{
	public:
		File();
		File(const std::string& dir, const std::string& name);
		File(const std::string& path);
		File(const File& f) = delete;
		File(File&& f) noexcept
			: file_(nullptr)
		{
			std::swap(file_, f.file_);
			std::swap(real_path_, f.real_path_);
		}
		~File();
	public:
		File& operator = (const File& f) = delete;
		File& operator = (File&& f) noexcept
		{
			std::swap(file_, f.file_);
			std::swap(real_path_, f.real_path_);
			return *this;
		}
		bool operator ! () const;
		operator bool() const;
	public:
		void SetPath(const std::string& path);
		void SetPath(const std::string& dir, const std::string& name);
		void Close();
		long Size() const;
		bool Open(const char* mode = "r");
		bool Open(const std::string& path, const char* mode = "r");
		size_t  Write(const char* data, size_t len);
		size_t Read(char* buf, size_t size);
		size_t  Write(const stringview& data);
		std::string ReadLine() const;
		size_t WriteLine(const stringview& data);
		std::string ReadAll() const;
		bool Eof() const;
		bool CopyTo(const std::string& real_path);
		bool CopyToDir(const std::string& dir, const std::string& name = std::string());
		int Flush();
		const std::string& Path() const { return real_path_; }
		utime_t GetCreateTime() const;
	public:
		static void RemoveFileSlash(std::string& path);
		static void AppendFileSlash(std::string& path);
		static bool Exists(const std::string& path);
		static bool IsDir(const std::string& path);
		static bool Delete(const std::string& path);
		static std::string RealPath(const std::string& path);
		static std::string Current();
		static std::string WorkDir();
		static std::string AppName();
		static bool CreateDir(const std::string& path);
	private:
		FILE* file_;
		std::string real_path_;
	};
}
#endif // !TINY_BASE_FILE_H
