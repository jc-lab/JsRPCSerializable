#pragma once

#ifndef HAS_RAPIDJSON
#define HAS_RAPIDJSON 1
#endif
#ifndef HAS_JSCPPUTILS
#define HAS_JSCPPUTILS 1
#endif

#include "../Serializable.h"

#if defined(HAS_RAPIDJSON) && HAS_RAPIDJSON
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#endif

#include <string>

namespace JsRPC {

	class JSONObjectMapper
	{
	public:
		class TypeNotMatchException : public std::exception
		{ };
		class DataOverrflowException : public std::exception
		{ };

#if defined(HAS_RAPIDJSON) && HAS_RAPIDJSON
		static void serializeTo(Serializable *serialiable, rapidjson::Document &jsonDoc);
		static void deserializeJsonObject(Serializable *serialiable, const rapidjson::Value &jsonObject);

		template<class AllocatorT>
		static void serializeTo(Serializable *serialiable, rapidjson::Value &targetObject, AllocatorT &jsonAllocator)
		{
			rapidjson::Document jsonDoc;
			serializeTo(serialiable, jsonDoc);
			targetObject.CopyFrom(jsonDoc, jsonAllocator);
		}

		static std::string serialize(Serializable *serialiable)
		{
			rapidjson::Document jsonDoc;
			rapidjson::StringBuffer jsonBuf;
			rapidjson::Writer<rapidjson::StringBuffer> jsonWriter(jsonBuf);
			serializeTo(serialiable, jsonDoc);
			jsonDoc.Accept(jsonWriter);
			return std::string(jsonBuf.GetString(), jsonBuf.GetLength());
		}

		static void deserialize(Serializable *serialiable, const std::string &json)
		{
			rapidjson::Document jsonDoc;
			jsonDoc.Parse(json.c_str(), json.length());
			deserializeJsonObject(serialiable, jsonDoc);
		}
#endif
	};

}
