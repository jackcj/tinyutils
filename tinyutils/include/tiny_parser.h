#ifndef CJ_TINYPARSER_H
#define	CJ_TINYPARSER_H
#include "tiny_string.h"
#include <unordered_map>
struct tinyJSON;

namespace tinyxml2
{
	class XMLDocument;
	class XMLElement;
}

namespace tiny
{

	/*
		class Xml;
	*/
	class Xml
	{
	public:
		class Node
		{
		public:
			Node() :node_(nullptr) {}
			Node(tinyxml2::XMLElement* node) : node_(node) {}
			Node(const Node& n) : node_(n.node_) {}
		public:
			inline bool operator ! () const { return (!node_); }
			operator bool() const { return (node_ != nullptr); }
			Xml::Node& operator = (const char* val);
			Xml::Node& operator = (const stringview& val);
			Xml::Node& operator = (const std::string& val);
			Xml::Node& operator = (int val);
			Xml::Node& operator = (unsigned val);
			Xml::Node& operator = (int64_t val);
			Xml::Node& operator = (uint64_t val);
			Xml::Node& operator = (bool val);
			Xml::Node& operator = (double val);
			Xml::Node& operator = (float val);
			Xml::Node& operator = (const  Xml::Node& n);
			Xml::Node operator [] (const stringview& name);
			Xml::Node operator [] (const char* name);
			const Xml::Node operator [] (const stringview& name) const;
			const Xml::Node operator [] (const char* name) const;
		public:
			Xml::Node Next(const char* name = nullptr);
			bool Valid() const;
			stringview Name() const;
			stringview Value(const char* defa = nullptr) const;
			void SetCDATA(const char* val);

			int asInt(int defaultValue = 0) const;
			unsigned asUnsigned(unsigned defaultValue = 0) const;
			int64_t asInt64(int64_t defaultValue = 0) const;
			uint64_t asUnsigned64(uint64_t defaultValue = 0) const;
			bool asBool(bool defaultValue = false) const;
			double asDouble(double defaultValue = 0) const;
			float asFloat(float defaultValue = 0) const;
			stringview Attrib(const char* name);
			void SetAttrib(const char* name, const char* val);
			void SetAttrib(const char* name, int val);
			void SetAttrib(const char* name, unsigned val);
			void SetAttrib(const char* name, int64_t val);
			void SetAttrib(const char* name, uint64_t val);
			void SetAttrib(const char* name, bool val);
			void SetAttrib(const char* name, double val);
			void SetAttrib(const char* name, float val);
		private:
			tinyxml2::XMLElement* node_;
		};
	public:
		Xml() : doc_(nullptr) {}
		Xml(const tinyxml2::XMLDocument* doc) = delete;
		Xml(const Xml& x) = delete;
		Xml(Xml&& xml) noexcept : doc_(nullptr) { doc_ = xml.doc_; xml.doc_ = nullptr; }
		~Xml() { Clear(); }
	public:
		Xml& operator = (const tinyxml2::XMLDocument* doc) = delete;
		Xml& operator = (const Xml& x) = delete;
		Xml& operator = (Xml&& x) noexcept { Clear(); doc_ = x.doc_; x.doc_ = nullptr; return *this; }
		Xml::Node operator [] (const stringview& name);
		const Xml::Node operator [] (const stringview& name) const;
		Xml::Node operator [] (const char* name);
		const Xml::Node operator [] (const char* name) const;
		bool operator ! () const { return (!doc_); }
		operator bool() const { return (doc_ != nullptr); }
	public:
		void Clear();
		bool Parse(const char* data, size_t len);
		bool Parse(const std::string& data) { return Parse(data.c_str(), data.size()); }
		bool LoadFile(const char* path);
		std::string ToString(bool styled = false) const;
		bool SaveAs(const stringview& path, bool styled = false);
	private:
		tinyxml2::XMLDocument* doc_;
	};
	/*
		Json
	*/
	class Json
	{
	public:
		enum class BOOL
		{
			False = 0,
			Ture
		};

		class str
		{
		public:
			str()
				: ptr_(nullptr)
				, len_(0) {}
			str(char* ptr)
				: ptr_(ptr)
				, len_((!ptr) ? 0 : strlen(ptr))
			{
			}
			str(const str&) = delete;
			str(str&& s) noexcept
				: ptr_(nullptr)
				, len_(0)
			{
				std::swap(ptr_, s.ptr_);
				std::swap(len_, s.len_);
			}
			~str() { clear(); }
		public:
			bool operator ! () const { return (len_ == 0); }
			operator bool() const { return (len_ > 0); }
			str& operator = (const str&) = delete;
			str& operator = (str&& s) noexcept
			{
				std::swap(ptr_, s.ptr_);
				std::swap(len_, s.len_);
				return *this;
			}
		public:
			void clear();
			stringview data() const { return stringview(ptr_, len_); }
			size_t size() const { return len_; }
		private:
			char* ptr_;
			size_t len_;
		};

		class Key
		{
		public:
			Key(const char* n = nullptr)
				: name_(n)
				, index_(-1) {}
			Key(const stringview& n)
				: name_(n)
				, index_(-1) {}
			Key(int index)
				: name_(nullptr)
				, index_(index) {}
			Key(const Key& k)
			{
				name_ = k.name_;
				index_ = k.index_;
			}
		public:
			bool operator ! () const
			{
				if ((index_ == -1) && (!name_))
				{
					return true;
				}
				return false;
			}
			bool operator == (const Key& k) const
			{
				return ((index_ == k.index_) && (name_ == k.name_));
			}

			bool operator != (const Key& k) const
			{
				return ((index_ != k.index_) || (name_ != k.name_));
			}
			Key& operator = (const Key& k)
			{
				name_ = k.name_;
				index_ = k.index_;
				return *this;
			}
			Key& operator = (const char* n)
			{
				index_ = -1;
				name_ = n;
				return *this;
			}
			Key& operator = (int index)
			{
				name_.clear();
				index_ = index;
				return *this;
			}

			operator bool() const { return valid(); }
		public:
			bool valid() const { if (index_ != -1) return true; return (!name_.empty()); }
			void clear() { index_ = -1; name_.clear(); }
			int index() const { return index_; }
			const stringview& name() const { return name_; }
			bool is_index() const { return (index_ != -1); }
		private:
			stringview name_;
			int index_;
		};
		class Value
		{
		public:
			Value() : parent_(nullptr) {}
			Value(struct tinyJSON* parent, const stringview& name) : parent_(parent), key_(name) {}
			Value(struct tinyJSON* parent, int index) : parent_(parent), key_(index) {}
			Value(const Value& el) : parent_(el.parent_), key_(el.key_) {}
		public:
			bool operator ! () const { return (!parent_ || !key_); }
			Json::Value& operator = (const Json::Value& el) { parent_ = el.parent_; key_ = el.key_; return *this; }
			Json::Value operator [] (const char* name);
			const Json::Value operator [] (const char* name) const;
			Json::Value operator [] (const stringview& name);
			const Json::Value operator [] (const stringview& name) const;
			Json::Value operator [] (int index);
			const Json::Value operator [] (int index) const;
		public:
			Json::Value& operator = (const std::string& val);
			Json::Value& operator = (const stringview& val);
			Json::Value& operator = (const char* val);
			Json::Value& operator = (double num);
			Json::Value& operator = (int num);
			Json::Value& operator = (Json::BOOL b);
		protected:
			struct tinyJSON* CurrentObject() const;
			struct tinyJSON* CreateObject();
			struct tinyJSON* CreateArray();
		public:
			void finalWithNullObject();
			void finalWithEmptyObject();
			stringview asString() const;
			int asInt() const;
			double asDouble() const;
			bool asBool() const;
			bool isArray() const;
			int size() const;
			stringview name() const;
			Json::str toString(bool use_style = false) const;
		private:
			struct tinyJSON* parent_;
			Key key_;
		};
	public:
		Json() : root_(nullptr) {}
		Json(const struct tinyJSON* json) = delete;
		Json(const Json& json) = delete;
		Json(Json&& j) noexcept : root_(nullptr)
		{
			std::swap(root_, j.root_);
		}
		~Json() { Clear(); }
	public:
		Json& operator = (const Json& j) = delete;
		Json& operator = (const struct tinyJSON* j) = delete;
		Json& operator = (Json&& j) noexcept
		{
			std::swap(root_, j.root_);
			return *this;
		}
		bool operator ! () const { return (!root_); }
		Json::Value operator [] (const char* name);
		const Json::Value operator [] (const char* name) const;
		Json::Value operator [] (const stringview& name) { return operator[] (name.c_str()); }
		const Json::Value operator [] (const stringview& name) const { return operator[] (name.c_str()); }
		Json::Value operator [] (int index);
		const Json::Value operator [] (int index) const;
	public:
		void Clear();
		bool Parse(const char* data);
		bool Parse(const std::string& data) { return Parse(data.c_str()); }
		bool LoadFile(const char* path);
		bool LoadFile(FILE* fp);
		Json::str ToString(bool use_style = false) const;
		int Size();
	private:
		struct tinyJSON* root_;
	};

	/*
		class IniFile
	*/
	class IniFile
	{
	public:
		using Record = std::unordered_map<std::string, std::string>;
		using Dict = std::unordered_map<std::string, Record>;
	public:
		class Iterator
		{
		public:
			Iterator() {}
			Iterator(const Iterator& i)
			{
				begin_ = i.begin_;
				end_ = i.end_;
			}
			Iterator(Record::const_iterator b, Record::const_iterator e)
				: begin_(b)
				, end_(e)
			{

			}
		public:
			operator bool() const { return (begin_ != end_); }
			bool operator ! () const { return (begin_ == end_); }
			bool operator == (const Iterator& i) const { return (begin_ == i.begin_); }
			Iterator& operator = (const Iterator& i)
			{
				begin_ = i.begin_;
				end_ = i.end_;
				return *this;
			}
			Iterator& operator ++ ()
			{
				if (begin_ == end_)
				{
					return *this;
				}
				++begin_;
				return *this;
			}
		public:
			bool empty() const { return (begin_ == end_); }
			const std::string& First() const { return begin_->first; }
			const std::string& Second() const { return begin_->second; }
		private:
			Record::const_iterator begin_;
			Record::const_iterator end_;
		};

	public:
		IniFile() {}
		IniFile(const std::string& path) { SetFile(path); }
	public:
		void SetFile(const std::string& path);
		const std::string& GetValue(const std::string& section, const std::string& key, const std::string& val = std::string()) const;
		int GetInt(const std::string& section, const std::string& key, int defa = 0) const;
		double GetDouble(const std::string& section, const std::string& key, double defa = 0.0) const;
		bool GetBool(const std::string& section, const std::string& key, bool defa = false) const;
		Iterator Begin(const std::string& section) const
		{
			Dict::const_iterator iter = dicts_.find(section);
			if (iter == dicts_.end())
			{
				return Iterator();
			}
			return Iterator(iter->second.begin(), iter->second.end());
		}
	private:
		static void TrimString(std::string& str);
	private:
		Record null_records_;
		Dict dicts_;
	};


}

inline std::ostream& operator << (std::ostream& out, const tiny::Xml& x)
{
	return out << x.ToString(true);
}

inline std::ostream& operator << (std::ostream& out, const tiny::Xml::Node& n)
{
	return out << n.Value();
}


inline std::ostream& operator << (std::ostream& out, const tiny::Json& j)
{
	return out << j.ToString(true).data();
}

inline std::ostream& operator << (std::ostream& out, const tiny::Json::Value& j)
{
	return out << j.asString();
}
#endif // !CJ_TINYPARSER_H
