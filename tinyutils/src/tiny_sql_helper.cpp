#include "tiny_sql_helper.h"
#include "tiny_sqlite3_helper.h"
namespace tiny
{
	namespace db
	{
		void DataField::set(const char* str)
		{
			data_.clear();
			char* ptr = sqlite3_mprintf("%Q", str);
			if (!ptr)
			{
				return;
			}
			data_ = std::string(ptr);
			sqlite3_free(ptr);
		}
		void DataField::set(const std::string& str)
		{
			set(str.c_str());
		}
		void DataField::set(int i)
		{
			data_.clear();
			char* ptr = sqlite3_mprintf("%d", i);
			if (!ptr)
			{
				return;
			}
			data_ = std::string(ptr);
			sqlite3_free(ptr);
		}
		void DataField::set(unsigned i)
		{
			data_.clear();
			char* ptr = sqlite3_mprintf("%u", i);
			if (!ptr)
			{
				return;
			}
			data_ = std::string(ptr);
			sqlite3_free(ptr);
		}
		void DataField::set(long i)
		{
			data_.clear();
			char* ptr = sqlite3_mprintf("%ld", i);
			if (!ptr)
			{
				return;
			}
			data_ = std::string(ptr);
			sqlite3_free(ptr);
		}
		void DataField::set(long long i)
		{
			data_.clear();
			char* ptr = sqlite3_mprintf("%lld", i);
			if (!ptr)
			{
				return;
			}
			data_ = std::string(ptr);
			sqlite3_free(ptr);
		}
		void DataField::set(unsigned long i)
		{
			data_.clear();
			char* ptr = sqlite3_mprintf("%lu", i);
			if (!ptr)
			{
				return;
			}
			data_ = std::string(ptr);
			sqlite3_free(ptr);
		}
		void DataField::set(unsigned long long i)
		{
			data_.clear();
			char* ptr = sqlite3_mprintf("%llu", i);
			if (!ptr)
			{
				return;
			}
			data_ = std::string(ptr);
			sqlite3_free(ptr);
		}
		void DataField::set(float f)
		{
			data_.clear();
			char* ptr = sqlite3_mprintf("%g", f);
			if (!ptr)
			{
				return;
			}
			data_ = std::string(ptr);
			sqlite3_free(ptr);
		}
		void DataField::set(double f)
		{
			data_.clear();
			char* ptr = sqlite3_mprintf("%g", f);
			if (!ptr)
			{
				return;
			}
			data_ = std::string(ptr);
			sqlite3_free(ptr);
		}
		void DataField::set(long double f)
		{
			data_.clear();
			char* ptr = sqlite3_mprintf("%Lg", f);
			if (!ptr)
			{
				return;
			}
			data_ = std::string(ptr);
			sqlite3_free(ptr);
		}


		/*
		*		class DBConnectFactory::ConnectOption
		*/
		ConnectFactory::ConnectOption::ConnectOption()
			: conn_id(0)
			, port(0)
		{
		}
		ConnectFactory::ConnectOption::ConnectOption(const ConnectFactory::ConnectOption& cc)
			: conn_id(0)
			, port(0)
		{
			conn_id = cc.conn_id;
			port = cc.port;
			host = cc.host;
			database = cc.database;
			username = cc.username;
			password = cc.password;
			charset = cc.charset;
		}
		ConnectFactory::ConnectOption& ConnectFactory::ConnectOption::operator=(const ConnectFactory::ConnectOption& cc)
		{
			conn_id = cc.conn_id;
			port = cc.port;
			host = cc.host;
			database = cc.database;
			username = cc.username;
			password = cc.password;
			charset = cc.charset;
			return *this;
		}

		/*
		*	class DriverFactory
		*/
		ScopedConnectFactory TinyDriverFactory::CreateConnectFactory(const std::string& name)
		{
			return ScopedConnectFactory(new Sqlite3ConnectFactory);
		}
		std::vector<std::string> TinyDriverFactory::Supports()
		{
			std::vector<std::string> l;
			l.push_back("sqlite3");
			return l;
		}


		/*
		*	class SqlHelper
		*/
		SqlHelper::Cond::Cond(SqlHelper& sr, const std::string& fn, const DataField& fv, const std::string& alg)
			: helper(sr)
		{
			helper.sql.append(" WHERE ");
			helper.sql.append(fn).append(alg).append(fv.c_str(), fv.size());
		}
		SqlHelper::Cond::Cond(const Cond& s)
			: helper(s.helper) {}
		SqlHelper::Cond& SqlHelper::Cond::operator=(const Cond& s)
		{
			helper = s.helper;
			return *this;
		}
		SqlHelper::Cond& SqlHelper::Cond::And(const std::string& fn, const DataField& fv, const std::string& alg)
		{
			helper.sql.append(" AND ");
			helper.sql.append(fn).append(alg).append(fv.c_str(), fv.size());
			return *this;
		}
		SqlHelper::Cond& SqlHelper::Cond::Or(const std::string& fn, const DataField& fv, const std::string& alg)
		{
			helper.sql.append(" OR ");
			helper.sql.append(fn).append(alg).append(fv.c_str(), fv.size());
			return *this;
		}
		int SqlHelper::Cond::Final(ConnectFactory::ConnectionRef db_conn)
		{
			helper.sql.append(";");
			return helper.Exec(db_conn);
		}
		int SqlHelper::Cond::Final(ConnectFactory::ConnectionRef db_conn, DatasetCallback&& dataset_cb)
		{
			helper.sql.append(";");
			return helper.Exec(db_conn, std::move(dataset_cb));
		}
		SqlHelper::Selector::Selector(SqlHelper& sr, const std::string& data)
			: helper(sr)
		{
			helper.sql = "SELECT ";
			helper.sql.append(data).append(" FROM ").append(helper.table_name);
		}
		SqlHelper::Selector::Selector(const Selector& s)
			: helper(s.helper) {}
		SqlHelper::Selector& SqlHelper::Selector::operator=(const Selector& s)
		{
			helper = s.helper;
			return *this;
		}
		SqlHelper::Cond SqlHelper::Selector::Where(const std::string& fn, const DataField& fv, const std::string& alg)
		{
			return Cond(helper, fn, fv, alg);
		}
		int SqlHelper::Selector::Final(ConnectFactory::ConnectionRef db_conn, DatasetCallback&& dataset_cb)
		{
			helper.sql.append(";");
			return helper.Exec(db_conn, std::move(dataset_cb));
		}
		SqlHelper::Updater::Updater(SqlHelper& sr, const std::string& fn, const DataField& fv)
			: helper(sr)
		{
			helper.sql = "UPDATE ";
			helper.sql.append(helper.table_name).append(" SET ");
			helper.sql.append(fn).append("=").append(fv.c_str(), fv.size());
		}
		SqlHelper::Updater::Updater(const Updater& s)
			: helper(s.helper)
		{

		}
		SqlHelper::Updater& SqlHelper::Updater::operator=(const Updater& s)
		{
			helper = s.helper;
			return *this;
		}
		SqlHelper::Updater& SqlHelper::Updater::Update(const std::string& fn, const DataField& fv)
		{
			helper.sql.append(", ");
			helper.sql.append(fn).append("=").append(fv.c_str(), fv.size());
			return *this;
		}
		SqlHelper::Cond SqlHelper::Updater::Where(const std::string& fn, const DataField& fv, const std::string& alg)
		{
			return Cond(helper, fn, fv, alg);
		}
		int SqlHelper::Updater::Final(ConnectFactory::ConnectionRef db_conn)
		{
			helper.sql.append(";");
			return helper.Exec(db_conn);
		}
		SqlHelper::Deletor::Deletor(SqlHelper& sr)
			: helper(sr)
		{
			helper.sql = "DELETE";
			helper.sql.append(" FROM ").append(helper.table_name);
		}
		SqlHelper::Deletor::Deletor(const Deletor& s)
			: helper(s.helper)
		{

		}
		SqlHelper::Deletor& SqlHelper::Deletor::operator=(const Deletor& s)
		{
			helper = s.helper;
			return *this;
		}
		SqlHelper::Cond SqlHelper::Deletor::Where(const std::string& fn, const DataField& fv, const std::string& alg)
		{
			return Cond(helper, fn, fv, alg);
		}
		int SqlHelper::Deletor::Final(ConnectFactory::ConnectionRef db_conn)
		{
			helper.sql.append(";");
			return helper.Exec(db_conn);
		}
		SqlHelper::Creator::Creator(SqlHelper& sr)
			: helper(sr)
		{
			helper.sql = "CREATE TABLE IF NOT EXISTS ";
			helper.sql.append(helper.table_name);
		}
		SqlHelper::Creator::Creator(const Creator& s)
			: helper(s.helper) {}
		SqlHelper::Creator& SqlHelper::Creator::operator=(const Creator& s)
		{
			helper = s.helper;
			return *this;
		}
		SqlHelper::Creator& SqlHelper::Creator::Field(const std::string& fn, const std::string& ft)
		{
			if (!fields.empty())
			{
				fields.append(",");
			}
			fields.append(fn).append(" ").append(ft);
			return *this;
		}
		SqlHelper::Creator& SqlHelper::Creator::Field(const std::pair<std::string, std::string>& nt)
		{
			return Field(nt.first, nt.second);
		}
		int SqlHelper::Creator::Final(ConnectFactory::ConnectionRef db_conn)
		{
			if (fields.empty())
			{
				return -1;
			}
			helper.sql.append("(").append(fields).append(");");
			return helper.Exec(db_conn);
		}
		SqlHelper::InsertInto::InsertInto(SqlHelper& sr, bool replace)
			: helper(sr)
		{
			helper.sql = "INSERT INTO ";
			if (replace)
			{
				helper.sql = "REPLACE INTO ";
			}
			helper.sql.append(helper.table_name);
		}
		SqlHelper::InsertInto::InsertInto(const InsertInto& s)
			: helper(s.helper) {}
		SqlHelper::InsertInto& SqlHelper::InsertInto::operator=(const InsertInto& s)
		{
			helper = s.helper;
			return *this;
		}
		SqlHelper::InsertInto& SqlHelper::InsertInto::Value(const std::string& fn, const DataField& fv)
		{
			if (!fns.empty())
			{
				fns.append(",");
				fvs.append(",");
			}
			fns.append(fn);
			fvs.append(fv.c_str(), fv.size());
			return *this;
		}
		int SqlHelper::InsertInto::Final(ConnectFactory::ConnectionRef db_conn)
		{
			if (fns.empty())
			{
				return -1;
			}
			helper.sql.append("(").append(fns).append(") VALUES (").append(fvs).append(");");
			return helper.Exec(db_conn);
		}

	}
}