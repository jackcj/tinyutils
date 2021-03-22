#ifndef TINY_SQLITE3_HELPER_H
#define	TINY_SQLITE3_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "sqlite3.h"

#ifdef __cplusplus
}
#endif // __cplusplus
namespace tiny
{
	namespace db
	{
		class Sqlite3Connection : public ConnectFactory::Connection
		{
		public:
			Sqlite3Connection();
			~Sqlite3Connection() override;
		public:
			int open(const std::string& path, ErrorCallback& err_cb);
			int Query(const std::string& sql
				, DatasetCallback&& field_cb
				, ErrorCallback& err_cb) override;
			int Exec(const std::string& sql
				, ErrorCallback& err_cb) override;
		protected:
			static int sqlite3_exec_callback(void* data, int nColumn, char** colValues, char** colNames);
		private:
			sqlite3* handle;
		};

		/*
		*	class Sqlite3ConnectFactory
		*/
		class Sqlite3ConnectFactory : public ConnectFactory
		{
		public:
			~Sqlite3ConnectFactory() override {}
		public:
			ConnectFactory::ConnectionRef CreateConnection(const ConnectOption& co, ErrorCallback&& err_cb) override;
		};
	}
}
#endif // !TINY_SQLITE3_HELPER_H
