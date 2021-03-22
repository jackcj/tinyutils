#include "tiny_sql_helper.h"

#include "tiny_location.h"
#include "tiny_logger.h"
#include "tiny_sqlite3_helper.h"
#define ldout(v)		dout_impl(v, tiny_sqlite3)
#define lderr()				dout_impl(-1, tiny_sqlite3)
namespace tiny
{
	namespace db
	{
		class Sqlite3SelectFunc
		{
		public:
			Sqlite3SelectFunc(DatasetCallback&& f)
				: func(std::move(f)), rows(0){}
		public:
			void callback(const std::vector<tiny::stringview>& colNames, const std::vector<tiny::stringview>& colValues)
			{
				++rows;
				func(colNames, colValues);
			}
			unsigned get_rows() const { return rows; }
		private:
			DatasetCallback func;
			unsigned rows;
		};

		Sqlite3Connection::Sqlite3Connection()
			: handle(nullptr)
		{

		}
		Sqlite3Connection::~Sqlite3Connection()
		{
			if (handle)
			{
				sqlite3_close(handle);
			}
		}
		int Sqlite3Connection::open(const std::string& path, ErrorCallback& err_cb)
		{

			if (handle)
			{
				sqlite3_close(handle);
			}
			int errcode = 0;
			errcode = sqlite3_open_v2(path.c_str(), &handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
				SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE,
				NULL);
			if (SQLITE_OK != errcode)
			{
				lderr() << TINY_FROM_HERE << "sqlite3_open_v2 failed,code : " << errcode << "; msg: " << sqlite3_errstr(errcode) << dendl;
				if (err_cb)
				{
					err_cb(errcode, sqlite3_errstr(errcode));
				}
				return -1;
			}
			return 0;
		}
		int Sqlite3Connection::Query(const std::string& sql
			, DatasetCallback&& field_cb
			, ErrorCallback& err_cb)
		{
			if (!handle)
			{
				if (err_cb)
				{
					err_cb(-1, "sqlite3 unconnected!");
				}
				return -1;
			}
			char* err_msg = nullptr;
			int ret = 0;

			Sqlite3SelectFunc f(std::move(field_cb));
			ret = sqlite3_exec(handle, sql.c_str(), &sqlite3_exec_callback, (void*)&f, &err_msg);
			if (ret != 0)
			{
				lderr() << TINY_FROM_HERE << "sqlite3 query [ " << sql << " ];  ret=" << ret << "; error msg :" << tiny::stringview(err_msg) << dendl;
				if (err_cb)
				{
					err_cb(ret, err_msg);
				}
				return -1;
			}
			return f.get_rows();
		}
		int Sqlite3Connection::Exec(const std::string& sql, ErrorCallback& err_cb)
		{
			if (!handle)
			{
				if (err_cb)
				{
					err_cb(-1, "sqlite3 unconnected!");
				}
				return -1;
			}
			char* err_msg = nullptr;
			int ret = 0;
			ret = sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &err_msg);
			if (ret != 0)
			{
				lderr() << TINY_FROM_HERE << "sqlite3 query [ " << sql << " ];  ret=" << ret << "; error msg :" << tiny::stringview(err_msg) << dendl;
				if (err_cb)
				{
					err_cb(ret, err_msg);
				}
				return -1;
			}
			return 0;
		}
		int Sqlite3Connection::sqlite3_exec_callback(void* data, int nColumn, char** colValues, char** colNames)
		{
			if (!data)
			{
				return -1;
			}
			Sqlite3SelectFunc* sf =
				(Sqlite3SelectFunc*)data;
			std::vector<tiny::stringview> fs,vs;
			fs.resize(nColumn);
			vs.resize(nColumn);
			for (int i = 0; i < nColumn; ++i)
			{
				fs[i] = tiny::stringview(colNames[i]);
				vs[i] = tiny::stringview(colValues[i]);
			}
			sf->callback(fs, vs);
			return 0;
		}
		ConnectFactory::ConnectionRef Sqlite3ConnectFactory::CreateConnection(const ConnectOption& co, ErrorCallback&& err_cb)
		{
			if (co.database.empty())
			{
				return ConnectFactory::ConnectionRef();
			}
			std::shared_ptr<Sqlite3Connection> conn(new Sqlite3Connection);
			std::string path = co.host;
			path.append("/").append(co.database);

			ErrorCallback f = std::move(err_cb);
			if (conn->open(path, f) != 0)
			{
				return ConnectFactory::ConnectionRef();
			}
			return conn;
		}
	}
}