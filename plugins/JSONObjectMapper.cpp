/*
* Licensed to the Apache Software Foundation (ASF) under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.
* The ASF licenses this file to You under the Apache License, Version 2.0
* (the "License"); you may not use this file except in compliance with
* the License.  You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
 * @file	JSONObjectMapper.cppp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2018/12/06
 * @copyright Copyright (C) 2018 jichan.\n
 *             This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
 
#include "JSONObjectMapper.h"

#if defined(HAS_RAPIDJSON) && HAS_RAPIDJSON
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
#include <JsCPPUtils/StringBuffer.h>
#include <JsCPPUtils/StringEncoding.h>
#endif

namespace JsRPC {
	template<typename T>
	static T readFromPayload(const rapidjson::Value &jsonValue);
	template<>
	static bool readFromPayload<bool>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsBool())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetBool();
	}
	template<>
	static char readFromPayload<char>(const rapidjson::Value &jsonValue) {
		const char *text = jsonValue.GetString();
		return text[0];
	}
	template<>
	static wchar_t readFromPayload<wchar_t>(const rapidjson::Value &jsonValue) {
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
		std::basic_string<wchar_t> text = JsCPPUtils::StringEncoding::UTF8ToStringW(jsonValue.GetString(), jsonValue.GetStringLength());
		if (text.length() > 0)
		{
			return text.at(0);
		}
#endif
		return 0;
	}
	template<>
	static int8_t readFromPayload<int8_t>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsInt())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetInt();
	}
	template<>
	static uint8_t readFromPayload<uint8_t>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsUint())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetUint();
	}
	template<>
	static int16_t readFromPayload<int16_t>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsInt())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetInt();
	}
	template<>
	static uint16_t readFromPayload<uint16_t>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsUint())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetUint();
	}
	template<>
	static int32_t readFromPayload<int32_t>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsInt())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetInt();
	}
	template<>
	static uint32_t readFromPayload<uint32_t>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsUint())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetUint();
	}
	template<>
	static int64_t readFromPayload<int64_t>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsInt64())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetInt64();
	}
	template<>
	static uint64_t readFromPayload<uint64_t>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsUint64())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetUint64();
	}
	template<>
	static float readFromPayload<float>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsFloat())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetFloat();
	}
	template<>
	static double readFromPayload<double>(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsDouble())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetDouble();
	}

	// Native Element
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const bool *data) {
		jsonValue.SetBool(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const char *data) {
		jsonValue.SetString(data, 1, jsonAllocator);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const wchar_t *data) {
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
		JsCPPUtils::StringBuffer<char> utf8Text = JsCPPUtils::StringEncoding::StringToUTF8SB(data, 1);
		jsonValue.SetString(utf8Text.c_str(), utf8Text.length(), jsonAllocator);
#endif
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const int8_t *data) {
		jsonValue.SetInt(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const uint8_t *data) {
		jsonValue.SetUint(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const int16_t *data) {
		jsonValue.SetInt(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const uint16_t *data) {
		jsonValue.SetUint(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const int32_t *data) {
		jsonValue.SetInt(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const uint32_t *data) {
		jsonValue.SetUint(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const int64_t *data) {
		jsonValue.SetInt64(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const uint64_t *data) {
		jsonValue.SetUint64(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const float *data) {
		jsonValue.SetFloat(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const double *data) {
		jsonValue.SetDouble(*data);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const Serializable *data) {
		if (data) {
			rapidjson::Document jsonDoc;
			JSONObjectMapper::serializeTo(data, jsonDoc);
			jsonValue.CopyFrom(jsonDoc, jsonAllocator);
		} else {
			jsonValue.SetNull();
		}
	}

	template<typename T>
	static void readElementFromPayload(const rapidjson::Value &jsonValue, T *data) {
		*data = readFromPayload<T>(jsonValue);
	}
	template<>
	static void readElementFromPayload<Serializable>(const rapidjson::Value &jsonValue, Serializable *data) {
		JSONObjectMapper::deserializeJsonObject(data, jsonValue);
	}

	template <typename T, typename JsonAllocatorT>
	static void writeElementArrayToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const T* data, size_t length)
	{
		size_t i;
		jsonValue.SetArray();
		for (i = 0; i < length; i++) {
			rapidjson::Value jsonElement;
			writeElementToPayload(jsonElement, jsonAllocator, &data[i]);
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}

	template <typename T>
	static void readElementArrayFromPayload(const rapidjson::Value &jsonValue, T* data, size_t length)
	{
		size_t i = 0;
		if (!jsonValue.IsArray())
			throw JSONObjectMapper::TypeNotMatchException();
		if (jsonValue.Size() != length)
			throw JSONObjectMapper::TypeNotMatchException();
		for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
		{
			readElementFromPayload(*iter, &data[i++]);
		}
	}

	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const std::basic_string<char> *data) {
		jsonValue.SetString(data->c_str(), data->length(), jsonAllocator);
	}
	template <typename JsonAllocatorT>
	static void writeElementToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const std::basic_string<wchar_t> *data) {
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
		JsCPPUtils::StringBuffer<char> sbUtf8Text = JsCPPUtils::StringEncoding::StringToUTF8SB(*data);
		jsonValue.SetString(sbUtf8Text.c_str(), sbUtf8Text.length(), jsonAllocator);
#endif
	}
	static void readElementFromPayload(const rapidjson::Value &jsonValue, std::basic_string<char> *data)
	{
		if(!jsonValue.IsString())
			throw JSONObjectMapper::TypeNotMatchException();
		*data = std::basic_string<char>(jsonValue.GetString(), jsonValue.GetStringLength());
	}
	static void readElementFromPayload(const rapidjson::Value &jsonValue, std::basic_string<wchar_t> *data)
	{
		if (!jsonValue.IsString())
			throw JSONObjectMapper::TypeNotMatchException();
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
		*data = JsCPPUtils::StringEncoding::UTF8ToStringW(jsonValue.GetString(), jsonValue.GetStringLength());
#endif
	}
	template <typename T, typename JsonAllocatorT>
	static void writeStdVectorToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const std::vector<T> *data) {
		jsonValue.SetArray();
		for (std::vector<T>::const_iterator iter = data->begin(); iter != data->end(); iter++)
		{
			rapidjson::Value jsonElement;
			T value = *iter;
			writeElementToPayload(jsonElement, jsonAllocator, &value);
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}
	template <typename T>
	static void readStdVectorFromPayload(const rapidjson::Value &jsonValue, std::vector<T> *data) {
		if(!jsonValue.IsArray())
			throw JSONObjectMapper::TypeNotMatchException();
		data->clear();
		for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
		{
			T value;
			readElementFromPayload(*iter, &value);
			data->push_back(value);
		}
	}

	template <class T, typename JsonAllocatorT>
	static void writeStdListToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const std::list<T> *data);
	template <typename JsonAllocatorT>
	static void writeStdListToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const std::list< std::basic_string<char> > *data)
	{
		jsonValue.SetArray();
		for (std::list< std::basic_string<char> >::const_iterator iter = data->begin(); iter != data->end(); iter++)
		{
			rapidjson::Value jsonElement;
			jsonElement.SetString(iter->c_str(), iter->length(), jsonAllocator);
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}
	template <typename JsonAllocatorT>
	static void writeStdListToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const std::list< std::basic_string<wchar_t> > *data)
	{
		jsonValue.SetArray();
		for (std::list< std::basic_string<wchar_t> >::const_iterator iter = data->begin(); iter != data->end(); iter++)
		{
			rapidjson::Value jsonElement;
			JsCPPUtils::StringBuffer<char> sbUtf8Text = JsCPPUtils::StringEncoding::StringToUTF8SB(*iter);
			jsonElement.SetString(sbUtf8Text.c_str(), sbUtf8Text.length(), jsonAllocator);
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}
	template <typename T, typename JsonAllocatorT>
	static void writeStdListToPayload(rapidjson::Value &jsonValue, JsonAllocatorT &jsonAllocator, const std::list< std::vector<T> > *data)
	{
		jsonValue.SetArray();
		for (std::list< std::vector< T > >::const_iterator iter = data->begin(); iter != data->end(); iter++)
		{
			rapidjson::Value jsonElement;
			for (std::vector< T >::const_iterator subiter = iter->begin(); subiter != iter->end(); subiter++)
			{
				rapidjson::Value jsonSubElement;
				T value = *subiter;
				writeElementToPayload(jsonSubElement, jsonAllocator, &value);
				jsonElement.PushBack(jsonSubElement, jsonAllocator);
			}
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}

	template <class T>
	static void readStdListFromPayload(const rapidjson::Value &jsonValue, std::list<T> *data);
	template <typename T>
	static void readStdListFromPayload(const rapidjson::Value &jsonValue, std::list< std::basic_string<T> > *data)
	{
		data->clear();
		for(rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
		{
			std::basic_string<T> element;
			readElementFromPayload(*iter, &element);
			data->push_back(element);
		}
	}
	template <typename T>
	static void readStdListFromPayload(const rapidjson::Value &jsonValue, std::list< std::vector<T> > *data)
	{
		data->clear();
		for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
		{
			std::vector<T> element;
			readStdVectorFromPayload(*iter, &element);
			data->push_back(element);
		}
	}

	static void _serializeCheckNotEoo(uint16_t *tempEtype, std::list<internal::SerializableMemberInfo::EncapType>::const_iterator *iterEncap, std::list<internal::SerializableMemberInfo::EncapType>::const_iterator endOfEncap)
	{
		if ((*iterEncap) == endOfEncap)
			throw Serializable::Serializable::UnavailableTypeException();
		*tempEtype = *((*iterEncap)++);
	}

	static void _serializeCheckEoo(uint16_t *tempEtype, std::list<internal::SerializableMemberInfo::EncapType>::const_iterator *iterEncap, std::list<internal::SerializableMemberInfo::EncapType>::const_iterator endOfEncap)
	{
		*tempEtype = *((*iterEncap)++);
		if ((*iterEncap) != endOfEncap)
			throw Serializable::Serializable::UnavailableTypeException();
	}

	void JSONObjectMapper::serializeTo(const Serializable *serialiable, rapidjson::Document &jsonDoc)
	{
		rapidjson::Document::AllocatorType &jsonAllocator = jsonDoc.GetAllocator();

		const std::list<internal::STypeCommon*> &members = serialiable->serializableMembers();

		jsonDoc.SetObject();

		// Data
		for (std::list<internal::STypeCommon*>::const_iterator iterMem = members.begin(); iterMem != members.end(); iterMem++)
		{
			std::list<internal::SerializableMemberInfo::EncapType>::const_iterator iterEncap = (*iterMem)->_memberInfo.encaps.begin();
			std::list<internal::SerializableMemberInfo::EncapType>::const_iterator endOfEncap = (*iterMem)->_memberInfo.encaps.end();
			uint16_t tempEtype;
			rapidjson::Value jsonName;
			rapidjson::Value jsonValue;
			jsonName.SetString((*iterMem)->_memberInfo.name.c_str(), (*iterMem)->_memberInfo.name.length(), jsonAllocator);
			_serializeCheckNotEoo(&tempEtype, &iterEncap, endOfEncap);
			if ((*iterMem)->isNull())
			{
				jsonValue.SetNull();
			}
			else {
				if ((tempEtype & 0xFF00) == internal::SerializableMemberInfo::EncapType::ETYPE_NATIVE)
				{
					switch (tempEtype & 0x00FF)
					{
					case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
						writeElementToPayload(jsonValue, jsonAllocator, (const bool*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
						writeElementToPayload(jsonValue, jsonAllocator, (const int8_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
						writeElementToPayload(jsonValue, jsonAllocator, (const uint8_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
						writeElementToPayload(jsonValue, jsonAllocator, (const int16_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
						writeElementToPayload(jsonValue, jsonAllocator, (const uint16_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
						writeElementToPayload(jsonValue, jsonAllocator, (const int32_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
						writeElementToPayload(jsonValue, jsonAllocator, (const uint32_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
						writeElementToPayload(jsonValue, jsonAllocator, (const int64_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
						writeElementToPayload(jsonValue, jsonAllocator, (const uint64_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
						writeElementToPayload(jsonValue, jsonAllocator, (const char*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
						writeElementToPayload(jsonValue, jsonAllocator, (const wchar_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
						writeElementToPayload(jsonValue, jsonAllocator, (const float*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
						writeElementToPayload(jsonValue, jsonAllocator, (const double*)((*iterMem)->_memberInfo.ptr));
						break;
					default:
						throw Serializable::UnavailableTypeException();
					}
				}
				else {
					switch (tempEtype)
					{
					case internal::SerializableMemberInfo::EncapType::ETYPE_STDBASICSTRING:
						_serializeCheckNotEoo(&tempEtype, &iterEncap, endOfEncap);
						writeElementToPayload(jsonValue, jsonAllocator, &tempEtype);
						if (iterEncap != endOfEncap)
							throw Serializable::UnavailableTypeException();
						if (checkFlagsAll(tempEtype, internal::SerializableMemberInfo::EncapType::ETYPE_CHAR))
							writeElementToPayload(jsonValue, jsonAllocator, (const std::basic_string<char>*)(*iterMem)->_memberInfo.ptr);
						else if (checkFlagsAll(tempEtype, internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR))
							writeElementToPayload(jsonValue, jsonAllocator, (const std::basic_string<wchar_t>*)(*iterMem)->_memberInfo.ptr);
						else
							throw Serializable::UnavailableTypeException();
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_STDVECTOR:
						_serializeCheckNotEoo(&tempEtype, &iterEncap, endOfEncap);
						writeElementToPayload(jsonValue, jsonAllocator, &tempEtype);
						if (iterEncap != endOfEncap)
							throw Serializable::UnavailableTypeException();
						switch (tempEtype & 0x00FF)
						{
						case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<bool>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<int8_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<uint8_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<int16_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<uint16_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<int32_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<uint32_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<int64_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<uint64_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<char>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<wchar_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<float>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
							writeStdVectorToPayload(jsonValue, jsonAllocator, (std::vector<double>*)((*iterMem)->_memberInfo.ptr));
							break;
						default:
							throw Serializable::UnavailableTypeException();
						}
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_STDLIST:
						_serializeCheckNotEoo(&tempEtype, &iterEncap, endOfEncap);
						writeElementToPayload(jsonValue, jsonAllocator, &tempEtype);
						if (iterEncap == endOfEncap)
						{
							throw Serializable::UnavailableTypeException();
						}
						else {
							switch (tempEtype)
							{
							case internal::SerializableMemberInfo::EncapType::ETYPE_SMARTPOINTER:
								tempEtype = *(iterEncap++);
								writeElementToPayload(jsonValue, jsonAllocator, &tempEtype);
								switch (tempEtype)
								{
								case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
									if (iterEncap != endOfEncap)
										throw Serializable::UnavailableTypeException();
									for (std::list<JsCPPUtils::SmartPointer<Serializable> >::const_iterator subiter = ((std::list<JsCPPUtils::SmartPointer<Serializable> >*)(*iterMem)->_memberInfo.ptr)->begin(); subiter != ((std::list<JsCPPUtils::SmartPointer<Serializable> >*)(*iterMem)->_memberInfo.ptr)->end(); subiter++)
									{
										writeElementToPayload(jsonValue, jsonAllocator, subiter->getPtr());
									}
									break;
								default:
									throw Serializable::UnavailableTypeException();
								}
								break;
							case internal::SerializableMemberInfo::EncapType::ETYPE_STDVECTOR:
								_serializeCheckEoo(&tempEtype, &iterEncap, endOfEncap);
								writeElementToPayload(jsonValue, jsonAllocator, &tempEtype);
								switch (tempEtype & 0x00FF)
								{
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::vector<int8_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::vector<uint8_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::vector<int16_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::vector<uint16_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::vector<int32_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::vector<uint32_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::vector<int64_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::vector<uint64_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::vector<char> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::vector<wchar_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								default:
									throw Serializable::UnavailableTypeException();
								}
								break;
							case internal::SerializableMemberInfo::EncapType::ETYPE_STDBASICSTRING:
								_serializeCheckEoo(&tempEtype, &iterEncap, endOfEncap);
								writeElementToPayload(jsonValue, jsonAllocator, &tempEtype);
								if (checkFlagsAll(tempEtype, internal::SerializableMemberInfo::EncapType::ETYPE_CHAR))
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list< std::basic_string<char> > *)(*iterMem)->_memberInfo.ptr);
								else if (checkFlagsAll(tempEtype, internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR))
									writeStdListToPayload(jsonValue, jsonAllocator, (std::list<std::basic_string<wchar_t> >*)(*iterMem)->_memberInfo.ptr);
								else
									throw Serializable::UnavailableTypeException();
							default:
								throw Serializable::UnavailableTypeException();
							}
						}
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
						writeElementToPayload(jsonValue, jsonAllocator, (Serializable*)(*iterMem)->_memberInfo.ptr);
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_SMARTPOINTER:
						tempEtype = *(iterEncap++);
						switch (tempEtype)
						{
						case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
							if (iterEncap != endOfEncap)
								throw Serializable::UnavailableTypeException();
							if (!((JsCPPUtils::SmartPointer<Serializable>*)(*iterMem)->_memberInfo.ptr)->getPtr() || (*iterMem)->isNull())
							{
								tempEtype |= internal::SerializableMemberInfo::EncapType::ETYPE_NULL;
								writeElementToPayload(jsonValue, jsonAllocator, &tempEtype);
							}
							else {
								writeElementToPayload(jsonValue, jsonAllocator, &tempEtype);
								writeElementToPayload(jsonValue, jsonAllocator, ((JsCPPUtils::SmartPointer<Serializable>*)(*iterMem)->_memberInfo.ptr)->getPtr());
							}

							break;
						default:
							throw Serializable::UnavailableTypeException();
						}
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_NATIVEARRAY:
						switch (tempEtype & 0x00FF)
						{
						case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const bool*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const int8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const uint8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const int16_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const uint16_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const int32_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const uint32_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const int64_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const uint64_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const char*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const wchar_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const float*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
							writeElementArrayToPayload(jsonValue, jsonAllocator, (const double*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
						default:
							throw Serializable::UnavailableTypeException();
						}
						break;
					default:
						throw Serializable::UnavailableTypeException();
					}
				}
			}
			jsonDoc.AddMember(jsonName, jsonValue, jsonAllocator);
		}
	}

	void JSONObjectMapper::deserializeJsonObject(Serializable *serialiable, const rapidjson::Value &jsonObject) throw(JSONObjectMapper::TypeNotMatchException)
	{
		const std::list<internal::STypeCommon*> members = serialiable->serializableMembers();

		for(std::list<internal::STypeCommon*>::const_iterator iterMem = members.begin(); iterMem != members.end(); iterMem++)
		{
			std::list<internal::SerializableMemberInfo::EncapType>::const_iterator iterEncap = (*iterMem)->_memberInfo.encaps.begin();
			std::list<internal::SerializableMemberInfo::EncapType>::const_iterator endOfEncap = (*iterMem)->_memberInfo.encaps.end();
			uint16_t tempEtypeReal;
			const char *name = (*iterMem)->_memberInfo.name.c_str();
			_serializeCheckNotEoo(&tempEtypeReal, &iterEncap, endOfEncap);
			(*iterMem)->clear();
			if (jsonObject.HasMember(name))
			{
				const rapidjson::Value &jsonValue = jsonObject[name];
				if (jsonValue.IsNull()) {
					(*iterMem)->setNull();
				} else {
					if ((tempEtypeReal & 0xFF00) == internal::SerializableMemberInfo::EncapType::ETYPE_NATIVE)
					{
						switch (tempEtypeReal & 0x00FF)
						{
						case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
							readElementFromPayload<bool>(jsonValue, (bool*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
							readElementFromPayload<int8_t>(jsonValue, (int8_t*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
							readElementFromPayload<uint8_t>(jsonValue, (uint8_t*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
							readElementFromPayload<int16_t>(jsonValue, (int16_t*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
							readElementFromPayload<uint16_t>(jsonValue, (uint16_t*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
							readElementFromPayload<int32_t>(jsonValue, (int32_t*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
							readElementFromPayload<uint32_t>(jsonValue, (uint32_t*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
							readElementFromPayload<int64_t>(jsonValue, (int64_t*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
							readElementFromPayload<uint64_t>(jsonValue, (uint64_t*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
							readElementFromPayload<char>(jsonValue, (char*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
							readElementFromPayload<wchar_t>(jsonValue, (wchar_t*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
							readElementFromPayload<float>(jsonValue, (float*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
							readElementFromPayload<double>(jsonValue, (double*)((*iterMem)->_memberInfo.ptr));
							break;
						default:
							throw Serializable::UnavailableTypeException();
						}
					}
					else {
						switch (tempEtypeReal)
						{
						case internal::SerializableMemberInfo::EncapType::ETYPE_STDBASICSTRING:
							_serializeCheckEoo(&tempEtypeReal, &iterEncap, endOfEncap);
							if (iterEncap != endOfEncap)
								throw Serializable::UnavailableTypeException();
							if (checkFlagsAll(tempEtypeReal, internal::SerializableMemberInfo::EncapType::ETYPE_CHAR))
								readElementFromPayload(jsonValue, (std::basic_string<char>*)(*iterMem)->_memberInfo.ptr);
							else if (checkFlagsAll(tempEtypeReal, internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR))
								readElementFromPayload(jsonValue, (std::basic_string<wchar_t>*)(*iterMem)->_memberInfo.ptr);
							else
								throw Serializable::UnavailableTypeException();
							break;
						case internal::SerializableMemberInfo::EncapType::ETYPE_STDVECTOR:
							_serializeCheckNotEoo(&tempEtypeReal, &iterEncap, endOfEncap);
							if (iterEncap != endOfEncap)
								throw Serializable::UnavailableTypeException();
							if (!(tempEtypeReal & internal::SerializableMemberInfo::EncapType::ETYPE_NULL))
							{
								switch (tempEtypeReal & 0x00FF)
								{
								case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
									readStdVectorFromPayload(jsonValue, (std::vector<bool>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
									readStdVectorFromPayload(jsonValue, (std::vector<int8_t>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
									readStdVectorFromPayload(jsonValue, (std::vector<uint8_t>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
									readStdVectorFromPayload(jsonValue, (std::vector<int16_t>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
									readStdVectorFromPayload(jsonValue, (std::vector<uint16_t>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
									readStdVectorFromPayload(jsonValue, (std::vector<int32_t>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
									readStdVectorFromPayload(jsonValue, (std::vector<uint32_t>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
									readStdVectorFromPayload(jsonValue, (std::vector<int64_t>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
									readStdVectorFromPayload(jsonValue, (std::vector<uint64_t>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
									readStdVectorFromPayload(jsonValue, (std::vector<char>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
									readStdVectorFromPayload(jsonValue, (std::vector<wchar_t>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
									readStdVectorFromPayload(jsonValue, (std::vector<float>*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
									readStdVectorFromPayload(jsonValue, (std::vector<double>*)((*iterMem)->_memberInfo.ptr));
									break;
								default:
									throw Serializable::UnavailableTypeException();
								}
							}
							break;
						case internal::SerializableMemberInfo::EncapType::ETYPE_STDLIST:
							_serializeCheckNotEoo(&tempEtypeReal, &iterEncap, endOfEncap);
							if (iterEncap == endOfEncap)
							{
								throw Serializable::UnavailableTypeException();
							}
							else {
								switch (tempEtypeReal)
								{
								case internal::SerializableMemberInfo::EncapType::ETYPE_SMARTPOINTER:
									tempEtypeReal = *(iterEncap++);
									switch (tempEtypeReal)
									{
									case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
										if (iterEncap != endOfEncap)
											throw Serializable::UnavailableTypeException();
										{
											std::list<JsCPPUtils::SmartPointer<Serializable> > *plist = ((std::list<JsCPPUtils::SmartPointer<Serializable> >*)(*iterMem)->_memberInfo.ptr);
											plist->clear();
											for(rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
											{
												JsCPPUtils::SmartPointer<Serializable> obj = (*iterMem)->_memberInfo.createFactory->create();
												readElementFromPayload(*iter, obj.getPtr());
												plist->push_back(obj);
											}
										}
										break;
									default:
										throw Serializable::UnavailableTypeException();
									}
									break;
								case internal::SerializableMemberInfo::EncapType::ETYPE_STDVECTOR:
									_serializeCheckEoo(&tempEtypeReal, &iterEncap, endOfEncap);
									switch (tempEtypeReal & 0x00FF)
									{
									case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
										readStdListFromPayload(jsonValue, (std::list< std::vector<int8_t> >*)((*iterMem)->_memberInfo.ptr));
										break;
									case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
										readStdListFromPayload(jsonValue, (std::list< std::vector<uint8_t> >*)((*iterMem)->_memberInfo.ptr));
										break;
									case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
										readStdListFromPayload(jsonValue, (std::list< std::vector<int16_t> >*)((*iterMem)->_memberInfo.ptr));
										break;
									case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
										readStdListFromPayload(jsonValue, (std::list< std::vector<uint16_t> >*)((*iterMem)->_memberInfo.ptr));
										break;
									case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
										readStdListFromPayload(jsonValue, (std::list< std::vector<int32_t> >*)((*iterMem)->_memberInfo.ptr));
										break;
									case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
										readStdListFromPayload(jsonValue, (std::list< std::vector<uint32_t> >*)((*iterMem)->_memberInfo.ptr));
										break;
									case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
										readStdListFromPayload(jsonValue, (std::list< std::vector<int64_t> >*)((*iterMem)->_memberInfo.ptr));
										break;
									case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
										readStdListFromPayload(jsonValue, (std::list< std::vector<uint64_t> >*)((*iterMem)->_memberInfo.ptr));
										break;
									case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
										readStdListFromPayload(jsonValue, (std::list< std::vector<char> >*)((*iterMem)->_memberInfo.ptr));
										break;
									case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
										readStdListFromPayload(jsonValue, (std::list< std::vector<wchar_t> >*)((*iterMem)->_memberInfo.ptr));
										break;
									default:
										throw Serializable::UnavailableTypeException();
									}
									break;
								case internal::SerializableMemberInfo::EncapType::ETYPE_STDBASICSTRING:
									_serializeCheckEoo(&tempEtypeReal, &iterEncap, endOfEncap);
									if (checkFlagsAll(tempEtypeReal, internal::SerializableMemberInfo::EncapType::ETYPE_CHAR))
										readStdListFromPayload(jsonValue, (std::list< std::basic_string<char> > *)(*iterMem)->_memberInfo.ptr);
									else if (checkFlagsAll(tempEtypeReal, internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR))
										readStdListFromPayload(jsonValue, (std::list<std::basic_string<wchar_t> >*)(*iterMem)->_memberInfo.ptr);
									else
										throw Serializable::UnavailableTypeException();
								default:
									throw Serializable::UnavailableTypeException();
								}
							}
							break;
						case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
							readElementFromPayload(jsonValue, (Serializable*)(*iterMem)->_memberInfo.ptr);
							break;
						case internal::SerializableMemberInfo::EncapType::ETYPE_SMARTPOINTER:
							tempEtypeReal = *(iterEncap++);
							if (tempEtypeReal & internal::SerializableMemberInfo::EncapType::ETYPE_NULL)
							{
								(*iterMem)->setNull();
							}
							else {
								switch (tempEtypeReal & 0xFF00)
								{
								case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
									if (!(tempEtypeReal & internal::SerializableMemberInfo::EncapType::ETYPE_NULL))
									{
										JsCPPUtils::SmartPointer<Serializable> obj = (*iterMem)->_memberInfo.createFactory->create();
										readElementFromPayload(jsonValue, (Serializable*)obj.getPtr());
									}
								default:
									throw Serializable::UnavailableTypeException();
								}
							}
							break;
						case internal::SerializableMemberInfo::EncapType::ETYPE_NATIVEARRAY:
							switch (tempEtypeReal & 0x00FF)
							{
							case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
								readElementArrayFromPayload(jsonValue, (int8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
								readElementArrayFromPayload(jsonValue, (int8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
								readElementArrayFromPayload(jsonValue, (uint8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
								readElementArrayFromPayload(jsonValue, (int16_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
								readElementArrayFromPayload(jsonValue, (uint16_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
								readElementArrayFromPayload(jsonValue, (int32_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
								readElementArrayFromPayload(jsonValue, (uint32_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
								readElementArrayFromPayload(jsonValue, (int64_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
								readElementArrayFromPayload(jsonValue, (uint64_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
								readElementArrayFromPayload(jsonValue, (char*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
								readElementArrayFromPayload(jsonValue, (wchar_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
								readElementArrayFromPayload(jsonValue, (float*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
								readElementArrayFromPayload(jsonValue, (double*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							default:
								throw Serializable::UnavailableTypeException();
							}
							break;
						case internal::SerializableMemberInfo::EncapType::ETYPE_NULL:
							(*iterMem)->setNull();
							break;
						default:
							throw Serializable::UnavailableTypeException();
						}
					}
				}
			}
			else {
				(*iterMem)->setNull();
			}
			iterMem++;
		}
	}
}

#endif
