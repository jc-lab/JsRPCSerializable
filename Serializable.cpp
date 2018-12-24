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
 * @file	Serializable.cpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2018/12/06
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
 
#include "Serializable.h"

namespace JsRPC {

	const unsigned char Serializable::header[] = { 'J', 0x18, 'R', 'S', 0x00, 0x01 };

	Serializable::Serializable(const char *name, int64_t serialVersionUID)
	{
		m_name = name;
		m_serialVersionUID = serialVersionUID;
	}

	Serializable::~Serializable()
	{

	}

	template<typename T>
	static void writePtrToPayload(std::vector<unsigned char>& payload, const T *ptr, size_t length) {
		size_t pos = payload.size();
		if (length > 0)
		{
			payload.resize(pos + length, 0);
			memcpy(&payload[pos], ptr, length);
		}
	}
	template<>
	static void writePtrToPayload<bool>(std::vector<unsigned char>& payload, const bool *ptr, size_t length) {
		size_t i;
		for (i = 0; i < length; i++)
		{
			payload.push_back(ptr[i] ? 1 : 0);
		}
	}

	template<typename T>
	static T readFromPayload(const std::vector<unsigned char>& payload, uint32_t *pos)
	{
		size_t ds = sizeof(T);
		size_t remainsize = payload.size() - *pos;
		T value;
		if (remainsize < ds)
			throw Serializable::ParseException();
		if (ds > 0)
		{
			memcpy(&value, &payload[*pos], ds);
			*pos += ds;
		}
		return value;
	}

	static void readFromPayload(const std::vector<unsigned char>& payload, uint32_t *pos, void *ptr, uint32_t length)
	{
		size_t ds = length;
		size_t remainsize = payload.size() - *pos;
		if (remainsize < ds)
			throw Serializable::ParseException();
		if (ds > 0)
		{
			memcpy(ptr, &payload[*pos], ds);
			*pos += ds;
		}
	}

	void writeArrayElementSize(std::vector<unsigned char>& payload, uint32_t length) {
		writePtrToPayload(payload, &length, sizeof(length));
	}
	
	// Native Element
	template<typename T>
	static void writeElementToPayload(std::vector<unsigned char>& payload, const T *data) {
		writePtrToPayload(payload, data, sizeof(T));
	}
	static void writeElementToPayload(std::vector<unsigned char>& payload, const bool *data) {
		payload.push_back((unsigned char)(data ? 1 : 0));
	}
	static void writeElementToPayload(std::vector<unsigned char>& payload, const Serializable *data) {
		if (data) {
			std::vector<unsigned char> subpayload;
			data->serialize(subpayload);
			writeArrayElementSize(payload, subpayload.size());
			payload.insert(payload.end(), subpayload.begin(), subpayload.end());
		} else {
			writeArrayElementSize(payload, 0);
		}
	}

	template<typename T>
	static void readElementFromPayload(const std::vector<unsigned char>& payload, uint32_t *pos, T *data) {
		*data = readFromPayload<T>(payload, pos);
	}
	template<>
	static void readElementFromPayload<bool>(const std::vector<unsigned char>& payload, uint32_t *pos, bool *data) {
		*data = readFromPayload<unsigned char>(payload, pos) ? true : false;
	}
	template<>
	static void readElementFromPayload<Serializable>(const std::vector<unsigned char>& payload, uint32_t *pos, Serializable *data) {
		std::vector<unsigned char> subpayload;
		uint32_t size = readFromPayload<uint32_t>(payload, pos);
		const unsigned char *endptr = (const unsigned char*)&payload[*pos];
		endptr += size;
		subpayload.insert(subpayload.end(), (const unsigned char*)&payload[*pos], endptr);
		data->deserialize(subpayload);
		*pos += size;
	}

	template <typename T>
	static void writeElementArrayToPayload(std::vector<unsigned char>& payload, const T* data, size_t length)
	{
		writeArrayElementSize(payload, length);
		writePtrToPayload(payload, data, sizeof(T) * length);
	}

	template <typename T>
	static void readElementArrayFromPayload(const std::vector<unsigned char>& payload, uint32_t *pos, T* data, size_t length)
	{
		uint32_t size = readFromPayload<uint32_t>(payload, pos);
		if (length != size)
			throw Serializable::ParseException();
		readFromPayload(payload, pos, data, size * sizeof(T));
	}
	template <>
	static void readElementArrayFromPayload<bool>(const std::vector<unsigned char>& payload, uint32_t *pos, bool* data, size_t length)
	{
		uint32_t size = readFromPayload<uint32_t>(payload, pos);
		size_t ds = length;
		size_t remainsize = payload.size() - *pos;
		if (length != size)
			throw Serializable::ParseException();
		if (remainsize < ds)
			throw Serializable::ParseException();
		if (ds > 0)
		{
			uint32_t i;
			for (i = 0; i < size; i++)
			{
				data[i] = payload[*(pos++)] ? true : false;
			}
		}
	}

	template <typename T>
	static void writeElementToPayload(std::vector<unsigned char>& payload, const std::basic_string<T> *data) {
		uint32_t size = data->length();
		writeArrayElementSize(payload, size);
		writePtrToPayload(payload, (const T*)data->c_str(), sizeof(T) * size);
	}
	template <typename T>
	static void readElementFromPayload(const std::vector<unsigned char>& payload, uint32_t *pos, std::basic_string<T> *data) {
		uint32_t size = readFromPayload<uint32_t>(payload, pos);
		size_t remainsize = payload.size() - *pos;
		size_t datasize = size * sizeof(T);
		data->clear();
		if(remainsize < datasize)
			throw Serializable::ParseException();
		if (size > 0)
		{
			*data = std::basic_string<T>((const T*)&payload[*pos], size);
			*pos += datasize;
		}
	}
	template <typename T>
	static void writeStdVectorToPayload(std::vector<unsigned char>& payload, const std::vector<T> *data) {
		uint32_t size = data->size();
		writePtrToPayload(payload, &size, sizeof(size));
		if(size > 0)
			writePtrToPayload(payload, &(*data)[0], sizeof(T) * size);
	}
	template <>
	static void writeStdVectorToPayload(std::vector<unsigned char>& payload, const std::vector<bool> *data) {
		uint32_t size = data->size();
		writePtrToPayload(payload, &size, sizeof(size));
		for (std::vector<bool>::const_iterator iter = data->begin(); iter != data->end(); iter++)
		{
			bool element = *iter;
			writeElementToPayload(payload, &element);
		}
	}
	template <typename T>
	static void readStdVectorFromPayload(const std::vector<unsigned char>& payload, uint32_t *pos, std::vector<T> *data) {
		uint32_t size = readFromPayload<uint32_t>(payload, pos);
		size_t remainsize = payload.size() - *pos;
		size_t datasize = size * sizeof(T);
		data->clear();
		if (remainsize < datasize)
			throw Serializable::ParseException();
		if (size > 0)
		{
			const T* endptr = (const T*)&payload[*pos];
			endptr += size;
			data->insert(data->end(), (const T*)&payload[*pos], endptr);
			*pos += datasize;
		}
	}
	template <>
	static void readStdVectorFromPayload<bool>(const std::vector<unsigned char>& payload, uint32_t *pos, std::vector<bool> *data) {
		uint32_t size = readFromPayload<uint32_t>(payload, pos);
		size_t remainsize = payload.size() - *pos;
		size_t datasize = size * sizeof(bool);
		data->clear();
		if (remainsize < datasize)
			throw Serializable::ParseException();
		for (uint32_t i = 0; i < size; i++)
			data->push_back(&payload[*pos++] ? true : false);
	}

	template <class T>
	static void writeStdListToPayload(std::vector<unsigned char>& payload, const std::list<T> *data);
	template <typename T>
	static void writeStdListToPayload(std::vector<unsigned char>& payload, const std::list< std::basic_string<T> > *data)
	{
		uint32_t size = data->size();
		writeElementToPayload(payload, &size);
		for (std::list< std::basic_string< T > >::const_iterator iter = data->begin(); iter != data->end(); iter++)
		{
			writePtrToPayload(payload, &size, sizeof(size));
		}
	}
	template <typename T>
	static void writeStdListToPayload(std::vector<unsigned char>& payload, const std::list< std::vector<T> > *data)
	{
		uint32_t size = data->size();
		writeElementToPayload(payload, &size);
		for (std::list< std::vector< T > >::const_iterator iter = data->begin(); iter != data->end(); iter++)
		{
			writePtrToPayload(payload, &size, sizeof(size));
		}
	}

	template <class T>
	static void readStdListFromPayload(const std::vector<unsigned char>& payload, uint32_t *pos, std::list<T> *data);
	template <typename T>
	static void readStdListFromPayload(const std::vector<unsigned char>& payload, uint32_t *pos, std::list< std::basic_string<T> > *data)
	{
		uint32_t i;
		uint32_t size = readFromPayload<uint32_t>(payload, pos);
		data->clear();
		for (i = 0; i < size; i++)
		{
			std::basic_string<T> element;
			readElementFromPayload(payload, pos, &element);
			data->push_back(element);
		}
	}
	template <typename T>
	static void readStdListFromPayload(const std::vector<unsigned char>& payload, uint32_t *pos, std::list< std::vector<T> > *data)
	{
		uint32_t i;
		uint32_t size = readFromPayload<uint32_t>(payload, pos);
		data->clear();
		for (i = 0; i < size; i++)
		{
			std::vector<T> element;
			readStdVectorFromPayload(payload, pos, &element);
			data->push_back(element);
		}
	}

	internal::STypeCommon &Serializable::serializableMapMember(const char *name, internal::STypeCommon &object)
	{
		object._memberInfo.name = name;
		m_members.push_back(&object);
		return object;
	}

	void Serializable::serializableClearObjects()
	{
		for (std::list<internal::STypeCommon*>::const_iterator iter = m_members.begin(); iter != m_members.end(); iter++)
		{
			(*iter)->clear();
		}
	}

	static void _serializeCheckNotEoo(uint16_t *tempEtype, std::list<internal::SerializableMemberInfo::EncapType>::const_iterator *iterEncap, std::list<internal::SerializableMemberInfo::EncapType>::const_iterator endOfEncap)
	{
		if ((*iterEncap) == endOfEncap)
			throw Serializable::UnavailableTypeException();
		*tempEtype = *((*iterEncap)++);
	}

	static void _serializeCheckEoo(uint16_t *tempEtype, std::list<internal::SerializableMemberInfo::EncapType>::const_iterator *iterEncap, std::list<internal::SerializableMemberInfo::EncapType>::const_iterator endOfEncap)
	{
		*tempEtype = *((*iterEncap)++);
		if ((*iterEncap) != endOfEncap)
			throw Serializable::UnavailableTypeException();
	}

	void Serializable::serialize(std::vector<unsigned char>& payload) const throw(UnavailableTypeException)
	{
		uint32_t pos = 0;

		payload.clear();
		payload.assign(sizeof(header) + 9 + m_name.length(), 0);

		// [0] Header
		memcpy(&payload[0], header, sizeof(header));
		pos += sizeof(header);
		// [6] Version
		payload[pos++] = m_name.length();
		memcpy(&payload[pos], m_name.c_str(), m_name.length());
		pos += m_name.length();
		payload[pos++] = ((unsigned char)(m_serialVersionUID >> 0));
		payload[pos++] = ((unsigned char)(m_serialVersionUID >> 8));
		payload[pos++] = ((unsigned char)(m_serialVersionUID >> 16));
		payload[pos++] = ((unsigned char)(m_serialVersionUID >> 24));
		payload[pos++] = ((unsigned char)(m_serialVersionUID >> 32));
		payload[pos++] = ((unsigned char)(m_serialVersionUID >> 40));
		payload[pos++] = ((unsigned char)(m_serialVersionUID >> 48));
		payload[pos++] = ((unsigned char)(m_serialVersionUID >> 56));

		// Data
		for (std::list<internal::STypeCommon*>::const_iterator iterMem = m_members.begin(); iterMem != m_members.end(); iterMem++)
		{
			std::list<internal::SerializableMemberInfo::EncapType>::const_iterator iterEncap = (*iterMem)->_memberInfo.encaps.begin();
			std::list<internal::SerializableMemberInfo::EncapType>::const_iterator endOfEncap = (*iterMem)->_memberInfo.encaps.end();
			uint16_t tempEtype;
			_serializeCheckNotEoo(&tempEtype, &iterEncap, endOfEncap);
			if ((*iterMem)->isNull())
			{
				tempEtype |= internal::SerializableMemberInfo::EncapType::ETYPE_NULL;
				writeElementToPayload(payload, &tempEtype);
			}
			else {
				writeElementToPayload(payload, &tempEtype);
				if ((tempEtype & 0xFF00) == internal::SerializableMemberInfo::EncapType::ETYPE_NATIVE)
				{
					switch (tempEtype & 0x00FF)
					{
					case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
						writeElementToPayload<bool>(payload, (bool*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
						writeElementToPayload<int8_t>(payload, (int8_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
						writeElementToPayload<uint8_t>(payload, (uint8_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
						writeElementToPayload<int16_t>(payload, (int16_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
						writeElementToPayload<uint16_t>(payload, (uint16_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
						writeElementToPayload<int32_t>(payload, (int32_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
						writeElementToPayload<uint32_t>(payload, (uint32_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
						writeElementToPayload<int64_t>(payload, (int64_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
						writeElementToPayload<uint64_t>(payload, (uint64_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
						writeElementToPayload<char>(payload, (char*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
						writeElementToPayload<wchar_t>(payload, (wchar_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
						writeElementToPayload<float>(payload, (float*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
						writeElementToPayload<double>(payload, (double*)((*iterMem)->_memberInfo.ptr));
						break;
					default:
						throw UnavailableTypeException();
					}
				}
				else {
					switch (tempEtype)
					{
					case internal::SerializableMemberInfo::EncapType::ETYPE_STDBASICSTRING:
						_serializeCheckNotEoo(&tempEtype, &iterEncap, endOfEncap);
						writeElementToPayload(payload, &tempEtype);
						if (iterEncap != endOfEncap)
							throw UnavailableTypeException();
						if (checkFlagsAll(tempEtype, internal::SerializableMemberInfo::EncapType::ETYPE_CHAR))
							writeElementToPayload(payload, (const std::basic_string<char>*)(*iterMem)->_memberInfo.ptr);
						else if (checkFlagsAll(tempEtype, internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR))
							writeElementToPayload(payload, (const std::basic_string<wchar_t>*)(*iterMem)->_memberInfo.ptr);
						else
							throw UnavailableTypeException();
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_STDVECTOR:
						_serializeCheckNotEoo(&tempEtype, &iterEncap, endOfEncap);
						writeElementToPayload(payload, &tempEtype);
						if (iterEncap != endOfEncap)
							throw UnavailableTypeException();
						switch (tempEtype & 0x00FF)
						{
						case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
							writeStdVectorToPayload(payload, (std::vector<bool>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
							writeStdVectorToPayload(payload, (std::vector<int8_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
							writeStdVectorToPayload(payload, (std::vector<uint8_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
							writeStdVectorToPayload(payload, (std::vector<int16_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
							writeStdVectorToPayload(payload, (std::vector<uint16_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
							writeStdVectorToPayload(payload, (std::vector<int32_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
							writeStdVectorToPayload(payload, (std::vector<uint32_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
							writeStdVectorToPayload(payload, (std::vector<int64_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
							writeStdVectorToPayload(payload, (std::vector<uint64_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
							writeStdVectorToPayload(payload, (std::vector<char>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
							writeStdVectorToPayload(payload, (std::vector<wchar_t>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
							writeStdVectorToPayload(payload, (std::vector<float>*)((*iterMem)->_memberInfo.ptr));
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
							writeStdVectorToPayload(payload, (std::vector<double>*)((*iterMem)->_memberInfo.ptr));
							break;
						default:
							throw UnavailableTypeException();
						}
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_STDLIST:
						_serializeCheckNotEoo(&tempEtype, &iterEncap, endOfEncap);
						writeElementToPayload(payload, &tempEtype);
						if (iterEncap == endOfEncap)
						{
							throw UnavailableTypeException();
						}
						else {
							switch (tempEtype)
							{
							case internal::SerializableMemberInfo::EncapType::ETYPE_SMARTPOINTER:
								tempEtype = *(iterEncap++);
								writeElementToPayload(payload, &tempEtype);
								switch (tempEtype)
								{
								case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
									if (iterEncap != endOfEncap)
										throw UnavailableTypeException();
									writeArrayElementSize(payload, ((std::list<JsCPPUtils::SmartPointer<Serializable> >*)(*iterMem)->_memberInfo.ptr)->size());
									for (std::list<JsCPPUtils::SmartPointer<Serializable> >::const_iterator subiter = ((std::list<JsCPPUtils::SmartPointer<Serializable> >*)(*iterMem)->_memberInfo.ptr)->begin(); subiter != ((std::list<JsCPPUtils::SmartPointer<Serializable> >*)(*iterMem)->_memberInfo.ptr)->end(); subiter++)
									{
										writeElementToPayload(payload, subiter->getPtr());
									}
									break;
								default:
									throw UnavailableTypeException();
								}
								break;
							case internal::SerializableMemberInfo::EncapType::ETYPE_STDVECTOR:
								_serializeCheckEoo(&tempEtype, &iterEncap, endOfEncap);
								writeElementToPayload(payload, &tempEtype);
								switch (tempEtype & 0x00FF)
								{
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
									writeStdListToPayload(payload, (std::list< std::vector<int8_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
									writeStdListToPayload(payload, (std::list< std::vector<uint8_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
									writeStdListToPayload(payload, (std::list< std::vector<int16_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
									writeStdListToPayload(payload, (std::list< std::vector<uint16_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
									writeStdListToPayload(payload, (std::list< std::vector<int32_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
									writeStdListToPayload(payload, (std::list< std::vector<uint32_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
									writeStdListToPayload(payload, (std::list< std::vector<int64_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
									writeStdListToPayload(payload, (std::list< std::vector<uint64_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
									writeStdListToPayload(payload, (std::list< std::vector<char> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
									writeStdListToPayload(payload, (std::list< std::vector<wchar_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								default:
									throw UnavailableTypeException();
								}
								break;
							case internal::SerializableMemberInfo::EncapType::ETYPE_STDBASICSTRING:
								_serializeCheckEoo(&tempEtype, &iterEncap, endOfEncap);
								writeElementToPayload(payload, &tempEtype);
								if (checkFlagsAll(tempEtype, internal::SerializableMemberInfo::EncapType::ETYPE_CHAR))
									writeStdListToPayload(payload, (std::list< std::basic_string<char> > *)(*iterMem)->_memberInfo.ptr);
								else if (checkFlagsAll(tempEtype, internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR))
									writeStdListToPayload(payload, (std::list<std::basic_string<wchar_t> >*)(*iterMem)->_memberInfo.ptr);
								else
									throw UnavailableTypeException();
								break;
							default:
								throw UnavailableTypeException();
							}
						}
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
						writeElementToPayload(payload, (Serializable*)(*iterMem)->_memberInfo.ptr);
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_SMARTPOINTER:
						tempEtype = *(iterEncap++);
						switch (tempEtype)
						{
						case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
							if (iterEncap != endOfEncap)
								throw UnavailableTypeException();
							if (!((JsCPPUtils::SmartPointer<Serializable>*)(*iterMem)->_memberInfo.ptr)->getPtr() || (*iterMem)->isNull())
							{
								tempEtype |= internal::SerializableMemberInfo::EncapType::ETYPE_NULL;
								writeElementToPayload(payload, &tempEtype);
							} else {
								writeElementToPayload(payload, &tempEtype);
								writeElementToPayload(payload, ((JsCPPUtils::SmartPointer<Serializable>*)(*iterMem)->_memberInfo.ptr)->getPtr());
							}

							break;
						default:
							throw UnavailableTypeException();
						}
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_NATIVEARRAY:
						switch (tempEtype & 0x00FF)
						{
						case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
							writeElementArrayToPayload(payload, (int8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
							writeElementArrayToPayload(payload, (int8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
							writeElementArrayToPayload(payload, (uint8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
							writeElementArrayToPayload(payload, (int16_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
							writeElementArrayToPayload(payload, (uint16_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
							writeElementArrayToPayload(payload, (int32_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
							writeElementArrayToPayload(payload, (uint32_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
							writeElementArrayToPayload(payload, (int64_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
							writeElementArrayToPayload(payload, (uint64_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
							writeElementArrayToPayload(payload, (char*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
							writeElementArrayToPayload(payload, (wchar_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
							writeElementArrayToPayload(payload, (float*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
							writeElementArrayToPayload(payload, (double*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
						default:
							throw UnavailableTypeException();
						}
						break;
					default:
						throw UnavailableTypeException();
					}
				}
			}
		}
	}

	void Serializable::deserialize(const std::vector<unsigned char>& payload) throw(ParseException)
	{
		uint32_t pos = 0;
		size_t remainsize = 0;
		size_t totalsize = payload.size();
		int headersize = sizeof(header) + 9 + m_name.length();
		unsigned char serialVersionUID[8];

		std::list<internal::STypeCommon*>::iterator iterMem = m_members.begin();

		if (payload.size() < headersize)
		{
			throw ParseException();
		}
		if (memcmp(&payload[0], header, sizeof(header)))
		{
			throw ParseException();
		}
		pos += sizeof(header);
		if (payload[pos++] != m_name.length())
		{
			throw ParseException();
		}
		if (memcmp(&payload[pos], m_name.c_str(), m_name.length()))
		{
			throw ParseException();
		}
		pos += m_name.length();
		serialVersionUID[0] = ((unsigned char)(m_serialVersionUID >> 0));
		serialVersionUID[1] = ((unsigned char)(m_serialVersionUID >> 8));
		serialVersionUID[2] = ((unsigned char)(m_serialVersionUID >> 16));
		serialVersionUID[3] = ((unsigned char)(m_serialVersionUID >> 24));
		serialVersionUID[4] = ((unsigned char)(m_serialVersionUID >> 32));
		serialVersionUID[5] = ((unsigned char)(m_serialVersionUID >> 40));
		serialVersionUID[6] = ((unsigned char)(m_serialVersionUID >> 48));
		serialVersionUID[7] = ((unsigned char)(m_serialVersionUID >> 56));
		if (memcmp(&payload[pos], serialVersionUID, sizeof(serialVersionUID)))
		{
			throw ParseException();
		}
		pos += 8;

		remainsize = totalsize - pos;
		while (remainsize > 0 && iterMem != m_members.end())
		{
			std::list<internal::SerializableMemberInfo::EncapType>::const_iterator iterEncap = (*iterMem)->_memberInfo.encaps.begin();
			std::list<internal::SerializableMemberInfo::EncapType>::const_iterator endOfEncap = (*iterMem)->_memberInfo.encaps.end();
			uint16_t tempEtypeRecv;
			uint16_t tempEtypeReal;
			_serializeCheckNotEoo(&tempEtypeReal, &iterEncap, endOfEncap);
			tempEtypeRecv = readFromPayload<uint16_t>(payload, &pos);
			(*iterMem)->clear();
			(*iterMem)->setNull(tempEtypeRecv & internal::SerializableMemberInfo::EncapType::ETYPE_NULL);
			if (!(tempEtypeRecv & internal::SerializableMemberInfo::EncapType::ETYPE_NULL))
			{
				if ((tempEtypeRecv & 0xFF00) == internal::SerializableMemberInfo::EncapType::ETYPE_NATIVE)
				{
					switch (tempEtypeRecv & 0x00FF)
					{
					case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
						readElementFromPayload<bool>(payload, &pos, (bool*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
						readElementFromPayload<int8_t>(payload, &pos, (int8_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
						readElementFromPayload<uint8_t>(payload, &pos, (uint8_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
						readElementFromPayload<int16_t>(payload, &pos, (int16_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
						readElementFromPayload<uint16_t>(payload, &pos, (uint16_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
						readElementFromPayload<int32_t>(payload, &pos, (int32_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
						readElementFromPayload<uint32_t>(payload, &pos, (uint32_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
						readElementFromPayload<int64_t>(payload, &pos, (int64_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
						readElementFromPayload<uint64_t>(payload, &pos, (uint64_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
						readElementFromPayload<char>(payload, &pos, (char*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
						readElementFromPayload<wchar_t>(payload, &pos, (wchar_t*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
						readElementFromPayload<float>(payload, &pos, (float*)((*iterMem)->_memberInfo.ptr));
						break;
					case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
						readElementFromPayload<double>(payload, &pos, (double*)((*iterMem)->_memberInfo.ptr));
						break;
					default:
						throw UnavailableTypeException();
					}
				}
				else {
					switch (tempEtypeRecv)
					{
					case internal::SerializableMemberInfo::EncapType::ETYPE_STDBASICSTRING:
						_serializeCheckEoo(&tempEtypeReal, &iterEncap, endOfEncap);
						tempEtypeRecv = readFromPayload<uint16_t>(payload, &pos);
						if (iterEncap != endOfEncap)
							throw UnavailableTypeException();
						if (checkFlagsAll(tempEtypeReal, internal::SerializableMemberInfo::EncapType::ETYPE_CHAR))
							readElementFromPayload(payload, &pos, (std::basic_string<char>*)(*iterMem)->_memberInfo.ptr);
						else if (checkFlagsAll(tempEtypeReal, internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR))
							readElementFromPayload(payload, &pos, (std::basic_string<wchar_t>*)(*iterMem)->_memberInfo.ptr);
						else
							throw UnavailableTypeException();
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_STDVECTOR:
						_serializeCheckNotEoo(&tempEtypeReal, &iterEncap, endOfEncap);
						tempEtypeRecv = readFromPayload<uint16_t>(payload, &pos);
						if (iterEncap != endOfEncap)
							throw UnavailableTypeException();
						if (!(tempEtypeRecv & internal::SerializableMemberInfo::EncapType::ETYPE_NULL))
						{
							switch (tempEtypeRecv & 0x00FF)
							{
							case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
								readStdVectorFromPayload(payload, &pos,(std::vector<bool>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
								readStdVectorFromPayload(payload, &pos, (std::vector<int8_t>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
								readStdVectorFromPayload(payload, &pos, (std::vector<uint8_t>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
								readStdVectorFromPayload(payload, &pos, (std::vector<int16_t>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
								readStdVectorFromPayload(payload, &pos, (std::vector<uint16_t>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
								readStdVectorFromPayload(payload, &pos, (std::vector<int32_t>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
								readStdVectorFromPayload(payload, &pos, (std::vector<uint32_t>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
								readStdVectorFromPayload(payload, &pos, (std::vector<int64_t>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
								readStdVectorFromPayload(payload, &pos, (std::vector<uint64_t>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
								readStdVectorFromPayload(payload, &pos, (std::vector<char>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
								readStdVectorFromPayload(payload, &pos, (std::vector<wchar_t>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
								readStdVectorFromPayload(payload, &pos, (std::vector<float>*)((*iterMem)->_memberInfo.ptr));
								break;
							case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
								readStdVectorFromPayload(payload, &pos, (std::vector<double>*)((*iterMem)->_memberInfo.ptr));
								break;
							default:
								throw UnavailableTypeException();
							}
						}
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_STDLIST:
						_serializeCheckNotEoo(&tempEtypeReal, &iterEncap, endOfEncap);
						tempEtypeRecv = readFromPayload<uint16_t>(payload, &pos);
						if (iterEncap == endOfEncap)
						{
							throw UnavailableTypeException();
						}
						else {
							switch (tempEtypeReal)
							{
							case internal::SerializableMemberInfo::EncapType::ETYPE_SMARTPOINTER:
								tempEtypeReal = *(iterEncap++);
								tempEtypeRecv = readFromPayload<uint16_t>(payload, &pos);
								switch (tempEtypeReal)
								{
								case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
									if (iterEncap != endOfEncap)
										throw UnavailableTypeException();
									{
										uint32_t i;
										uint32_t length = readFromPayload<uint32_t>(payload, &pos);
										std::list<JsCPPUtils::SmartPointer<Serializable> > *plist = ((std::list<JsCPPUtils::SmartPointer<Serializable> >*)(*iterMem)->_memberInfo.ptr);
										plist->clear();
										for (i = 0; i < length; i++)
										{
											JsCPPUtils::SmartPointer<Serializable> obj = (*iterMem)->_memberInfo.createFactory->create();
											readElementFromPayload(payload, &pos, obj.getPtr());
											plist->push_back(obj);
										}
									}
									break;
								default:
									throw UnavailableTypeException();
								}
								break;
							case internal::SerializableMemberInfo::EncapType::ETYPE_STDVECTOR:
								_serializeCheckEoo(&tempEtypeReal, &iterEncap, endOfEncap);
								tempEtypeRecv = readFromPayload<uint16_t>(payload, &pos);
								switch (tempEtypeReal & 0x00FF)
								{
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
									readStdListFromPayload(payload, &pos, (std::list< std::vector<int8_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
									readStdListFromPayload(payload, &pos, (std::list< std::vector<uint8_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
									readStdListFromPayload(payload, &pos, (std::list< std::vector<int16_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
									readStdListFromPayload(payload, &pos, (std::list< std::vector<uint16_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
									readStdListFromPayload(payload, &pos, (std::list< std::vector<int32_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
									readStdListFromPayload(payload, &pos, (std::list< std::vector<uint32_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
									readStdListFromPayload(payload, &pos, (std::list< std::vector<int64_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
									readStdListFromPayload(payload, &pos, (std::list< std::vector<uint64_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
									readStdListFromPayload(payload, &pos, (std::list< std::vector<char> >*)((*iterMem)->_memberInfo.ptr));
									break;
								case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
									readStdListFromPayload(payload, &pos, (std::list< std::vector<wchar_t> >*)((*iterMem)->_memberInfo.ptr));
									break;
								default:
									throw UnavailableTypeException();
								}
								break;
							case internal::SerializableMemberInfo::EncapType::ETYPE_STDBASICSTRING:
								_serializeCheckEoo(&tempEtypeReal, &iterEncap, endOfEncap);
								tempEtypeRecv = readFromPayload<uint16_t>(payload, &pos);
								if (checkFlagsAll(tempEtypeReal, internal::SerializableMemberInfo::EncapType::ETYPE_CHAR))
									readStdListFromPayload(payload, &pos, (std::list< std::basic_string<char> > *)(*iterMem)->_memberInfo.ptr);
								else if (checkFlagsAll(tempEtypeReal, internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR))
									readStdListFromPayload(payload, &pos, (std::list<std::basic_string<wchar_t> >*)(*iterMem)->_memberInfo.ptr);
								else
									throw UnavailableTypeException();
							default:
								throw UnavailableTypeException();
							}
						}
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
						readElementFromPayload(payload, &pos, (Serializable*)(*iterMem)->_memberInfo.ptr);
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_SMARTPOINTER:
						tempEtypeReal = *(iterEncap++);
						tempEtypeRecv = readFromPayload<uint16_t>(payload, &pos);
						if (tempEtypeRecv & internal::SerializableMemberInfo::EncapType::ETYPE_NULL)
						{
							(*iterMem)->setNull();
						}else{
							switch (tempEtypeReal & 0xFF00)
							{
							case internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD:
								if (!(tempEtypeRecv & internal::SerializableMemberInfo::EncapType::ETYPE_NULL))
								{
									JsCPPUtils::SmartPointer<Serializable> obj = (*iterMem)->_memberInfo.createFactory->create();
									readElementFromPayload(payload, &pos, (Serializable*)obj.getPtr());
								}
							default:
								throw UnavailableTypeException();
							}
						}
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_NATIVEARRAY:
						switch (tempEtypeRecv & 0x00FF)
						{
						case (internal::SerializableMemberInfo::EncapType::ETYPE_BOOL):
							readElementArrayFromPayload(payload, &pos, (int8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 1):
							readElementArrayFromPayload(payload, &pos, (int8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 1):
							readElementArrayFromPayload(payload, &pos, (uint8_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 2):
							readElementArrayFromPayload(payload, &pos, (int16_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 2):
							readElementArrayFromPayload(payload, &pos, (uint16_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 4):
							readElementArrayFromPayload(payload, &pos, (int32_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 4):
							readElementArrayFromPayload(payload, &pos, (uint32_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_SINT | 8):
							readElementArrayFromPayload(payload, &pos, (int64_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_UINT | 8):
							readElementArrayFromPayload(payload, &pos, (uint64_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_CHAR):
							readElementArrayFromPayload(payload, &pos, (char*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_WCHAR):
							readElementArrayFromPayload(payload, &pos, (wchar_t*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_FLOAT):
							readElementArrayFromPayload(payload, &pos, (float*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
							break;
						case (internal::SerializableMemberInfo::EncapType::ETYPE_DOUBLE):
							readElementArrayFromPayload(payload, &pos, (double*)((*iterMem)->_memberInfo.ptr), (*iterMem)->_memberInfo.length);
						default:
							throw UnavailableTypeException();
						}
						break;
					case internal::SerializableMemberInfo::EncapType::ETYPE_NULL:
						(*iterMem)->setNull();
						break;
					default:
						throw UnavailableTypeException();
					}
				}
			}
			iterMem++;
			remainsize = totalsize - pos;
		}
		if (remainsize != 0)
			throw ParseException();
		if (iterMem != m_members.end())
			throw ParseException();
	}
}
