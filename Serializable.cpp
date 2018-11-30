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

	static void writeToPayload(std::vector<unsigned char>& payload, const void *ptr, size_t length) {
		size_t pos = payload.size();
		payload.resize(pos + length, 0);
		memcpy(&payload[pos], ptr, length);
	}
	static void writeToPayload(std::vector<unsigned char>& payload, char data) {
		payload.push_back((unsigned char)data);
	}
	static void writeToPayload(std::vector<unsigned char>& payload, wchar_t data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, int8_t data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, uint8_t data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, int16_t data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, uint16_t data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, int32_t data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, uint32_t data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, int64_t data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, uint64_t data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, float data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, double data) {
		writeToPayload(payload, &data, sizeof(data));
	}
	static void writeToPayload(std::vector<unsigned char>& payload, const std::basic_string<char> &data) {
		writeToPayload(payload, data.c_str(), sizeof(char) * data.length());
	}
	static void writeToPayload(std::vector<unsigned char>& payload, const std::basic_string<wchar_t> &data) {
		writeToPayload(payload, data.c_str(), sizeof(wchar_t) * data.length());
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

	template<typename T>
	static void _serializeStdBasicString(std::vector<unsigned char> &payload, const std::basic_string<T> &text)
	{
		uint32_t pos;
		uint32_t length = (uint32_t)text.length();
		uint32_t datasize = sizeof(T) * length;
		payload.push_back((unsigned char)(length >> 0));
		payload.push_back((unsigned char)(length >> 8));
		payload.push_back((unsigned char)(length >> 16));
		payload.push_back((unsigned char)(length >> 24));
		if (text.length() > 0)
		{
			pos = payload.size();
			payload.resize(pos + datasize, 0);
			memcpy(&payload[pos], &text[0], datasize);
		}
	}

	template<typename T>
	static void _deserializeStdBasicString(const std::vector<unsigned char> &payload, uint32_t *pos, std::basic_string<T> &text)
	{
		uint32_t length = readFromPayload<uint32_t>(payload, pos);
		uint32_t datasize = sizeof(T) * length;
		if(length > 0)
		{
			const T *textPtr = (const T *)&payload[*pos];
			text.clear();
			text = std::basic_string<T>(&textPtr[0], length);
			*pos += datasize;
		}
	}

	static void _serializeSubPayload(std::vector<unsigned char> &payload, Serializable *ptr)
	{
		if (ptr == NULL)
		{
			writeToPayload(payload, (uint32_t)0);
		}
		else {
			std::vector<unsigned char> subpayload;
			ptr->serialize(subpayload);
			writeToPayload(payload, (uint32_t)subpayload.size());
			if (subpayload.size() > 0)
				writeToPayload(payload, &subpayload[0], subpayload.size());
		}
	}

	static bool _deserializeSubPayload(const std::vector<unsigned char> &payload, uint32_t *pos, Serializable *ptr)
	{
		uint32_t datasize = readFromPayload<uint32_t>(payload, pos);
		if (!datasize)
		{
			return false;
		} else {
			std::vector<unsigned char> subPayload(payload[*pos], payload[*pos + datasize]);
			ptr->deserialize(subPayload);
			*pos += datasize;
			return true;
		}
	}

#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
	template<typename T>
	static void _serializeStdListSmartPointerStdBasicString(std::vector<unsigned char> &payload, const void *ptr)
	{
		const std::list< JsCPPUtils::SmartPointer< std::basic_string<T> > > *plist = (const std::list< JsCPPUtils::SmartPointer< std::basic_string<T> > > *)ptr;
		int32_t listSize = plist->size();
		int32_t i;
		writeToPayload(payload, listSize);
		for(std::list< JsCPPUtils::SmartPointer< std::basic_string<T> > >::const_iterator iter = plist->begin(); iter != plist->end(); iter++)
		{
			_serializeStdBasicString(payload, *(*iter));
		}
	}

	template<typename T>
	static void _deserializeStdListSmartPointerStdBasicString(const std::vector<unsigned char> &payload, uint32_t *pos, SerializableMemberInfo &memberInfo)
	{
		const std::list< JsCPPUtils::SmartPointer< std::basic_string<T> > > *plist = (const std::list< JsCPPUtils::SmartPointer< std::basic_string<T> > > *)memberInfo.ptr;
		int32_t listSize = plist->size();
		int32_t i;
		uint32_t listcount = readFromPayload<uint32_t>(payload, pos);
		for (i = 0; i < listcount; i++)
		{
			JsCPPUtils::SmartPointer< std::basic_string<T> > spItem = new std::basic_string<T>();
			_deserializeStdBasicString(payload, pos, *spItem);
		}
	}

	static void _serializeStdListSmartPointerSubPayload(std::vector<unsigned char> &payload, const void *ptr)
	{
		const std::list< JsCPPUtils::SmartPointer< Serializable > > *plist = (const std::list< JsCPPUtils::SmartPointer< Serializable > > *)ptr;
		int32_t listSize = plist->size();
		int32_t i;
		writeToPayload(payload, listSize);
		for (std::list< JsCPPUtils::SmartPointer< Serializable > >::const_iterator iter = plist->begin(); iter != plist->end(); iter++)
		{
			_serializeSubPayload(payload, iter->getPtr());
		}
	}
	
	static void _deserializeStdListSmartPointerSubPayload(const std::vector<unsigned char> &payload, uint32_t *pos, SerializableMemberInfo &memberInfo)
	{
		std::list< JsCPPUtils::SmartPointer< Serializable > > *plist = (std::list< JsCPPUtils::SmartPointer< Serializable > > *)memberInfo.ptr;
		int32_t listSize = readFromPayload<uint32_t>(payload, pos);
		int32_t i;
		plist->clear();
		for (i = 0; i < listSize; i++)
		{
			JsCPPUtils::SmartPointer< Serializable > object = memberInfo.createFactory->create();
			_deserializeSubPayload(payload, pos, object.getPtr());
			plist->push_back(object);
		}
	}
#endif
	
	template<typename T>
	static void _serializeStdListStdBasicString(std::vector<unsigned char> &payload, const void *ptr)
	{
		const std::list< std::basic_string<T> > *plist = (const std::list< std::basic_string<T> > *)ptr;
		int32_t listSize = plist->size();
		int32_t i;
		writeToPayload(payload, listSize);
		for (std::list< std::basic_string<T> >::const_iterator iter = plist->begin(); iter != plist->end(); iter++)
		{
			_serializeStdBasicString(payload, (*iter));
		}
	}

	template<typename T>
	static void _deserializeStdListStdBasicString(const std::vector<unsigned char> &payload, uint32_t *pos, const void *ptr)
	{
		std::list< std::basic_string<T> > *plist = (std::list< std::basic_string<T> > *)ptr;
		int32_t listSize = readFromPayload<uint32_t>(payload, pos);
		int32_t i;
		plist->clear();
		for (i = 0; i < listSize; i++)
		{
			std::basic_string<T> text;
			_deserializeStdBasicString(payload, pos, text);
			plist->push_back(text);
		}
	}

	static void _serializeStdListSubPayload(std::vector<unsigned char> &payload, std::list< JsRPC::Serializable*> *plist)
	{
		for (std::list< JsRPC::Serializable*>::iterator iter = plist->begin(); iter != plist->end(); iter++)
		{
			if (*iter == NULL)
			{
				writeToPayload(payload, (uint32_t)0);
			} else {
				std::vector<unsigned char> subpayload;
				(*iter)->serialize(subpayload);
				writeToPayload(payload, (uint32_t)subpayload.size());
				if (subpayload.size() > 0)
					writeToPayload(payload, &subpayload[0], subpayload.size());
			}
		}
	}

	static void _deserializeStdListSubPayload(const std::vector<unsigned char> &payload, uint32_t *pos, SerializableMemberInfo &memberInfo)
	{
		std::list< JsRPC::Serializable*> *plist = (std::list< JsRPC::Serializable*> *)memberInfo.ptr;
		int32_t listSize = readFromPayload<uint32_t>(payload, pos);
		int32_t i;
		plist->clear();
		for (i = 0; i < listSize; i++)
		{
			JsRPC::Serializable *serializable = memberInfo.createFactory->create();
			uint32_t subpayloadSize = readFromPayload<uint32_t>(payload, pos);
			if (subpayloadSize > 0)
			{
				std::vector<unsigned char> subpayload(subpayloadSize, 0);
				readFromPayload(payload, pos, &subpayload[0], subpayloadSize);
				serializable->deserialize(subpayload);
				plist->push_back(serializable);
			} else {
				plist->push_back(NULL);
			}
		}
	}

	static void _serializeStdList(std::vector<unsigned char> &payload, const SerializableMemberInfo &memberInfo)
	{
		int32_t i;
		int32_t listSize;
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
		if (memberInfo.mtype & SerializableMemberInfo::MTYPE_JSCPPUTILSMARTPOINTER)
		{
			if (memberInfo.mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_CHAR:
					_serializeStdListSmartPointerStdBasicString<char>(payload, memberInfo.ptr);
					return;
				case SerializableMemberInfo::PTYPE_WCHAR:
					_serializeStdListSmartPointerStdBasicString<wchar_t>(payload, memberInfo.ptr);
					return;
				}
			}
			else
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_SUBPAYLOAD:
					_serializeStdListSmartPointerSubPayload(payload, memberInfo.ptr);
					return;
				}
			}
		}
		else
#endif
		{
			if (memberInfo.mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_CHAR:
					_serializeStdListStdBasicString<char>(payload, memberInfo.ptr);
					return;
				case SerializableMemberInfo::PTYPE_WCHAR:
					_serializeStdListStdBasicString<wchar_t>(payload, memberInfo.ptr);
					return;
				}
			}
			else
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_SUBPAYLOAD:
					_serializeStdListSubPayload(payload, (std::list<JsRPC::Serializable*>*)memberInfo.ptr);
					return;
				}
			}
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _deserializeStdList(const std::vector<unsigned char> &payload, uint32_t *pos, SerializableMemberInfo &memberInfo)
	{
		int32_t i;
		int32_t listSize;
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
		if (memberInfo.mtype & SerializableMemberInfo::MTYPE_JSCPPUTILSMARTPOINTER)
		{
			if (memberInfo.mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_CHAR:
					_deserializeStdListSmartPointerStdBasicString<char>(payload, pos, memberInfo);
					return;
				case SerializableMemberInfo::PTYPE_WCHAR:
					_deserializeStdListSmartPointerStdBasicString<wchar_t>(payload, pos, memberInfo);
					return;
				}
			}
			else
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_SUBPAYLOAD:
					_deserializeStdListSmartPointerSubPayload(payload, pos, memberInfo);
					return;
				}
			}
		}
		else
#endif
		{
			if (memberInfo.mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_CHAR:
					_deserializeStdListStdBasicString<char>(payload, pos, memberInfo.ptr);
					return;
				case SerializableMemberInfo::PTYPE_WCHAR:
					_deserializeStdListStdBasicString<wchar_t>(payload, pos, memberInfo.ptr);
					return;
				}
			}
			else
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_SUBPAYLOAD:
					_deserializeStdListSubPayload(payload, pos, memberInfo);
					return;
				}
			}
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _clearStdList(const SerializableMemberInfo &memberInfo)
	{
		int32_t i;
		int32_t listSize;
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
		if (memberInfo.mtype & SerializableMemberInfo::MTYPE_JSCPPUTILSMARTPOINTER)
		{
			if (memberInfo.mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_CHAR:
					((std::list< JsCPPUtils::SmartPointer< std::basic_string<char> > > *)memberInfo.ptr)->clear();
					return;
				case SerializableMemberInfo::PTYPE_WCHAR:
					((std::list< JsCPPUtils::SmartPointer< std::basic_string<wchar_t> > > *)memberInfo.ptr)->clear();
					return;
				}
			}
			else
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_SUBPAYLOAD:
					((std::list< JsCPPUtils::SmartPointer< Serializable > > *)memberInfo.ptr)->clear();
					return;
				}
			}
		}
		else
#endif
		{
			if (memberInfo.mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_CHAR:
					((std::list< std::basic_string<char> > *)memberInfo.ptr)->clear();
					return;
				case SerializableMemberInfo::PTYPE_WCHAR:
					((std::list< std::basic_string<wchar_t> > *)memberInfo.ptr)->clear();
					return;
				}
			}
			else
			{
				switch (memberInfo.ptype & 0xFFF)
				{
				case SerializableMemberInfo::PTYPE_SUBPAYLOAD:
					((std::list<JsRPC::Serializable*>*)memberInfo.ptr)->clear();
					return;
				}
			}
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _serializeStdVectorNativeBool(std::vector<unsigned char> &payload, void *ptr)
	{
		const std::vector<bool> *pvector = (const std::vector<bool>*)ptr;
		writeToPayload(payload, (uint32_t)pvector->size());
		if (pvector->size())
		{
			for (std::vector<bool>::const_iterator iter = pvector->begin(); iter != pvector->end(); iter++)
				payload.push_back(*iter ? 1 : 0);
		}
	}

	static void _deserializeStdVectorNativeBool(const std::vector<unsigned char> &payload, uint32_t *pos, void *ptr)
	{
		std::vector<bool> *pvector = (std::vector<bool>*)ptr;
		uint32_t vectorSize = readFromPayload<uint32_t>(payload, pos);
		uint32_t i;
		pvector->clear();
		for (i = 0; i < vectorSize; i++)
		{
			pvector->push_back(readFromPayload<char>(payload, pos) ? true : false);
		}
	}

	template<typename T>
	static void _serializeStdVectorNative(std::vector<unsigned char> &payload, void *ptr)
	{
		const std::vector<T> *pvector = (const std::vector<T>*)ptr;
		size_t ds = pvector->size() * sizeof(T);
		writeToPayload(payload, (uint32_t)pvector->size());
		if (pvector->size())
		{
			writeToPayload(payload, &(*pvector)[0], ds);
		}
	}

	template<typename T>
	static void _deserializeStdVectorNative(const std::vector<unsigned char> &payload, uint32_t *pos, void *ptr)
	{
		std::vector<T> *pvector = (std::vector<T>*)ptr;
		uint32_t vectorSize = readFromPayload<uint32_t>(payload, pos);
		uint32_t i;
		pvector->clear();
		if (vectorSize > 0)
		{
			pvector->assign(vectorSize, 0);
			readFromPayload(payload, pos, (void*)&(*pvector)[0], vectorSize);
		}
	}

	template<typename T>
	static void _clearStdVectorNative(void *ptr)
	{
		std::vector<T> *pvector = (std::vector<T>*)ptr;
		pvector->clear();
	}

	static void _serializeStdVector(std::vector<unsigned char> &payload, const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			_serializeStdVectorNativeBool(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
			_serializeStdVectorNative<int8_t>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT8:
			_serializeStdVectorNative<uint8_t>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT16:
			_serializeStdVectorNative<int16_t>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT16:
			_serializeStdVectorNative<uint16_t>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT32:
			_serializeStdVectorNative<int32_t>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT32:
			_serializeStdVectorNative<uint32_t>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT64:
			_serializeStdVectorNative<int64_t>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT64:
			_serializeStdVectorNative<uint64_t>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_FLOAT:
			_serializeStdVectorNative<float>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_DOUBLE:
			_serializeStdVectorNative<double>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_CHAR:
			_serializeStdVectorNative<char>(payload, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_WCHAR:
			_serializeStdVectorNative<wchar_t>(payload, memberInfo.ptr);
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _deserializeStdVector(const std::vector<unsigned char> &payload, uint32_t *pos, const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			_deserializeStdVectorNativeBool(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
			_deserializeStdVectorNative<int8_t>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT8:
			_deserializeStdVectorNative<uint8_t>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT16:
			_deserializeStdVectorNative<int16_t>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT16:
			_deserializeStdVectorNative<uint16_t>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT32:
			_deserializeStdVectorNative<int32_t>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT32:
			_deserializeStdVectorNative<uint32_t>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT64:
			_deserializeStdVectorNative<int64_t>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT64:
			_deserializeStdVectorNative<uint64_t>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_FLOAT:
			_deserializeStdVectorNative<float>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_DOUBLE:
			_deserializeStdVectorNative<double>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_CHAR:
			_deserializeStdVectorNative<char>(payload, pos, memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_WCHAR:
			_deserializeStdVectorNative<wchar_t>(payload, pos, memberInfo.ptr);
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _clearStdVector(const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			_clearStdVectorNative<bool>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
			_clearStdVectorNative<int8_t>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT8:
			_clearStdVectorNative<uint8_t>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT16:
			_clearStdVectorNative<int16_t>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT16:
			_clearStdVectorNative<uint16_t>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT32:
			_clearStdVectorNative<int32_t>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT32:
			_clearStdVectorNative<uint32_t>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_SINT64:
			_clearStdVectorNative<int64_t>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_UINT64:
			_clearStdVectorNative<uint64_t>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_FLOAT:
			_clearStdVectorNative<float>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_DOUBLE:
			_clearStdVectorNative<double>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_CHAR:
			_clearStdVectorNative<char>(memberInfo.ptr);
			return;
		case SerializableMemberInfo::PTYPE_WCHAR:
			_clearStdVectorNative<wchar_t>(memberInfo.ptr);
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _serializeNativeArrayBool(std::vector<unsigned char> &payload, const SerializableMemberInfo &memberInfo)
	{
		int32_t i;
		const bool *pdata = (const bool*)memberInfo.ptr;
		writeToPayload(payload, (uint32_t)memberInfo.length);
		for (i = 0; i < memberInfo.length; i++)
		{
			payload.push_back(pdata[i] ? 1 : 0);
		}
	}

	static void _deserializeNativeArrayBool(const std::vector<unsigned char> &payload, uint32_t *pos, const SerializableMemberInfo &memberInfo)
	{
		int32_t i;
		bool *pdata = (bool*)memberInfo.ptr;
		uint32_t vectorSize = readFromPayload<uint32_t>(payload, pos);
		if (memberInfo.length < vectorSize)
			throw Serializable::ParseException();
		for (i = 0; i < vectorSize; i++)
		{
			pdata[i] = readFromPayload<char>(payload, pos) ? true : false;
		}
	}

	static void _serializeNativeArray(std::vector<unsigned char> &payload, const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			_serializeNativeArrayBool(payload, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
		case SerializableMemberInfo::PTYPE_UINT8:
		case SerializableMemberInfo::PTYPE_SINT16:
		case SerializableMemberInfo::PTYPE_UINT16:
		case SerializableMemberInfo::PTYPE_SINT32:
		case SerializableMemberInfo::PTYPE_UINT32:
		case SerializableMemberInfo::PTYPE_SINT64:
		case SerializableMemberInfo::PTYPE_UINT64:
		case SerializableMemberInfo::PTYPE_FLOAT:
		case SerializableMemberInfo::PTYPE_DOUBLE:
		case SerializableMemberInfo::PTYPE_CHAR:
		case SerializableMemberInfo::PTYPE_WCHAR:
			writeToPayload(payload, (uint32_t)memberInfo.length);
			writeToPayload(payload, memberInfo.ptr, memberInfo.length * (memberInfo.ptype & 0xF));
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _deserializeNativeArray(const std::vector<unsigned char> &payload, uint32_t *pos, const SerializableMemberInfo &memberInfo)
	{
		uint32_t arrSize;
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			_deserializeNativeArrayBool(payload, pos, memberInfo);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
		case SerializableMemberInfo::PTYPE_UINT8:
		case SerializableMemberInfo::PTYPE_SINT16:
		case SerializableMemberInfo::PTYPE_UINT16:
		case SerializableMemberInfo::PTYPE_SINT32:
		case SerializableMemberInfo::PTYPE_UINT32:
		case SerializableMemberInfo::PTYPE_SINT64:
		case SerializableMemberInfo::PTYPE_UINT64:
		case SerializableMemberInfo::PTYPE_FLOAT:
		case SerializableMemberInfo::PTYPE_DOUBLE:
		case SerializableMemberInfo::PTYPE_CHAR:
		case SerializableMemberInfo::PTYPE_WCHAR:
			arrSize = readFromPayload<uint32_t>(payload, pos);
			readFromPayload(payload, pos, memberInfo.ptr, (uint32_t)(arrSize * (memberInfo.ptype & 0xF)));
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _clearNativeArray(const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			memset((void*)memberInfo.ptr, 0, sizeof(bool) * memberInfo.length);
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
		case SerializableMemberInfo::PTYPE_UINT8:
		case SerializableMemberInfo::PTYPE_SINT16:
		case SerializableMemberInfo::PTYPE_UINT16:
		case SerializableMemberInfo::PTYPE_SINT32:
		case SerializableMemberInfo::PTYPE_UINT32:
		case SerializableMemberInfo::PTYPE_SINT64:
		case SerializableMemberInfo::PTYPE_UINT64:
		case SerializableMemberInfo::PTYPE_FLOAT:
		case SerializableMemberInfo::PTYPE_DOUBLE:
		case SerializableMemberInfo::PTYPE_CHAR:
		case SerializableMemberInfo::PTYPE_WCHAR:
			memset((void*)memberInfo.ptr, 0, (memberInfo.ptype & 0xF) * memberInfo.length);
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _serializeNativeObject(std::vector<unsigned char> &payload, const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			writeToPayload(payload, (uint8_t)((*((bool*)memberInfo.ptr)) ? 1 : 0));
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
		case SerializableMemberInfo::PTYPE_UINT8:
		case SerializableMemberInfo::PTYPE_SINT16:
		case SerializableMemberInfo::PTYPE_UINT16:
		case SerializableMemberInfo::PTYPE_SINT32:
		case SerializableMemberInfo::PTYPE_UINT32:
		case SerializableMemberInfo::PTYPE_SINT64:
		case SerializableMemberInfo::PTYPE_UINT64:
		case SerializableMemberInfo::PTYPE_FLOAT:
		case SerializableMemberInfo::PTYPE_DOUBLE:
		case SerializableMemberInfo::PTYPE_CHAR:
		case SerializableMemberInfo::PTYPE_WCHAR:
			writeToPayload(payload, memberInfo.ptr, (memberInfo.ptype & 0xF));
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _deserializeNativeObject(const std::vector<unsigned char> &payload, uint32_t *pos, const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			*((bool*)memberInfo.ptr) = readFromPayload<char>(payload, pos) ? true : false;
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
		case SerializableMemberInfo::PTYPE_UINT8:
		case SerializableMemberInfo::PTYPE_SINT16:
		case SerializableMemberInfo::PTYPE_UINT16:
		case SerializableMemberInfo::PTYPE_SINT32:
		case SerializableMemberInfo::PTYPE_UINT32:
		case SerializableMemberInfo::PTYPE_SINT64:
		case SerializableMemberInfo::PTYPE_UINT64:
		case SerializableMemberInfo::PTYPE_FLOAT:
		case SerializableMemberInfo::PTYPE_DOUBLE:
		case SerializableMemberInfo::PTYPE_CHAR:
		case SerializableMemberInfo::PTYPE_WCHAR:
			readFromPayload(payload, pos, memberInfo.ptr, (memberInfo.ptype & 0xF));
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	static void _clearNativeObject(const SerializableMemberInfo &memberInfo)
	{
		switch (memberInfo.ptype & 0xFFF)
		{
		case SerializableMemberInfo::PTYPE_BOOL:
			*((bool*)memberInfo.ptr) = 0;
			return;
		case SerializableMemberInfo::PTYPE_SINT8:
		case SerializableMemberInfo::PTYPE_UINT8:
		case SerializableMemberInfo::PTYPE_SINT16:
		case SerializableMemberInfo::PTYPE_UINT16:
		case SerializableMemberInfo::PTYPE_SINT32:
		case SerializableMemberInfo::PTYPE_UINT32:
		case SerializableMemberInfo::PTYPE_SINT64:
		case SerializableMemberInfo::PTYPE_UINT64:
		case SerializableMemberInfo::PTYPE_FLOAT:
		case SerializableMemberInfo::PTYPE_DOUBLE:
		case SerializableMemberInfo::PTYPE_CHAR:
		case SerializableMemberInfo::PTYPE_WCHAR:
			memset((void*)memberInfo.ptr, 0, (memberInfo.ptype & 0xF));
			return;
		}
		throw Serializable::UnavailableTypeException();
	}

	void Serializable::serialize(std::vector<unsigned char>& payload) throw(UnavailableTypeException)
	{
		size_t pos = 0;

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
		for (std::list<SerializableMemberInfo>::const_iterator iter = m_members.begin(); iter != m_members.end(); iter++)
		{
			int i;
			uint32_t size = 0;
			int32_t listLength = 1;
			payload.push_back((unsigned char)(iter->ptype >> 0)); // PType
			payload.push_back((unsigned char)(iter->ptype >> 8)); // PType
			if (iter->ptype & SerializableMemberInfo::PTYPE_LIST)
			{
				// List
				if (iter->mtype & SerializableMemberInfo::MTYPE_STDLIST)
				{
					_serializeStdList(payload, *iter);
				}
			}
			else if (iter->ptype & SerializableMemberInfo::PTYPE_ARRAY)
			{
				// Single object
				if (iter->mtype & SerializableMemberInfo::MTYPE_STDVECTOR)
				{
					_serializeStdVector(payload, *iter);
				}else if (iter->mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
				{
					switch (iter->ptype & 0xFFF)
					{
					case SerializableMemberInfo::PTYPE_CHAR:
						_serializeStdBasicString(payload, *(std::basic_string<char>*)iter->ptr);
						break;
					case SerializableMemberInfo::PTYPE_WCHAR:
						_serializeStdBasicString(payload, *(std::basic_string<wchar_t>*)iter->ptr);
						break;
					default:
						throw UnavailableTypeException();
					}
				}else{
					_serializeNativeArray(payload, *iter);
				}
			}
			else
			{
				// Just object
				_serializeNativeObject(payload, *iter);
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

		std::list<SerializableMemberInfo>::iterator memberInfoIter = m_members.begin();

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
		while (remainsize > 0 && memberInfoIter != m_members.end())
		{
			SerializableMemberInfo::ProtoType ptype = (SerializableMemberInfo::ProtoType)readFromPayload<uint16_t>(payload, &pos);
			if (ptype & SerializableMemberInfo::PTYPE_LIST)
			{
				// List
				if (memberInfoIter->mtype & SerializableMemberInfo::MTYPE_STDLIST)
				{
					_deserializeStdList(payload, &pos, *memberInfoIter);
				}
			}
			else if (ptype & SerializableMemberInfo::PTYPE_ARRAY)
			{
				// Single object
				if (memberInfoIter->mtype & SerializableMemberInfo::MTYPE_STDVECTOR)
				{
					_deserializeStdVector(payload, &pos, *memberInfoIter);
				}
				else if (memberInfoIter->mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
				{
					switch (memberInfoIter->ptype & 0xFFF)
					{
					case SerializableMemberInfo::PTYPE_CHAR:
						_deserializeStdBasicString(payload, &pos, *(std::basic_string<char>*)memberInfoIter->ptr);
						break;
					case SerializableMemberInfo::PTYPE_WCHAR:
						_deserializeStdBasicString(payload, &pos, *(std::basic_string<wchar_t>*)memberInfoIter->ptr);
						break;
					default:
						throw UnavailableTypeException();
					}
				} else {
					_deserializeNativeArray(payload, &pos, *memberInfoIter);
				}
			}
			else
			{
				// Just object
				_deserializeNativeObject(payload, &pos, *memberInfoIter);
			}
			remainsize = totalsize - pos;
			memberInfoIter++;
		}
		if (remainsize != 0)
			throw ParseException();
	}

	void Serializable::clearObject(Serializable *serializable)
	{
		const std::list<SerializableMemberInfo> &serializableMembers = serializable->serializableMembers();
		for (std::list<SerializableMemberInfo>::const_iterator iter = serializableMembers.begin(); iter != serializableMembers.end(); iter++)
		{
			int i;
			uint32_t size = 0;
			int32_t listLength = 1;
			if (iter->ptype & SerializableMemberInfo::PTYPE_LIST)
			{
				// List
				if (iter->mtype & SerializableMemberInfo::MTYPE_STDLIST)
				{
					_clearStdList(*iter);
				}
			}
			else if (iter->ptype & SerializableMemberInfo::PTYPE_ARRAY)
			{
				// Single object
				if (iter->mtype & SerializableMemberInfo::MTYPE_STDVECTOR)
				{
					_clearStdVector(*iter);
				}else if (iter->mtype & SerializableMemberInfo::MTYPE_STDBASICSTRING)
				{
					switch (iter->ptype & 0xFFF)
					{
					case SerializableMemberInfo::PTYPE_CHAR:
						((std::basic_string<char>*)iter->ptr)->clear();
						break;
					case SerializableMemberInfo::PTYPE_WCHAR:
						((std::basic_string<wchar_t>*)iter->ptr)->clear();
						break;
					default:
						throw UnavailableTypeException();
					}
				}else{
					_clearNativeArray(*iter);
				}
			}
			else
			{
				// Just object
				_clearNativeObject(*iter);
			}
		}
	}

	void Serializable::serializableMapMember(const char *name, bool &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_BOOL, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, char &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_CHAR, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, wchar_t &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_WCHAR, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, int8_t &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_SINT8, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, uint8_t &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_UINT8, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, int16_t &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_SINT16, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, uint16_t &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_UINT16, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, int32_t &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_SINT32, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, uint32_t &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_UINT32, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, int64_t &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_SINT64, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, uint64_t &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_UINT64, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, float &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_FLOAT, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, double &object){ m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_DOUBLE, SerializableMemberInfo::MTYPE_NATIVE, (void*)&object, 1)); }
	void Serializable::serializableMapMember(const char *name, Serializable *object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_SUBPAYLOAD), SerializableMemberInfo::MTYPE_NATIVE, (void*)object, 1)); }
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
	SerializableMemberInfo & Serializable::serializableMapMember(const char *name, JsCPPUtils::SmartPointer<Serializable> *object) { m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_SUBPAYLOAD, (SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_JSCPPUTILSMARTPOINTER), (void*)&object, 0)); return *m_members.rbegin(); }
#endif

	void Serializable::serializableMapMemberArray(const char *name, std::basic_string<char> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_CHAR), SerializableMemberInfo::MTYPE_STDBASICSTRING, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::basic_string<wchar_t> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_WCHAR), SerializableMemberInfo::MTYPE_STDBASICSTRING, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<char> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_CHAR), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<wchar_t> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_WCHAR), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<int8_t> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_SINT8), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<uint8_t> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_UINT8), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<int16_t> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_SINT16), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<uint16_t> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_UINT16), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<int32_t> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_SINT32), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<uint32_t> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_UINT32), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<int64_t> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_SINT64), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<uint64_t> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_UINT64), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<float> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_FLOAT), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }
	void Serializable::serializableMapMemberArray(const char *name, std::vector<double> &object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_DOUBLE), SerializableMemberInfo::MTYPE_STDVECTOR, (void*)&object, 1)); }

	void Serializable::serializableMapMemberArray(const char *name, bool *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_BOOL ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, char *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_CHAR ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, wchar_t *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_WCHAR ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, int8_t *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_SINT8 ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, uint8_t *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_UINT8 ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, int16_t *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_SINT16 ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, uint16_t *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_UINT16 ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, int32_t *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_SINT32 ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, uint32_t *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_UINT32 ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, int64_t *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_SINT64 ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, uint64_t *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_UINT64 ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, float *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_FLOAT ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	void Serializable::serializableMapMemberArray(const char *name, double *object, int32_t arrlength) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_ARRAY | SerializableMemberInfo::PTYPE_DOUBLE ), SerializableMemberInfo::MTYPE_NATIVE | SerializableMemberInfo::MTYPE_NATIVEARRAY, (void*)object, arrlength)); }
	
	void Serializable::serializableMapMemberList(const char *name, std::list< std::basic_string<char> >& object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_LIST | SerializableMemberInfo::PTYPE_CHAR), (SerializableMemberInfo::MTYPE_STDLIST | SerializableMemberInfo::MTYPE_STDBASICSTRING), (void*)&object, 1)); }
	void Serializable::serializableMapMemberList(const char *name, std::list< std::basic_string<wchar_t> >& object) { m_members.push_back(SerializableMemberInfo(name, (SerializableMemberInfo::PTYPE_LIST | SerializableMemberInfo::PTYPE_WCHAR), (SerializableMemberInfo::MTYPE_STDLIST | SerializableMemberInfo::MTYPE_STDBASICSTRING), (void*)&object, 1)); }

	SerializableMemberInfo & Serializable::serializableMapMemberList(const char *name, std::list<Serializable*>& object) { m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_LIST | SerializableMemberInfo::PTYPE_SUBPAYLOAD, SerializableMemberInfo::MTYPE_STDLIST, (void*)&object, 1)); return *m_members.rbegin(); }
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
	SerializableMemberInfo & Serializable::serializableMapMemberList(const char *name, std::list<JsCPPUtils::SmartPointer<Serializable> >& object) { m_members.push_back(SerializableMemberInfo(name, SerializableMemberInfo::PTYPE_LIST | SerializableMemberInfo::PTYPE_SUBPAYLOAD, SerializableMemberInfo::MTYPE_STDLIST | SerializableMemberInfo::MTYPE_JSCPPUTILSMARTPOINTER, (void*)&object, 1)); return *m_members.rbegin(); }
#endif
}
