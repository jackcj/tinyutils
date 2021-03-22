#ifndef TINY_SQL_HELPER_H
#define	TINY_SQL_HELPER_H

#include "tiny_string.h"

#include "tiny_assert.h"
#include <functional>
#include <memory>
namespace tiny
{
	namespace db
	{
		/*
		*	class DataField
		*/
		class DataField
		{
		public:
			DataField() {}
			DataField(const char* str) { set(str); }
			DataField(const std::string& str) { set(str); }
			DataField(int i) { set(i); }
			DataField(unsigned i) { set(i); }
			DataField(long i) { set(i); }             // NOLINT
			DataField(long long i) { set(i); }      // NOLINT
			DataField(unsigned long i) { set(i); }    // NOLINT
			DataField(unsigned long long i) { set(i); }  // NOLINT
			DataField(float f) { set(f); }
			DataField(double f) { set(f); }
			DataField(long double f) { set(f); }
			DataField(const DataField&) = delete;
			DataField(DataField&& f) noexcept
			{
				std::swap(data_, f.data_);
			}
		public:
			DataField& operator= (const char* str) { set(str); return *this; }
			DataField& operator= (const std::string& str) { set(str); return *this; }
			DataField& operator= (int i) { set(i); return *this; }
			DataField& operator= (unsigned i) { set(i); return *this; }
			DataField& operator= (long i) { set(i); return *this; }					// NOLINT
			DataField& operator= (long long i) { set(i); return *this; }			// NOLINT
			DataField& operator= (unsigned long i) { set(i); return *this; }		// NOLINT
			DataField& operator= (unsigned long long i) { set(i); return *this; }	// NOLINT
			DataField& operator= (float f) { set(f); return *this; }
			DataField& operator= (double f) { set(f); return *this; }
			DataField& operator= (long double f) { set(f); return *this; }
			DataField& operator = (const DataField&) = delete;
			DataField& operator = (DataField&& f) noexcept { std::swap(data_, f.data_); return *this; }
		public:
			operator bool() const { return (!data_.empty()); }
			bool operator ! () const { return data_.empty(); }
			bool operator == (const DataField& f) const { return (data_ == f.data_); }
			bool operator != (const DataField& f) const { return (data_ != f.data_); }
		public:
			void set(const char* str);
			void set(const std::string& str);
			void set(int i);
			void set(unsigned i);
			void set(long i);                // NOLINT
			void set(long long i);           // NOLINT
			void set(unsigned long i);       // NOLINT
			void set(unsigned long long i);  // NOLINT
			void set(float f);
			void set(double f);
			void set(long double f);
			const char* c_str() const { return data_.c_str(); }
			size_t size() const { return data_.size(); }
			const std::string& data() const { return data_; }
		private:
			std::string data_;
		};

		/*
		*	class DBConnection
		*/

		class DataRecord
		{
		public:
			DataRecord() = default;
			DataRecord(const std::vector<tiny::stringview>& vals)
			{
				values.resize(vals.size());
				for (size_t i = 0; i < vals.size(); ++i)
				{
					values[i] = vals[i].to_string();
				}
			}
			DataRecord(DataRecord&& r) noexcept
			{
				std::swap(values, r.values);
			}
			DataRecord(const DataRecord&) = delete;
			~DataRecord() = default;
		public:
			DataRecord& operator = (const DataRecord&) = delete;
			DataRecord& operator = (DataRecord&& r) noexcept 
			{
				std::swap(values, r.values);
				return *this;
			}
			const std::string& operator [] (unsigned index) const
			{
				tiny_assert((index < values.size()));
				return values[index];
			}
			std::string& operator [] (unsigned index)
			{
				if (index < values.size())
				{
					return values[index];
				}
				values.resize(index + 1);
				return values[index];
			}
			bool operator ! () const { return values.empty(); }
			operator bool() const { return (values.size() > 0); }
		public:
			bool empty() const { return values.empty(); }
			int asInt(unsigned index, int defa = 0) const
			{
				if (index >= values.size())
				{
					return defa;
				}
				int v = 0;
				if (1 == sscanf(values[index].c_str(), "%d", &v))
				{
					return v;
				}
				return defa;
			}
			double asDouble(unsigned index, double defa = 0.0) const
			{
				if (index >= values.size())
				{
					return defa;
				}
				double v = 0;
				if (1 == sscanf(values[index].c_str(), "%lf", &v))
				{
					return v;
				}
				return defa;
			}
			const std::string& asString(unsigned index, const std::string& defa = std::string())
			{
				if (index >= values.size())
				{
					return defa;
				}
				return values[index];
			}
			size_t size() const { return values.size(); }
		private:
			std::vector<std::string> values;
			
		};

		class DataTable
		{
		public:
			DataTable() = default;
			DataTable(const DataTable&) = delete;
			DataTable(DataTable&& dt) noexcept
			{
				std::swap(field_names, dt.field_names);
				records = std::move(dt.records);
			}
			~DataTable() = default;
		public:
			DataTable& operator = (const DataTable&) = delete;
			DataTable& operator = (DataTable&& dt) noexcept
			{
				std::swap(field_names, dt.field_names);
				records = std::move(dt.records);
				return *this;
			}
			bool empty() const { return field_names.empty(); }
		public:
			void set_field_naes(const std::vector<tiny::stringview>& names)
			{
				field_names.resize(names.size());
				for (size_t i = 0; i < names.size(); ++i)
				{
					field_names[i] = names[i].to_string();
				}
			}
			void append_values(const std::vector<tiny::stringview>& values)
			{
				records.emplace_back(DataRecord(values));
			}

			const std::vector<std::string>& get_field_names() const { return field_names; }
			const std::vector<DataRecord>& get_records() const { return records; }
			bool has_fields() const { return (field_names.size() > 0); }
			void clear() { field_names.clear(); records.clear(); }
		private:
			std::vector<std::string> field_names;
			std::vector<DataRecord> records;
		};

		/*
		*	class DBConnectFactory
		*/
		using DatasetCallback = std::function<void(const std::vector<tiny::stringview>& names, const std::vector<tiny::stringview>& values)>;
		using ErrorCallback = std::function<void(int err_code, const std::string& err_msg)>;
		class ConnectFactory
		{
		public:
			class ConnectOption
			{
			public:
				ConnectOption();
				ConnectOption(const ConnectOption& cc);
				~ConnectOption() = default;
			public:
				ConnectOption& operator = (const ConnectOption& cc);
			public:
				int conn_id;
				unsigned short port;
				std::string host;
				std::string database;
				std::string username;
				std::string password;
				std::string charset;
			};
			class Connection
			{
			public:
				virtual ~Connection() {}
			public:
				virtual int Query(const std::string& sql
					, DatasetCallback&& field_cb
					, ErrorCallback& err_cb) = 0;
				virtual int Exec(const std::string& sql
					, ErrorCallback& err_cb) = 0;
			};
			using ConnectionRef = std::shared_ptr<Connection>;
		public:
			virtual ~ConnectFactory() {}
		public:
			virtual ConnectionRef CreateConnection(const ConnectOption& co, ErrorCallback&& err_cb) = 0;
		};
		using ScopedConnectFactory = std::unique_ptr<ConnectFactory>;


		class DriverFactoryInterface
		{
		public:
			virtual ~DriverFactoryInterface() {}
		public:
			virtual ScopedConnectFactory CreateConnectFactory(const std::string& name) = 0;
			virtual std::vector<std::string> Supports() = 0;
		};

		class TinyDriverFactory : public DriverFactoryInterface
		{
		public:
			TinyDriverFactory(){}
			~TinyDriverFactory() override{}
		public:
			ScopedConnectFactory CreateConnectFactory(const std::string& name) override;
			std::vector<std::string> Supports() override;
		};

		/*
		*	class SqlHelper
		*/

		class SqlHelper
		{
		protected:
			class Cond
			{
			public:
				Cond(SqlHelper& sr, const std::string& fn, const DataField& fv, const std::string& alg = "=");
				Cond(const Cond& s);
				Cond& operator = (const Cond& s);

			public:
				Cond& And(const std::string& fn, const DataField& fv, const std::string& alg = "=");
				Cond& Or(const std::string& fn, const DataField& fv, const std::string& alg = "=");

				int Final(ConnectFactory::ConnectionRef db_conn);
				int Final(ConnectFactory::ConnectionRef db_conn, DatasetCallback&& dataset_cb);
			private:
				SqlHelper& helper;
			};
			class Selector
			{
			public:
				Selector(SqlHelper& sr, const std::string& data = "*");
				Selector(const Selector& s);
				Selector& operator = (const Selector& s);
				Cond Where(const std::string& fn, const DataField& fv, const std::string& alg = "=");
				int Final(ConnectFactory::ConnectionRef db_conn, DatasetCallback&& dataset_cb);
			private:
				SqlHelper& helper;
			};
			class Updater
			{
			public:
				Updater(SqlHelper& sr, const std::string& fn, const DataField& fv);
				Updater(const Updater& s);
				Updater& operator = (const Updater& s);
			public:
				Updater& Update(const std::string& fn, const DataField& fv);
				Cond Where(const std::string& fn, const DataField& fv, const std::string& alg = "=");
				int Final(ConnectFactory::ConnectionRef db_conn);
			private:
				SqlHelper& helper;
			};
			class Deletor
			{
			public:
				Deletor(SqlHelper& sr);
				Deletor(const Deletor& s);
				Deletor& operator = (const Deletor& s);
			public:
				Cond Where(const std::string& fn, const DataField& fv, const std::string& alg = "=");
				int Final(ConnectFactory::ConnectionRef db_conn);
			private:
				SqlHelper& helper;
			};
			class Creator
			{
			public:
				Creator(SqlHelper& sr);
				Creator(const Creator& s);
				Creator& operator = (const Creator& s);
			public:
				Creator& Field(const std::string& fn, const std::string& ft);
				Creator& Field(const std::pair<std::string,std::string>& nt);
				int Final(ConnectFactory::ConnectionRef db_conn);
			private:
				SqlHelper& helper;
				std::string fields;
			};

			class InsertInto
			{
			public:
				InsertInto(SqlHelper& sr, bool replace = false);
				InsertInto(const InsertInto& s);
				InsertInto& operator = (const InsertInto& s);
			public:
				InsertInto& Value(const std::string& fn, const DataField& fv);
				int Final(ConnectFactory::ConnectionRef db_conn);
			private:
				SqlHelper& helper;
				std::string fns;
				std::string fvs;
			};
		public:
			SqlHelper(const std::string& table) : err_code(0), table_name(table){}
		public:
			SqlHelper::Selector Select(const std::string& n = std::string("*"))
			{
				return Selector(*this, n);
			}
			SqlHelper::Updater Update(const std::string& fn, const DataField& fv)
			{
				return Updater(*this, fn, fv);
			}

			SqlHelper::Deletor Delete()
			{
				return Deletor(*this);
			}
			SqlHelper::InsertInto Insert()
			{
				return InsertInto(*this);
			}
			SqlHelper::InsertInto Replace()
			{
				return InsertInto(*this,true);
			}
			SqlHelper::Creator Create()
			{
				return Creator(*this);
			}
		public:
			void reset(const std::string& table)
			{
				table_name = table;
				sql.clear();
				err_code = 0;
				err_msg.clear();
			}
			int Exec(ConnectFactory::ConnectionRef db_conn)
			{
				if (!db_conn)
				{
					return -1;
				}
				ErrorCallback ec = std::bind(&SqlHelper::error_callback, this, std::placeholders::_1, std::placeholders::_2);
				return db_conn->Exec(sql, ec);
			}
			int Exec(ConnectFactory::ConnectionRef db_conn,DatasetCallback&& field)
			{
				if (!db_conn)
				{
					std::cout << "engine is null" << std::endl;
					return -1;
				}
				ErrorCallback ec = std::bind(&SqlHelper::error_callback, this, std::placeholders::_1, std::placeholders::_2);
				return db_conn->Query(sql, std::move(field), ec);
			}
			const std::string& Sql() const { return sql; }
			void error_callback(int ec, const std::string& em)
			{
				err_code = ec;
				err_msg = em;
			}
		private:
			int err_code;
			std::string err_msg;
			std::string table_name;
			std::string sql;
		};
	}
}

using SqlHelper = tiny::db::SqlHelper;
using DBConnRef = tiny::db::ConnectFactory::ConnectionRef;
using DBConnOptiont = tiny::db::ConnectFactory::ConnectOption;
using DBTinyDriverFactory = tiny::db::TinyDriverFactory;
using DBScopedConnectFactory = tiny::db::ScopedConnectFactory;
#endif // !TINY_SQL_HELPER_H
