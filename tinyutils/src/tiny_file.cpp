#include "tiny_file.h"
#include <fcntl.h> 
#include <errno.h>
#ifdef UNI_WIN
#include <Windows.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif // UNI_WIN


#include <limits.h>
#include <iostream>
namespace tiny
{
	File::File()
		: file_(nullptr)
	{
	}

	File::File(const std::string& dir, const std::string& name)
		: file_(nullptr)
	{
		SetPath(dir, name);
	}

	File::File(const std::string& path)
		: file_(nullptr)
	{
		SetPath(path);
	}

	File::~File()
	{
		Close();
	}
	bool File::operator!() const
	{
		return (!file_);
	}
	File::operator bool() const
	{
		return ((file_ != nullptr) && (feof(file_) == 0));
	}
	void File::SetPath(const std::string& path)
	{
		Close();
		real_path_ = RealPath(path);
		RemoveFileSlash(real_path_);
	}
	void File::SetPath(const std::string& dir, const std::string& name)
	{
		std::string tmp = dir;
		AppendFileSlash(tmp);
		tmp.append(name);
		RemoveFileSlash(tmp);
		SetPath(tmp);
	}
	void File::Close()
	{
		if (!file_)
		{
			return;
		}
		fclose(file_);
		file_ = nullptr;
	}
	long File::Size() const
	{
		struct stat st;
		if (!file_)
		{
			::stat(real_path_.c_str(), &st);
		}
		else
		{
#ifdef UNI_WIN
			::fstat(_fileno(file_), &st);
#else
			::fstat(fileno(file_), &st);
#endif // UNI_WIN
		}
		return st.st_size;
	}
	bool File::Open(const char* mode)
	{
		Close();
		file_ = fopen(real_path_.c_str(), mode);
		return (file_ != nullptr);
	}
	bool File::Open(const std::string& path, const char* mode)
	{
		SetPath(path);
		return Open(mode);
	}
	size_t File::Write(const char* data, size_t len)
	{
		return Write(stringview(data, len));
	}
	size_t File::Read(char* buf, size_t size)
	{
		if (!file_ || feof(file_))
		{
			return 0;
		}
		return fread(buf, 1, size, file_);
	}
	size_t File::Write(const stringview& data)
	{
		if (data.empty())
		{
			return 0;
		}
		return fwrite(data.c_str(), 1, data.size(), file_);
	}
	std::string File::ReadLine() const
	{
		std::string line;
		if (Eof())
		{
			return line;
		}
		char buf[4096] = { 0 };
		while (fgets(buf, sizeof(buf), file_))
		{
			line.append(buf);
			memset(buf, 0, sizeof(buf));
			if (Eof())
			{
				break;
			}
			if (line.back() == '\n')
			{
				line.pop_back();
				break;
			}
		}
		return line;
	}
	size_t File::WriteLine(const stringview& data)
	{
		if (data.empty())
		{
			return 0;
		}
		size_t ws = Write(data.c_str(), data.size());
		ws += Write("\n", 1);
		return ws;
	}
	std::string File::ReadAll() const
	{
		std::string data;
		char buf[4096] = { 0 };
		while (!Eof())
		{
			size_t rs = fread(buf, 1, sizeof(buf),file_);
			if (rs > 0)
			{
				data.append(buf, rs);
			}
			memset(buf, 0, sizeof(buf));
		}
		return data;
	}
	bool File::Eof() const
	{
		return ((!file_) || (feof(file_)));
	}
	bool File::CopyTo(const std::string& real_path)
	{
		if (!Open("rb"))
		{
			return false;
		}
		File dist(real_path);
		if (!dist.Open("wb"))
		{
			return false;
		}
		while (!Eof())
		{
			char buf[1024] = { 0 };
			size_t bs = Read(buf, sizeof(buf));
			if (bs == 0)
			{
				break;
			}
			if (dist.Write(buf, bs) == 0)
			{
				break;
			}
		}
		return true;
	}
	bool File::CopyToDir(const std::string& dir, const std::string& name)
	{
		Close();
		if (!Exists(real_path_) || IsDir(real_path_) || (!IsDir(dir)))
		{
			return false;
		}

		std::string distpath = dir;
		AppendFileSlash(distpath);
		if (name.empty())
		{
			std::string::size_type pos = real_path_.rfind(TINY_FILE_SLASH);
			if (pos == std::string::npos)
			{
				distpath.append(real_path_);
			}
			else
			{
				distpath.append(real_path_.substr(pos + 1));
			}
		}
		else
		{
			distpath.append(name);
		}

		return CopyTo(distpath);
	}
	int File::Flush()
	{
		if (!file_)
		{
			return -1;
		}
		return fflush(file_);
	}
	utime_t File::GetCreateTime() const
	{
		if (!file_ && real_path_.empty())
		{
			return utime_t();
		}

		struct stat st;
		if (!file_)
		{
			::stat(real_path_.c_str(), &st);
		}
		else
		{
#ifdef UNI_WIN
			::fstat(_fileno(file_), &st);
#else
			::fstat(fileno(file_), &st);
#endif // UNI_WIN
		}
		return utime_t(st.st_ctime);
	}
	void File::RemoveFileSlash(std::string& path)
	{
		while (!path.empty())
		{
			if (path.back() != TINY_FILE_SLASH)
			{
				break;
			}
			path.pop_back();
		}
	}
	void File::AppendFileSlash(std::string& path)
	{
		if (path.empty() || (path.back() != TINY_FILE_SLASH))
		{
			path.append(TINY_FILE_SLASH_STR);
		}
	}
	bool File::Exists(const std::string& path)
	{
#ifdef UNI_WIN
		return (_access(path.c_str(), 0) == 0);
#else
		return (access(path.c_str(), F_OK) == 0);
#endif // UNI_WIN
	}
	bool File::IsDir(const std::string& path)
	{
		struct stat st;
		::stat(path.c_str(), &st);
		return (st.st_mode & S_IFDIR);
	}
	bool File::Delete(const std::string& path)
	{
		if (IsDir(path))
		{
			return false;
		}
#ifdef UNI_WIN
		return (_unlink(path.c_str()) == 0);
#else
		return (unlink(path.c_str()) == 0);
#endif // UNI_WIN
	}
	std::string File::RealPath(const std::string& path)
	{
		if (path.empty())
		{
			return std::string();
		}
		char* ptrRet = nullptr;
#ifdef UNI_WIN
		char rpath[_MAX_PATH + 1] = { 0 };
		ptrRet = _fullpath(rpath, path.c_str(), sizeof(rpath));
#else
		char rpath[PATH_MAX + 1] = { 0 };
		ptrRet = realpath(path.c_str(), rpath);
#endif // UNI_WIN
		if (!ptrRet)
		{
			return path;
		}
		return std::string(rpath);
	}
	std::string File::Current()
	{
		char* ptrRet = nullptr;
#ifdef UNI_WIN
		char path[_MAX_PATH + 1] = { 0 };
		ptrRet = _getcwd(path, sizeof(path));
#else
		char path[PATH_MAX + 1] = { 0 };
		ptrRet = getcwd(path, sizeof(path));
#endif // UNI_WIN
		if (!ptrRet)
		{
			return std::string(".");
		}
		return std::string(path);
	}
	std::string File::WorkDir()
	{
		return Current();
	}
	std::string File::AppName()
	{
		char name[500] = { 0 };
		int ns = 0;
#ifdef UNI_WIN
		ns = GetModuleFileName(nullptr, name, sizeof(name));
#else
		ns = readlink("/proc/self/exe", name, sizeof(name));
#endif // UNI_WIN
		if (ns <= 0)
		{
			return std::string();
		}
		std::string sn(name, static_cast<size_t>(ns));
		std::string::size_type pos = sn.rfind(TINY_FILE_SLASH);
		if (pos != std::string::npos)
		{
			sn.erase(0, pos + 1);
		}
		return sn;
	}
	bool File::CreateDir(const std::string& path)
	{
		if (path.empty())
		{
			return false;
		}
		if (!Exists(path))
		{
#ifndef UNI_WIN
			return (mkdir(path.c_str(), 0775) == 0);
#else
			return (_mkdir(path.c_str()) == 0);
#endif // !UNI_WIN
		}
		return IsDir(path);
	}
}