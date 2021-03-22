#include "tiny_parser.h"


#include "tinyxml2.h"
#include "tinyjson.h"
namespace tiny
{


	Xml::Node& Xml::Node::operator=(const char* val)
	{
		if (node_)
		{
			node_->SetText(val);
		}
		return *this;
	}
	Xml::Node& Xml::Node::operator=(const stringview& val)
	{
		if (node_)
		{
			node_->SetText(val.c_str());
		}
		return *this;
	}
	Xml::Node& Xml::Node::operator=(const std::string& val)
	{
		if (node_)
		{
			node_->SetText(val.c_str());
		}
		return *this;
	}
	Xml::Node& Xml::Node::operator=(int val)
	{
		if (node_)
		{
			node_->SetText(val);
		}
		return *this;
	}
	Xml::Node& Xml::Node::operator=(unsigned val)
	{
		if (node_)
		{
			node_->SetText(val);
		}
		return *this;
	}
	Xml::Node& Xml::Node::operator=(int64_t val)
	{
		if (node_)
		{
			node_->SetText(val);
		}
		return *this;
	}
	Xml::Node& Xml::Node::operator=(uint64_t val)
	{
		if (node_)
		{
			node_->SetText(val);
		}
		return *this;
	}
	Xml::Node& Xml::Node::operator=(bool val)
	{
		if (node_)
		{
			node_->SetText(val);
		}
		return *this;
	}
	Xml::Node& Xml::Node::operator=(double val)
	{
		if (node_)
		{
			node_->SetText(val);
		}
		return *this;
	}
	Xml::Node& Xml::Node::operator=(float val)
	{
		if (node_)
		{
			node_->SetText(val);
		}
		return *this;
	}
	Xml::Node& Xml::Node::operator=(const Xml::Node& n)
	{
		node_ = n.node_;
		return *this;
	}
	Xml::Node Xml::Node::operator[](const stringview& name)
	{
		if (!node_)
		{
			return Xml::Node();
		}
		tinyxml2::XMLElement* tmp = node_->FirstChildElement(name.c_str());
		if (!tmp)
		{
			tmp = node_->GetDocument()->NewElement(name.c_str());
			node_->InsertEndChild(tmp);
		}
		return Xml::Node(tmp);
	}
	Xml::Node Xml::Node::operator[](const char* name)
	{
		return operator[] (tiny::stringview(name));
	}
	const Xml::Node Xml::Node::operator[](const stringview& name) const
	{
		if (!node_)
		{
			return Xml::Node();
		}
		return Xml::Node(node_->FirstChildElement(name.c_str()));
	}
	const Xml::Node Xml::Node::operator[](const char* name) const
	{
		return operator[] (tiny::stringview(name));
	}
	Xml::Node Xml::Node::Next(const char* name)
	{
		if (!node_)
		{
			return Xml::Node();
		}
		tinyxml2::XMLElement* tmp = nullptr;
		if (!name)
		{
			tmp = node_->NextSiblingElement();
		}
		else
		{
			tmp = node_->NextSiblingElement(name);
		}
		return Xml::Node(tmp);
	}
	bool Xml::Node::Valid() const
	{
		return ((!node_) ? false : true);
	}
	stringview Xml::Node::Name() const
	{
		if (!node_)
		{
			return stringview();
		}
		return stringview(node_->Name());
	}
	stringview Xml::Node::Value(const char* defa) const
	{
		if (!node_ || !(node_->GetText()))
		{
			return stringview(defa);
		}
		return stringview(node_->GetText());
	}
	void Xml::Node::SetCDATA(const char* val)
	{
		if (!node_)
		{
			return;
		}
		node_->SetText(val);
		node_->FirstChild()->ToText()->SetCData(true);
	}
	int Xml::Node::asInt(int defaultValue) const
	{
		if (!node_)
		{
			return defaultValue;
		}
		return node_->IntText(defaultValue);
	}
	unsigned Xml::Node::asUnsigned(unsigned defaultValue) const
	{
		if (!node_)
		{
			return defaultValue;
		}
		return node_->UnsignedText(defaultValue);
	}
	int64_t Xml::Node::asInt64(int64_t defaultValue) const
	{
		if (!node_)
		{
			return defaultValue;
		}
		return node_->Int64Text(defaultValue);
	}
	uint64_t Xml::Node::asUnsigned64(uint64_t defaultValue) const
	{
		if (!node_)
		{
			return defaultValue;
		}
		return node_->Unsigned64Text(defaultValue);
	}
	bool Xml::Node::asBool(bool defaultValue) const
	{
		if (!node_)
		{
			return defaultValue;
		}
		return node_->BoolText(defaultValue);
	}
	double Xml::Node::asDouble(double defaultValue) const
	{
		if (!node_)
		{
			return defaultValue;
		}
		return node_->DoubleText(defaultValue);
	}
	float Xml::Node::asFloat(float defaultValue) const
	{
		if (!node_)
		{
			return defaultValue;
		}
		return node_->FloatText(defaultValue);
	}
	stringview Xml::Node::Attrib(const char* name)
	{
		if (!node_)
		{
			return stringview();
		}
		return stringview(node_->Attribute(name));
	}
	void Xml::Node::SetAttrib(const char* name, const char* val)
	{
		if (!node_ || !name || strlen(name) == 0)
		{
			return;
		}
		node_->SetAttribute(name, val);
	}
	void Xml::Node::SetAttrib(const char* name, int val)
	{
		if (!node_ || !name || strlen(name) == 0)
		{
			return;
		}
		node_->SetAttribute(name, val);
	}
	void Xml::Node::SetAttrib(const char* name, unsigned val)
	{
		if (!node_ || !name || strlen(name) == 0)
		{
			return;
		}
		node_->SetAttribute(name, val);
	}
	void Xml::Node::SetAttrib(const char* name, int64_t val)
	{
		if (!node_ || !name || strlen(name) == 0)
		{
			return;
		}
		node_->SetAttribute(name, val);
	}
	void Xml::Node::SetAttrib(const char* name, uint64_t val)
	{
		if (!node_ || !name || strlen(name) == 0)
		{
			return;
		}
		node_->SetAttribute(name, val);
	}
	void Xml::Node::SetAttrib(const char* name, bool val)
	{
		if (!node_ || !name || strlen(name) == 0)
		{
			return;
		}
		node_->SetAttribute(name, val);
	}
	void Xml::Node::SetAttrib(const char* name, double val)
	{
		if (!node_ || !name || strlen(name) == 0)
		{
			return;
		}
		node_->SetAttribute(name, val);
	}
	void Xml::Node::SetAttrib(const char* name, float val)
	{
		if (!node_ || !name || strlen(name) == 0)
		{
			return;
		}
		node_->SetAttribute(name, val);
	}

	Xml::Node Xml::operator[](const stringview& name)
	{
		if (!name)
		{
			return Xml::Node();
		}

		if (!doc_)
		{
			doc_ = new tinyxml2::XMLDocument;
			doc_->InsertEndChild(doc_->NewElement(name.c_str()));
		}
		return Xml::Node(doc_->FirstChildElement(name.c_str()));
	}
	const Xml::Node Xml::operator[](const stringview& name) const
	{
		if (!name || !doc_)
		{
			return Xml::Node();
		}
		return Xml::Node(doc_->FirstChildElement(name.c_str()));
	}
	Xml::Node Xml::operator[](const char* name)
	{
		if (!name)
		{
			return Xml::Node();
		}

		if (!doc_)
		{
			doc_ = new tinyxml2::XMLDocument;
			doc_->InsertEndChild(doc_->NewElement(name));
		}
		return Xml::Node(doc_->FirstChildElement(name));
	}
	const Xml::Node Xml::operator[](const char* name) const
	{
		if (!name || !doc_)
		{
			return Xml::Node();
		}
		return Xml::Node(doc_->FirstChildElement(name));
	}
	void Xml::Clear()
	{
		if (doc_)
		{
			delete doc_;
		}
		doc_ = nullptr;
	}
	bool Xml::Parse(const char* data, size_t len)
	{
		Clear();
		doc_ = new tinyxml2::XMLDocument;
		if (tinyxml2::XML_SUCCESS != doc_->Parse(data, len))
		{
			delete doc_;
			doc_ = nullptr;
			return false;
		}
		return true;
	}
	bool Xml::LoadFile(const char* path)
	{
		Clear();
		doc_ = new tinyxml2::XMLDocument;
		if (tinyxml2::XML_SUCCESS != doc_->LoadFile(path))
		{
			delete doc_;
			doc_ = nullptr;
			return false;
		}
		return true;
	}
	std::string Xml::ToString(bool styled) const
	{
		tinyxml2::XMLPrinter p(nullptr, !styled);
		doc_->Print(&p);
		if (!p.CStr())
		{
			return std::string();
		}
		return std::string(p.CStr(), p.CStrSize());
	}
	bool Xml::SaveAs(const stringview& path, bool styled)
	{
		if (!doc_ || !path)
		{
			return false;
		}

		if (tinyxml2::XML_SUCCESS != doc_->SaveFile(path.c_str(), !styled))
		{
			return false;
		}
		return true;
	}
	/*
		class Json::str
	*/
	void Json::str::clear()
	{
		if (ptr_)
		{
			tinyJSON_free(ptr_);
		}
		ptr_ = nullptr;
		len_ = 0;
	}

	/*
		class Json::Value
	*/
	Json::Value Json::Value::operator[](const char* name)
	{
		if (!name)
		{
			return Json::Value();
		}
		return Json::Value(CreateObject(), name);
	}
	const Json::Value Json::Value::operator[](const char* name) const
	{
		if (!name)
		{
			return Json::Value();
		}
		return Json::Value(CurrentObject(), name);
	}
	Json::Value Json::Value::operator[](const stringview& name)
	{
		return operator[](name.c_str());
	}
	const Json::Value Json::Value::operator[](const stringview& name) const
	{
		return operator[](name.c_str());
	}
	Json::Value Json::Value::operator[](int index)
	{
		if (index < 0)
		{
			return Json::Value();
		}
		return Json::Value(CreateArray(), index);
	}
	const Json::Value Json::Value::operator[](int index) const
	{
		if (index < 0)
		{
			return Json::Value();
		}
		return Json::Value(CurrentObject(), index);
	}
	Json::Value& Json::Value::operator=(const std::string& val)
	{
		return operator= (val.c_str());
	}
	Json::Value & Json::Value::operator=(const stringview & val)
	{
		return operator=(val.c_str());
	}
	Json::Value& Json::Value::operator=(const char* val)
	{
		if (!parent_ || !key_)
		{
			return *this;
		}
		if (key_.is_index())
		{
			tinyJSON_DeleteItemFromArray(parent_, key_.index());
			if (!val)
			{
				tinyJSON_InsertItemInArray(parent_, key_.index(), tinyJSON_CreateNull());
			}
			else if (strlen(val) == 0)
			{
				tinyJSON_InsertItemInArray(parent_, key_.index(), tinyJSON_CreateObject());
			}
			else
			{
				tinyJSON_InsertItemInArray(parent_, key_.index(), tinyJSON_CreateString(val));
			}
		}
		else
		{
			tinyJSON_DeleteItemFromObject(parent_, key_.name().c_str());
			if (!val)
			{
				tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tinyJSON_CreateNull());
			}
			else if (strlen(val) == 0)
			{
				tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tinyJSON_CreateObject());
			}
			else
			{
				tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tinyJSON_CreateString(val));
			}
		}
		return *this;
	}
	Json::Value& Json::Value::operator=(double num)
	{
		if (!parent_)
		{
			return *this;
		}
		if (key_.is_index())
		{
			tinyJSON_DeleteItemFromArray(parent_, key_.index());
			tinyJSON_InsertItemInArray(parent_, key_.index(), tinyJSON_CreateNumber(num));
		}
		else
		{
			tinyJSON_DeleteItemFromObject(parent_, key_.name().c_str());
			tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tinyJSON_CreateNumber(num));
		}
		return *this;
	}
	Json::Value& Json::Value::operator=(Json::BOOL b)
	{
		if (!parent_)
		{
			return *this;
		}

		int v = ((b == Json::BOOL::False) ? 0 : 1);
		if (key_.is_index())
		{
			tinyJSON_DeleteItemFromArray(parent_, key_.index());
			tinyJSON_InsertItemInArray(parent_, key_.index(), tinyJSON_CreateBool(v));
		}
		else
		{
			tinyJSON_DeleteItemFromObject(parent_, key_.name().c_str());
			tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tinyJSON_CreateBool(v));
		}
		return *this;
	}
	tinyJSON* Json::Value::CurrentObject() const
	{
		if (!parent_ || !key_)
		{
			return nullptr;
		}
		if (key_.is_index())
		{
			return tinyJSON_GetArrayItem(parent_, key_.index());
		}
		return tinyJSON_GetObjectItem(parent_, key_.name().c_str());
	}
	tinyJSON* Json::Value::CreateObject()
	{
		if (!parent_ || !key_)
		{
			return nullptr;
		}
		tinyJSON* tmp = nullptr;
		if (key_.is_index())
		{
			tmp = tinyJSON_GetArrayItem(parent_, key_.index());
			if (!tmp)
			{
				tmp = tinyJSON_CreateObject();
				tinyJSON_InsertItemInArray(parent_, key_.index(), tmp);
			}
			else
			{
				if (tmp->type != tinyJSON_Object)
				{
					tinyJSON_DeleteItemFromArray(parent_, key_.index());
					tmp = tinyJSON_CreateObject();
					tinyJSON_InsertItemInArray(parent_, key_.index(), tmp);
				}
			}
		}
		else
		{
			tmp = tinyJSON_GetObjectItem(parent_, key_.name().c_str());
			if (!tmp)
			{
				tmp = tinyJSON_CreateObject();
				tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tmp);
			}
			else
			{
				if (tmp->type != tinyJSON_Object)
				{
					tinyJSON_DeleteItemFromObject(parent_, key_.name().c_str());
					tmp = tinyJSON_CreateObject();
					tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tmp);
				}
			}
		}
		return tmp;
	}
	tinyJSON* Json::Value::CreateArray()
	{
		if (!parent_ || !key_)
		{
			return nullptr;
		}
		tinyJSON* tmp = nullptr;
		if (key_.is_index())
		{
			tmp = tinyJSON_GetArrayItem(parent_, key_.index());
			if (!tmp)
			{
				tmp = tinyJSON_CreateArray();
				tinyJSON_InsertItemInArray(parent_, key_.index(), tmp);
			}
			else
			{
				if (tmp->type != tinyJSON_Array)
				{
					tinyJSON_DeleteItemFromArray(parent_, key_.index());
					tmp = tinyJSON_CreateArray();
					tinyJSON_InsertItemInArray(parent_, key_.index(), tmp);
				}
			}
		}
		else
		{
			tmp = tinyJSON_GetObjectItem(parent_, key_.name().c_str());
			if (!tmp)
			{
				tmp = tinyJSON_CreateArray();
				tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tmp);
			}
			else
			{
				if (tmp->type != tinyJSON_Array)
				{
					tinyJSON_DeleteItemFromObject(parent_, key_.name().c_str());
					tmp = tinyJSON_CreateArray();
					tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tmp);
				}
			}
		}
		return tmp;
	}
	void Json::Value::finalWithNullObject()
	{
		if (!parent_ || !key_)
		{
			return;
		}
		if (key_.is_index())
		{
			tinyJSON_DeleteItemFromArray(parent_, key_.index());
			tinyJSON_InsertItemInArray(parent_, key_.index(), tinyJSON_CreateNull());
		}
		else
		{
			tinyJSON_DeleteItemFromObject(parent_, key_.name().c_str());
			tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tinyJSON_CreateNull());
		}
	}
	void Json::Value::finalWithEmptyObject()
	{
		if (!parent_ || !key_)
		{
			return;
		}
		if (key_.is_index())
		{
			tinyJSON_DeleteItemFromArray(parent_, key_.index());
			tinyJSON_InsertItemInArray(parent_, key_.index(), tinyJSON_CreateObject());
		}
		else
		{
			tinyJSON_DeleteItemFromObject(parent_, key_.name().c_str());
			tinyJSON_AddItemToObject(parent_, key_.name().c_str(), tinyJSON_CreateObject());
		}
	}
	stringview Json::Value::asString() const
	{
		tinyJSON* tmp = CurrentObject();
		if (!tmp)
		{
			return stringview();
		}
		return stringview(tmp->valuestring);
	}
	int Json::Value::asInt() const
	{
		tinyJSON* tmp = CurrentObject();
		if (!tmp)
		{
			return 0;
		}
		return tmp->valueint;
	}
	double Json::Value::asDouble() const
	{
		tinyJSON* tmp = CurrentObject();
		if (!tmp)
		{
			return 0;
		}
		return tmp->valuedouble;
	}
	bool Json::Value::asBool() const
	{
		tinyJSON* tmp = CurrentObject();
		if (!tmp)
		{
			return false;
		}
		return tinyJSON_IsTrue(tmp);
	}
	bool Json::Value::isArray() const
	{
		tinyJSON* tmp = CurrentObject();
		if (!tmp)
		{
			return false;
		}
		return (tinyJSON_IsArray(tmp) == tinyJSON_True);
	}
	int Json::Value::size() const
	{
		tinyJSON* tmp = CurrentObject();
		if (!tmp)
		{
			return 0;
		}
		return tinyJSON_GetArraySize(tmp);
	}
	stringview Json::Value::name() const
	{
		tinyJSON* tmp = CurrentObject();
		if (!tmp)
		{
			return stringview();
		}
		return stringview(tmp->string);
	}
	Json::str Json::Value::toString(bool use_style) const
	{
		if (!parent_)
		{
			return Json::str();
		}
		if (!use_style)
		{
			return Json::str(tinyJSON_PrintUnformatted(parent_));
		}
		return Json::str(tinyJSON_Print(parent_));
	}

	/*
		Json
	*/
	Json::Value Json::operator[](const char* name)
	{
		if (!name || strlen(name) == 0)
		{
			return Json::Value();
		}
		if (!root_)
		{
			root_ = tinyJSON_CreateObject();
		}
		else
		{
			if (root_->type != tinyJSON_Object)
			{
				Clear();
				root_ = tinyJSON_CreateObject();
			}
		}
		return Json::Value(root_, name);
	}
	Json::Value Json::operator[](int index)
	{
		if (index < 0)
		{
			return Json::Value();
		}
		if (!root_)
		{
			root_ = tinyJSON_CreateArray();
		}
		else
		{
			if (root_->type != tinyJSON_Array)
			{
				Clear();
				root_ = tinyJSON_CreateArray();
			}
		}
		return Json::Value(root_, index);
	}
	const Json::Value Json::operator[](const char* name) const
	{
		if (!root_ || !name)
		{
			return Json::Value();
		}
		if (root_->type != tinyJSON_Object || (tinyJSON_GetObjectItem(root_, name) == nullptr))
		{
			return Json::Value();
		}
		return Json::Value(root_, name);
	}
	const Json::Value Json::operator[](int index) const
	{
		if (!root_ || index < 0)
		{
			return Json::Value();
		}
		if (root_->type != tinyJSON_Array || tinyJSON_GetArraySize(root_) <= index)
		{
			return Json::Value();
		}
		return Json::Value(root_, index);
	}
	void Json::Clear()
	{
		if (root_)
		{
			tinyJSON_Delete(root_);
		}
		root_ = nullptr;
	}
	bool Json::Parse(const char* data)
	{
		Clear();
		if (!data)
		{
			return false;
		}
		root_ = tinyJSON_Parse(data);
		if (!root_)
		{
			return false;
		}
		return true;
	}
	bool Json::LoadFile(const char* path)
	{
		if (!path || strlen(path) == 0)
		{
			return false;
		}
		FILE* fp = fopen(path, "rb");
		if (!fp)
		{
			return false;
		}
		bool ret = LoadFile(fp);
		fclose(fp);
		return ret;
	}
	bool Json::LoadFile(FILE* fp)
	{
		Clear();
		if (!fp)
		{
			return false;
		}
		fseek(fp, 0, SEEK_SET);
		if (fgetc(fp) == EOF && ferror(fp) != 0) {
			return false;
		}

		fseek(fp, 0, SEEK_END);
		const long filelength = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (filelength <= 0) {
			return false;
		}

		const size_t size = filelength;
		std::vector<char> data;
		data.resize(size);

		const size_t read = fread(data.data(), 1, size, fp);
		if (read != size) {
			return false;
		}
		return Parse(data.data());
	}
	Json::str Json::ToString(bool use_style) const
	{
		std::string ret;
		if (!root_)
		{
			return Json::str();
		}
		if (!use_style)
		{
			return tinyJSON_PrintUnformatted(root_);
		}
		return tinyJSON_Print(root_);
	}
	int Json::Size()
	{
		return tinyJSON_GetArraySize(root_);
	}


	/*
		class IniFile
	*/

	void IniFile::TrimString(std::string& str)
	{
		while (!str.empty())
		{
			if (std::isspace(str.front()))
			{
				str.erase(0, 1);
			}
			else
			{
				break;
			}
		}
		while (!str.empty())
		{
			if (std::isspace(str.back()))
			{
				str.pop_back();
			}
			else
			{
				break;
			}
		}
	}
	void IniFile::SetFile(const std::string& path)
	{
		if (path.empty())
		{
			return;
		}

		FILE* file = fopen(path.c_str(), "r");
		if (!file)
		{
			return;
		}

		Record* record = nullptr;
		while (!feof(file))
		{
			char buff[4096] = { 0 };
			if (!fgets(buff, sizeof(buff), file))
			{
				break;
			}
			std::string line(buff);
			TrimString(line);
			if (line.empty())
			{
				continue;
			}
			if ((line.size() > 2) && line.front() == '[' && line.back() == ']')
			{
				line.erase(0, 1);
				line.pop_back();
				record = &dicts_[line];
				continue;
			}
			if (!record || (line.front() == '#') || (line.front() == '='))
			{
				continue;
			}
			std::string::size_type pos = line.find_first_of('=');
			if ((pos == std::string::npos) || (pos == 0))
			{
				continue;
			}
			(*record)[line.substr(0, pos)] = line.substr(pos + 1);
		}
		fclose(file);
	}
	const std::string& IniFile::GetValue(const std::string& section, const std::string& key, const std::string& val) const
	{
		Dict::const_iterator s_iter = dicts_.find(section);
		if (s_iter == dicts_.end())
		{
			return val;
		}
		const Record& record = s_iter->second;
		Record::const_iterator r_iter = record.find(key);
		if (r_iter == record.end())
		{
			return val;
		}
		return r_iter->second;
	}
	int IniFile::GetInt(const std::string& section, const std::string& key, int defa) const
	{
		std::string v = GetValue(section, key);
		if (v.empty())
		{
			return defa;
		}
		int iv = defa;
		if (1 == std::sscanf(v.c_str(), "%d", &iv))
		{
			return iv;
		}
		return defa;
	}
	double IniFile::GetDouble(const std::string& section, const std::string& key, double defa) const
	{
		std::string v = GetValue(section, key);
		if (v.empty())
		{
			return defa;
		}
		double iv = defa;
		if (1 == std::sscanf(v.c_str(), "%lf", &iv))
		{
			return iv;
		}
		return defa;
	}
	bool IniFile::GetBool(const std::string& section, const std::string& key, bool defa) const
	{
		std::string v = GetValue(section, key, "false");
		if ((v == "True")
			|| (v == "true")
			|| (v == "TRUE")
			|| (v == "yes")
			|| (v == "1"))
		{
			return true;
		}
		else if ((v == "False")
			|| (v == "false")
			|| (v == "FALSE")
			|| (v == "no")
			|| (v == "0"))
		{
			return false;
		}
		return defa;
	}
}