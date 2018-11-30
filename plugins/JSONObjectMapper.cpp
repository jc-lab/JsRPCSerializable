#include "JSONObjectMapper.h"

#if defined(HAS_RAPIDJSON) && HAS_RAPIDJSON
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#endif

#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
#include <JsCPPUtils/StringBuffer.h>
#include <JsCPPUtils/StringEncoding.h>
#endif

namespace JsRPC {
#if defined(HAS_RAPIDJSON) && HAS_RAPIDJSON

	//template<typename T, class AllocatorT>
	//static void _serializeStdBasicString(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const std::basic_string<T> &text);
	template<class AllocatorT>
	static void _serializeStdBasicString(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const std::basic_string<char> &text)
	{
		jsonValue.SetString(text.c_str(), text.length(), jsonAllocator);
	}
	template<class AllocatorT>
	static void _serializeStdBasicString(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const std::basic_string<wchar_t> &text)
	{
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
		JsCPPUtils::StringBuffer<char> sbUtf8Text = JsCPPUtils::StringEncoding::StringToUTF8SB(text);
		jsonValue.SetString(sbUtf8Text.c_str(), sbUtf8Text.length(), jsonAllocator);
#else
		assert(0);
#endif
	}

	template<class AllocatorT>
	static void _serializeSubPayload(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, Serializable *ptr)
	{
		if (ptr == NULL)
		{
			jsonValue.SetNull();
		} else {
			JSONObjectMapper::serializeTo(ptr, jsonValue, jsonAllocator);
		}
	}

	template<typename T, class AllocatorT>
	static void _serializeStdListSmartPointerStdBasicString(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const void *ptr)
	{
		const std::list< JsCPPUtils::SmartPointer< std::basic_string<T> > > *plist = (const std::list< JsCPPUtils::SmartPointer< std::basic_string<T> > > *)ptr;
		int32_t listSize = plist->size();
		int32_t i;
		jsonValue.SetArray();
		for (std::list< JsCPPUtils::SmartPointer< std::basic_string<T> > >::const_iterator iter = plist->begin(); iter != plist->end(); iter++)
		{
			rapidjson::Value jsonElement;
			_serializeStdBasicString(jsonElement, jsonAllocator, *(*iter));
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}

	template<class AllocatorT>
	static void _serializeStdListSmartPointerSubPayload(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const void *ptr)
	{
		const std::list< JsCPPUtils::SmartPointer< Serializable > > *plist = (const std::list< JsCPPUtils::SmartPointer< Serializable > > *)ptr;
		int32_t listSize = plist->size();
		int32_t i;
		jsonValue.SetArray();
		for (std::list< JsCPPUtils::SmartPointer< Serializable > >::const_iterator iter = plist->begin(); iter != plist->end(); iter++)
		{
			rapidjson::Value jsonElement;
			_serializeSubPayload(jsonElement, jsonAllocator, iter->getPtr());
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}

	template<typename T, class AllocatorT>
	static void _serializeStdListStdBasicString(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const std::list< std::basic_string<T> > *plist)
	{
		int32_t listSize = plist->size();
		int32_t i;
		jsonValue.SetArray();
		for (std::list< std::basic_string<T> >::const_iterator iter = plist->begin(); iter != plist->end(); iter++)
		{
			rapidjson::Value jsonElement;
			_serializeStdBasicString(jsonElement, jsonAllocator, (*iter));
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}

	template<class AllocatorT>
	static void _serializeStdListSubPayload(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, std::list< JsRPC::Serializable*> *plist)
	{
		jsonValue.SetArray();
		for (std::list< JsRPC::Serializable*>::iterator iter = plist->begin(); iter != plist->end(); iter++)
		{
			rapidjson::Value jsonElement;
			if (*iter == NULL)
			{
				jsonElement.SetNull();
			} else {
				JSONObjectMapper::serializeTo(*iter, jsonElement, jsonAllocator);
			}
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}

	template<class AllocatorT>
	static void _serializeStdList(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const SerializableMemberInfo &memberInfo)
	{
		int32_t i;
		int32_t listSize;
		if (memberInfo.mtype & SerializableMemberInfo::MTYPE_JSCPPUTILSMARTPOINTER)
		{
			if (memberInfo.mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_CHAR:
					_serializeStdListSmartPointerStdBasicString<char>(jsonValue, jsonAllocator, memberInfo.ptr);
					return;
				case SerializableMemberInfo::PTYPE_WCHAR:
					_serializeStdListSmartPointerStdBasicString<wchar_t>(jsonValue, jsonAllocator, memberInfo.ptr);
					return;
				}
			}
			else
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_SUBPAYLOAD:
					_serializeStdListSmartPointerSubPayload(jsonValue, jsonAllocator, memberInfo.ptr);
					return;
				}
			}
		}
		else
		{
			if (memberInfo.mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_CHAR:
					_serializeStdListStdBasicString(jsonValue, jsonAllocator, (const std::list< std::basic_string<char> >*)memberInfo.ptr);
					return;
				case SerializableMemberInfo::PTYPE_WCHAR:
					_serializeStdListStdBasicString(jsonValue, jsonAllocator, (const std::list< std::basic_string<wchar_t> >*)memberInfo.ptr);
					return;
				}
			}
			else
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_SUBPAYLOAD:
					_serializeStdListSubPayload(jsonValue, jsonAllocator, (std::list< JsRPC::Serializable*> *)memberInfo.ptr);
					return;
				}
			}
		}
		throw Serializable::UnavailableTypeException();
	}

	template<class AllocatorT>
	static void _serializeStdVectorNativeBool(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, void *ptr)
	{
		const std::vector<bool> *pvector = (const std::vector<bool>*)ptr;
		jsonValue.SetArray();
		if (pvector->size())
		{
			for (std::vector<bool>::const_iterator iter = pvector->begin(); iter != pvector->end(); iter++)
			{
				rapidjson::Value jsonElement; jsonElement.SetBool(*iter);
				jsonValue.PushBack(jsonElement, jsonAllocator);
			}
		}
	}

	template<typename T>
	static void _jsonSetByType(rapidjson::Value &jsonValue, char value) {
		jsonValue.SetInt(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, wchar_t value) {
		jsonValue.SetInt(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, int8_t value) {
		jsonValue.SetInt(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, uint8_t value) {
		jsonValue.SetUint(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, int16_t value) {
		jsonValue.SetInt(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, uint16_t value) {
		jsonValue.SetUint(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, int32_t value) {
		jsonValue.SetInt(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, uint32_t value) {
		jsonValue.SetUint(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, int64_t value) {
		jsonValue.SetInt64(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, uint64_t value) {
		jsonValue.SetUint64(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, float value) {
		jsonValue.SetFloat(value);
	}
	static void _jsonSetByType(rapidjson::Value &jsonValue, double value) {
		jsonValue.SetDouble(value);
	}

	template<typename T, class AllocatorT>
	static void _serializeStdVectorNative(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, void *ptr)
	{
		const std::vector<T> *pvector = (const std::vector<T>*)ptr;
		jsonValue.SetArray();
		if (pvector->size())
		{
			for (std::vector<T>::const_iterator iter = pvector->begin(); iter != pvector->end(); iter++)
			{
				rapidjson::Value jsonElement;
				_jsonSetByType(jsonElement, *iter);
				jsonValue.PushBack(jsonElement, jsonAllocator);
			}
		}
	}

	template<class AllocatorT>
	static void _serializeStdVector(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			_serializeStdVectorNativeBool(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
			_serializeStdVectorNative<int8_t>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT8:
			_serializeStdVectorNative<uint8_t>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT16:
			_serializeStdVectorNative<int16_t>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT16:
			_serializeStdVectorNative<uint16_t>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT32:
			_serializeStdVectorNative<int32_t>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT32:
			_serializeStdVectorNative<uint32_t>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT64:
			_serializeStdVectorNative<int64_t>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT64:
			_serializeStdVectorNative<uint64_t>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_FLOAT:
			_serializeStdVectorNative<float>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_DOUBLE:
			_serializeStdVectorNative<double>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_CHAR:
			_serializeStdVectorNative<char>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_WCHAR:
			_serializeStdVectorNative<wchar_t>(jsonValue, jsonAllocator, memberInfo.ptr);
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	template<class AllocatorT>
	static void _serializeNativeArrayBool(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const SerializableMemberInfo &memberInfo)
	{
		int32_t i;
		const bool *pdata = (const bool*)memberInfo.ptr;
		jsonValue.SetArray();
		for (i = 0; i < memberInfo.length; i++)
		{
			rapidjson::Value jsonElement;
			jsonElement.SetBool(pdata[i] ? true : false);
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}

	template<typename T, class AllocatorT>
	static void _serializeNativeArrayT(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const SerializableMemberInfo &memberInfo)
	{
		int32_t i;
		const T *pdata = (const T*)memberInfo.ptr;
		jsonValue.SetArray();
		for (i = 0; i < memberInfo.length; i++)
		{
			rapidjson::Value jsonElement;
			_jsonSetByType(jsonElement, pdata[i]);
			jsonValue.PushBack(jsonElement, jsonAllocator);
		}
	}

	template<class AllocatorT>
	static void _serializeNativeArray(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			_serializeNativeArrayBool(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
			_serializeNativeArrayT<int8_t>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_UINT8:
			_serializeNativeArrayT<uint8_t>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_SINT16:
			_serializeNativeArrayT<int16_t>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_UINT16:
			_serializeNativeArrayT<uint16_t>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_SINT32:
			_serializeNativeArrayT<int32_t>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_UINT32:
			_serializeNativeArrayT<uint32_t>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_SINT64:
			_serializeNativeArrayT<int64_t>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_UINT64:
			_serializeNativeArrayT<uint64_t>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_FLOAT:
			_serializeNativeArrayT<float>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_DOUBLE:
			_serializeNativeArrayT<double>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_CHAR:
			_serializeNativeArrayT<char>(jsonValue, jsonAllocator, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_WCHAR:
			_serializeNativeArrayT<wchar_t>(jsonValue, jsonAllocator, memberInfo);
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	template<class AllocatorT>
	static void _serializeNativeObject(rapidjson::Value &jsonValue, AllocatorT &jsonAllocator, const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			_jsonSetByType<bool>(jsonValue, *((bool*)memberInfo.ptr));
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
			_jsonSetByType<int8_t>(jsonValue, *((int8_t*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_UINT8:
			_jsonSetByType<uint8_t>(jsonValue, *((uint8_t*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_SINT16:
			_jsonSetByType<int16_t>(jsonValue, *((int16_t*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_UINT16:
			_jsonSetByType<uint16_t>(jsonValue, *((uint16_t*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_SINT32:
			_jsonSetByType<int32_t>(jsonValue, *((int32_t*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_UINT32:
			_jsonSetByType<uint32_t>(jsonValue, *((uint32_t*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_SINT64:
			_jsonSetByType<int64_t>(jsonValue, *((int64_t*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_UINT64:
			_jsonSetByType<uint64_t>(jsonValue, *((uint64_t*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_FLOAT:
			_jsonSetByType<float>(jsonValue, *((float*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_DOUBLE:
			_jsonSetByType<double>(jsonValue, *((double*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_CHAR:
			_jsonSetByType<char>(jsonValue, *((char*)memberInfo.ptr));
		case SerializableMemberInfo::PTYPE_WCHAR:
			_jsonSetByType<wchar_t>(jsonValue, *((wchar_t*)memberInfo.ptr));
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	void JSONObjectMapper::serializeTo(Serializable *serialiable, rapidjson::Document &jsonDoc)
	{
		rapidjson::Document::AllocatorType &jsonAllocator = jsonDoc.GetAllocator();
		size_t pos = 0;

		const std::list<SerializableMemberInfo> &members = serialiable->serializableMembers();

		jsonDoc.SetObject();

		// Data
		for (std::list<SerializableMemberInfo>::const_iterator iter = members.begin(); iter != members.end(); iter++)
		{
			rapidjson::Value jsonKey;
			rapidjson::Value jsonValue;
			int i;
			uint32_t size = 0;
			int32_t listLength = 1;

			jsonKey.SetString(iter->name.c_str(), iter->name.length(), jsonAllocator);


			if (iter->ptype & SerializableMemberInfo::PTYPE_LIST)
			{
				// List
				if (iter->mtype & SerializableMemberInfo::MTYPE_STDLIST)
				{
					_serializeStdList(jsonValue, jsonAllocator, *iter);
				}
			}
			else if (iter->ptype & SerializableMemberInfo::PTYPE_ARRAY)
			{
				// Single object
				if (iter->mtype & SerializableMemberInfo::MTYPE_STDVECTOR)
				{
					_serializeStdVector(jsonValue, jsonAllocator, *iter);
				}
				else if (iter->mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
				{
					switch (iter->ptype & 0xFFF)
					{
					case SerializableMemberInfo::PTYPE_CHAR:
						_serializeStdBasicString(jsonValue, jsonAllocator, *(std::basic_string<char>*)iter->ptr);
						break;
					case SerializableMemberInfo::PTYPE_WCHAR:
						_serializeStdBasicString(jsonValue, jsonAllocator, *(std::basic_string<wchar_t>*)iter->ptr);
						break;
					default:
						throw Serializable::UnavailableTypeException();
					}
				} else {
					_serializeNativeArray(jsonValue, jsonAllocator, *iter);
				}
			}
			else if (iter->ptype & SerializableMemberInfo::PTYPE_SUBPAYLOAD)
			{
				if (iter->mtype & SerializableMemberInfo::MTYPE_JSCPPUTILSMARTPOINTER)
				{
					JsCPPUtils::SmartPointer<Serializable> *pspobj = (JsCPPUtils::SmartPointer<Serializable> *)iter->ptr;
					_serializeSubPayload(jsonValue, jsonAllocator, pspobj->getPtr());
				} else {
					_serializeSubPayload(jsonValue, jsonAllocator, (Serializable*)iter->ptr);
				}
			}
			else
			{
				// Just object
				_serializeNativeObject(jsonValue, jsonAllocator, *iter);
			}

			jsonDoc.AddMember(jsonKey, jsonValue, jsonAllocator);
		}
	}

	/************************* Deserialize *************************/
	static void _deserializeStdBasicString(std::basic_string<char> *ptext, const rapidjson::Value &jsonValue)
	{
		if (jsonValue.IsNull())
			ptext->clear();
		else
			*ptext = std::basic_string<char>(jsonValue.GetString(), jsonValue.GetStringLength());
	}
	static void _deserializeStdBasicString(std::basic_string<wchar_t> *ptext, const rapidjson::Value &jsonValue)
	{
		if (jsonValue.IsNull())
			ptext->clear();
		else
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
			*ptext = JsCPPUtils::StringEncoding::UTF8ToString(jsonValue.GetString(), jsonValue.GetStringLength());
#else
			assert(0);
#endif
	}

	template<typename T>
	static void _deserializeStdListSmartPointerStdBasicString(const rapidjson::Value &jsonValue, const SerializableMemberInfo &memberInfo);

	template<>
	static void _deserializeStdListSmartPointerStdBasicString<char>(const rapidjson::Value &jsonValue, const SerializableMemberInfo &memberInfo)
	{
		std::list< JsCPPUtils::SmartPointer< std::basic_string<char> > > *plist = (std::list< JsCPPUtils::SmartPointer< std::basic_string<char> > > *)memberInfo.ptr;
		plist->clear();
		if (!jsonValue.IsNull())
		{
			for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
			{
				if (!iter->IsString())
				{
					throw JSONObjectMapper::TypeNotMatchException();
				}
				JsCPPUtils::SmartPointer< std::basic_string<char> > sptext = new std::basic_string<char>(jsonValue.GetString(), jsonValue.GetStringLength());
				plist->push_back(sptext);
			}
		}
	}

	template<>
	static void _deserializeStdListSmartPointerStdBasicString<wchar_t>(const rapidjson::Value &jsonValue, const SerializableMemberInfo &memberInfo)
	{
		std::list< JsCPPUtils::SmartPointer< std::basic_string<wchar_t> > > *plist = (std::list< JsCPPUtils::SmartPointer< std::basic_string<wchar_t> > > *)memberInfo.ptr;
		plist->clear();
		if (!jsonValue.IsNull())
		{
			for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
			{
				if (iter->IsNull()) {
					plist->push_back(NULL);
				}else if (!iter->IsString())
				{
					throw JSONObjectMapper::TypeNotMatchException();
				}
				else {
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
					std::basic_string<wchar_t> text = JsCPPUtils::StringEncoding::UTF8ToStringW(jsonValue.GetString(), jsonValue.GetStringLength());
					JsCPPUtils::SmartPointer< std::basic_string<wchar_t> > sptext = new std::basic_string<wchar_t>(text);
					plist->push_back(sptext);
#else
					assert(0);
#endif
				}
			}
		}
	}

	static Serializable *_deserializeSubPayloadToCreate(const SerializableMemberInfo &memberInfo, const rapidjson::Value &jsonValue)
	{
		Serializable *serializable = memberInfo.createFactory->create();
		JSONObjectMapper::deserializeJsonObject(serializable, jsonValue);
		return serializable;
	}

	static void _deserializeStdListSmartPointerSubPayload(const SerializableMemberInfo &memberInfo, const rapidjson::Value &jsonValue)
	{
		std::list< JsCPPUtils::SmartPointer< Serializable > > *plist = (std::list< JsCPPUtils::SmartPointer< Serializable > > *)memberInfo.ptr;
		plist->clear();
		for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
		{
			plist->push_back(_deserializeSubPayloadToCreate(memberInfo, *iter));
		}
	}

	template<typename T>
	static void _deserializeStdListStdBasicString(const SerializableMemberInfo &memberInfo, const rapidjson::Value &jsonValue)
	{
		std::list< std::basic_string<T> > *plist = (std::list< std::basic_string<T> >*)memberInfo.ptr;
		int32_t listSize = plist->size();
		int32_t i;
		for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
		{
			std::basic_string<T> text;
			_deserializeStdBasicString(&text, *iter);
			plist->push_back(text);
		}
	}

	static void _deserializeStdListSubPayload(const SerializableMemberInfo &memberInfo, const rapidjson::Value &jsonValue)
	{
		std::list< JsRPC::Serializable*> *plist = (std::list< JsRPC::Serializable*>*)memberInfo.ptr;
		for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
		{
			plist->push_back(_deserializeSubPayloadToCreate(memberInfo, jsonValue));
		}
	}

	void _deserializeStdList(const SerializableMemberInfo &memberInfo, const rapidjson::Value &jsonValue)
	{
		int32_t i;
		int32_t listSize;
		if (memberInfo.mtype & SerializableMemberInfo::MTYPE_JSCPPUTILSMARTPOINTER)
		{
			if (memberInfo.mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_CHAR:
					_deserializeStdListSmartPointerStdBasicString<char>(jsonValue, memberInfo);
					return;
				case SerializableMemberInfo::PTYPE_WCHAR:
					_deserializeStdListSmartPointerStdBasicString<wchar_t>(jsonValue, memberInfo);
					return;
				}
			}
			else
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_SUBPAYLOAD:
					_deserializeStdListSmartPointerSubPayload(memberInfo, jsonValue);
					return;
				}
			}
		}
		else
		{
			if (memberInfo.mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_CHAR:
					_deserializeStdListStdBasicString<char>(memberInfo, jsonValue);
					return;
				case SerializableMemberInfo::PTYPE_WCHAR:
					_deserializeStdListStdBasicString<wchar_t>(memberInfo, jsonValue);
					return;
				}
			}
			else
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_SUBPAYLOAD:
					_deserializeStdListSubPayload(memberInfo, jsonValue);
					return;
				}
			}
		}
		throw Serializable::UnavailableTypeException();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static void _deserializeStdVectorNativeBool(const rapidjson::Value &jsonValue, void *ptr)
	{
		std::vector<bool> *pvector = (std::vector<bool>*)ptr;
		pvector->clear();
		if (!jsonValue.IsNull()) {
			if (!jsonValue.IsArray())
				throw JSONObjectMapper::TypeNotMatchException();
			for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
			{
				if (!iter->IsBool())
					throw JSONObjectMapper::TypeNotMatchException();
				pvector->push_back(iter->GetBool());
			}
		}
	}

	template<typename T>
	static T _jsonGetByType(const rapidjson::Value &jsonValue);

	template<>
	static bool _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsBool())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetBool();
	}
	template<>
	static char _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsInt())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetInt();
	}
	template<>
	static wchar_t _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsInt())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetInt();
	}
	template<>
	static int8_t _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsInt())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetInt();
	}
	template<>
	static uint8_t _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsUint())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetUint();
	}
	template<>
	static int16_t _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsInt())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetInt();
	}
	template<>
	static uint16_t _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsUint())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetUint();
	}
	template<>
	static int32_t _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsInt())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetInt();
	}
	template<>
	static uint32_t _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsUint())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetUint();
	}
	template<>
	static int64_t _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsInt64())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetInt64();
	}
	template<>
	static uint64_t _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsUint64())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetUint64();
	}
	template<>
	static float _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsFloat())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetFloat();
	}
	template<>
	static double _jsonGetByType(const rapidjson::Value &jsonValue) {
		if (!jsonValue.IsDouble())
			throw JSONObjectMapper::TypeNotMatchException();
		return jsonValue.GetDouble();
	}

	template<typename T>
	static void _deserializeStdVectorNative(const rapidjson::Value &jsonValue, void *ptr)
	{
		std::vector<T> *pvector = (std::vector<T>*)ptr;
		pvector->clear();
		if (!jsonValue.IsNull()) {
			if (!jsonValue.IsArray())
				throw JSONObjectMapper::TypeNotMatchException();
			for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++)
			{
				pvector->push_back(_jsonGetByType<T>(*iter));
			}
		}
	}

	static void _deserializeStdVector(const SerializableMemberInfo &memberInfo, const rapidjson::Value &jsonValue)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			_deserializeStdVectorNativeBool(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
			_deserializeStdVectorNative<int8_t>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT8:
			_deserializeStdVectorNative<uint8_t>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT16:
			_deserializeStdVectorNative<int16_t>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT16:
			_deserializeStdVectorNative<uint16_t>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT32:
			_deserializeStdVectorNative<int32_t>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT32:
			_deserializeStdVectorNative<uint32_t>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT64:
			_deserializeStdVectorNative<int64_t>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT64:
			_deserializeStdVectorNative<uint64_t>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_FLOAT:
			_deserializeStdVectorNative<float>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_DOUBLE:
			_deserializeStdVectorNative<double>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_CHAR:
			_deserializeStdVectorNative<char>(jsonValue, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_WCHAR:
			_deserializeStdVectorNative<wchar_t>(jsonValue, memberInfo.ptr);
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	template<typename T>
	static void _deserializeNativeArrayT(const rapidjson::Value &jsonValue, const SerializableMemberInfo &memberInfo)
	{
		int32_t i = 0;
		T *pdata = (T*)memberInfo.ptr;
		if (!jsonValue.IsArray())
			throw JSONObjectMapper::TypeNotMatchException();

		if (memberInfo.length > jsonValue.Size())
			throw JSONObjectMapper::DataOverrflowException();

		for (rapidjson::Value::ConstValueIterator iter = jsonValue.Begin(); iter != jsonValue.End(); iter++, i++)
		{
			pdata[i] = _jsonGetByType<bool>(*iter);
		}
	}

	static void _deserializeNativeArray(const rapidjson::Value &jsonValue, const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			_deserializeNativeArrayT<bool>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
			_deserializeNativeArrayT<int8_t>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_UINT8:
			_deserializeNativeArrayT<uint8_t>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_SINT16:
			_deserializeNativeArrayT<int16_t>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_UINT16:
			_deserializeNativeArrayT<uint16_t>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_SINT32:
			_deserializeNativeArrayT<int32_t>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_UINT32:
			_deserializeNativeArrayT<uint32_t>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_SINT64:
			_deserializeNativeArrayT<int64_t>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_UINT64:
			_deserializeNativeArrayT<uint64_t>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_FLOAT:
			_deserializeNativeArrayT<float>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_DOUBLE:
			_deserializeNativeArrayT<double>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_CHAR:
			_deserializeNativeArrayT<char>(jsonValue, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_WCHAR:
			_deserializeNativeArrayT<wchar_t>(jsonValue, memberInfo);
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _deserializeNativeObject(const rapidjson::Value &jsonValue, const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			*((bool*)memberInfo.ptr) = _jsonGetByType<bool>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
			*((int8_t*)memberInfo.ptr) = _jsonGetByType<int8_t>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_UINT8:
			*((uint8_t*)memberInfo.ptr) = _jsonGetByType<uint8_t>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_SINT16:
			*((int16_t*)memberInfo.ptr) = _jsonGetByType<int16_t>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_UINT16:
			*((uint16_t*)memberInfo.ptr) = _jsonGetByType<uint16_t>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_SINT32:
			*((int32_t*)memberInfo.ptr) = _jsonGetByType<int32_t>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_UINT32:
			*((uint32_t*)memberInfo.ptr) = _jsonGetByType<uint32_t>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_SINT64:
			*((int64_t*)memberInfo.ptr) = _jsonGetByType<int64_t>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_UINT64:
			*((uint64_t*)memberInfo.ptr) = _jsonGetByType<uint64_t>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_FLOAT:
			*((float*)memberInfo.ptr) = _jsonGetByType<float>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_DOUBLE:
			*((double*)memberInfo.ptr) = _jsonGetByType<double>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_CHAR:
			*((char*)memberInfo.ptr) = _jsonGetByType<char>(jsonValue);
			return;
		case SerializableMemberInfo::PTYPE_WCHAR:
			*((wchar_t*)memberInfo.ptr) = _jsonGetByType<wchar_t>(jsonValue);
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void JSONObjectMapper::deserializeJsonObject(Serializable *serialiable, const rapidjson::Value &jsonObject)
	{
		size_t pos = 0;

		const std::list<SerializableMemberInfo> &members = serialiable->serializableMembers();

		if (!jsonObject.IsObject())
		{
			throw TypeNotMatchException();
		}

		// Data
		for (std::list<SerializableMemberInfo>::const_iterator iter = members.begin(); iter != members.end(); iter++)
		{
			if (jsonObject.HasMember(iter->name.c_str()))
			{
				const rapidjson::Value &jsonMember = jsonObject[iter->name.c_str()];

				if (iter->ptype & SerializableMemberInfo::PTYPE_LIST)
				{
					// List
					if (!jsonMember.IsArray())
					{
						throw TypeNotMatchException();
					}
					if (iter->mtype & SerializableMemberInfo::MTYPE_STDLIST)
					{
						_deserializeStdList(*iter, jsonMember);
					}
				}
				else if (iter->ptype & SerializableMemberInfo::PTYPE_ARRAY)
				{
					// Single object
					if (iter->mtype & SerializableMemberInfo::MTYPE_STDVECTOR)
					{
						_deserializeStdVector(*iter, jsonMember);
					}
					else if (iter->mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
					{
						switch (iter->ptype & 0xFFF)
						{
						case SerializableMemberInfo::PTYPE_CHAR:
							_deserializeStdBasicString((std::basic_string<char>*)iter->ptr, jsonMember);
							break;
						case SerializableMemberInfo::PTYPE_WCHAR:
							_deserializeStdBasicString((std::basic_string<wchar_t>*)iter->ptr, jsonMember);
							break;
						default:
							throw Serializable::UnavailableTypeException();
						}
					}
					else {
						_deserializeNativeArray(jsonMember, *iter);
					}
				}
				else if (iter->ptype & SerializableMemberInfo::PTYPE_SUBPAYLOAD)
				{
					if (iter->mtype & SerializableMemberInfo::MTYPE_JSCPPUTILSMARTPOINTER)
					{
						JsCPPUtils::SmartPointer<Serializable> *pspobj = (JsCPPUtils::SmartPointer<Serializable> *)iter->ptr;
						if (jsonMember.IsNull())
							*pspobj = NULL;
						else
							JSONObjectMapper::deserializeJsonObject(pspobj->getPtr(), jsonMember);
					} else {
						Serializable *pobj = (Serializable *)iter->ptr;
						if (jsonMember.IsNull())
							Serializable::clearObject(pobj);
						else
							JSONObjectMapper::deserializeJsonObject(pobj, jsonMember);
					}
				}
				else
				{
					// Just object
					_deserializeNativeObject(jsonMember, *iter);
				}
			}
		}
	}
#endif
}
