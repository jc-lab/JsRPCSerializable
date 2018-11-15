#pragma once

#include <stdint.h>
#include <wchar.h>
#include <string>
#include <list>
#include <vector>
#include <exception>

#ifndef HAS_JSCPPUTILS
#define HAS_JSCPPUTILS 1
#endif

#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
#include <JsCPPUtils/SmartPointer.h>
#endif

namespace JsRPC {

	class Serializable;

	class SerializableCreateFactory
	{
	public:
		virtual Serializable *create() = 0;
	};

	struct SerializableMemberInfo {
		enum ProtoType {
			PTYPE_NULL = 0,
			PTYPE_BOOL = 0x0001,
			PTYPE_SINT8 = 0x0011,
			PTYPE_UINT8 = 0x0021,
			PTYPE_SINT16 = 0x0012,
			PTYPE_UINT16 = 0x0022,
			PTYPE_SINT32 = 0x0014,
			PTYPE_UINT32 = 0x0024,
			PTYPE_SINT64 = 0x0018,
			PTYPE_UINT64 = 0x0028,
			PTYPE_FLOAT = 0x0044,
			PTYPE_DOUBLE = 0x0048,
			PTYPE_CHAR = 0x0081,
			PTYPE_WCHAR = 0x0082,
			PTYPE_SUBPAYLOAD = 0x0400, // Sub payload
			PTYPE_ARRAY = 0x1000,
			PTYPE_LIST = 0x4000
		};
		enum MemberType {
			MTYPE_NATIVE = 0,
			MTYPE_STDBASICSTRING = 0x0001, // Add Prefix : Length(4)
			MTYPE_STDVECTOR = 0x0002, // Add Prefix : Length(4)

			MTYPE_NATIVEARRAY = 0x010000, // Length : Array length
			MTYPE_STDLIST = 0x020000,
			MTYPE_JSCPPUTILSMARTPOINTER = 0x040000,
		};

		std::string name;
		ProtoType ptype;
		MemberType mtype;
		void *ptr;
		int32_t length;
		SerializableCreateFactory *createFactory;

		SerializableMemberInfo() {
			this->ptype = PTYPE_NULL;
			this->mtype = MTYPE_NATIVE;
			this->ptr = NULL;
			this->length = 0;
			this->createFactory = NULL;
		}
		SerializableMemberInfo(const std::string &_name, int _ptype, int _mtype, void *_ptr, int32_t _length) {
			this->name = _name;
			this->ptype = (ProtoType)_ptype;
			this->mtype = (MemberType)_mtype;
			this->ptr = _ptr;
			this->length = _length;
		}

		SerializableMemberInfo &setCreateFactory(SerializableCreateFactory *_createFactory) {
			this->createFactory = _createFactory;
			return *this;
		}
	};

	class Serializable
	{
	public:
		class UnavailableTypeException : public std::exception
		{ };
		class ParseException : public std::exception
		{ };

	private:
		static const unsigned char header[];
		std::string m_name;
		int64_t m_serialVersionUID;
		std::list<SerializableMemberInfo> m_members;

	public:
		Serializable(const char *name, int64_t serialVersionUID);
		virtual ~Serializable();
		void serialize(std::vector<unsigned char>& payload) throw(UnavailableTypeException);
		void deserialize(const std::vector<unsigned char>& payload) throw (ParseException);
		const std::list<SerializableMemberInfo> &serializableMembers() { return m_members; }

	protected:
		void serializableMapMember(const char *name, bool &object);
		void serializableMapMember(const char *name, char &object);
		void serializableMapMember(const char *name, wchar_t &object);
		void serializableMapMember(const char *name, int8_t &object);
		void serializableMapMember(const char *name, uint8_t &object);
		void serializableMapMember(const char *name, int16_t &object);
		void serializableMapMember(const char *name, uint16_t &object);
		void serializableMapMember(const char *name, int32_t &object);
		void serializableMapMember(const char *name, uint32_t &object);
		void serializableMapMember(const char *name, int64_t &object);
		void serializableMapMember(const char *name, uint64_t &object);
		void serializableMapMember(const char *name, float &object);
		void serializableMapMember(const char *name, double &object);
		void serializableMapMember(const char *name, Serializable *object);
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
		SerializableMemberInfo &serializableMapMember(const char *name, JsCPPUtils::SmartPointer<Serializable> *object);
#endif

		void serializableMapMemberArray(const char *name, std::basic_string<char> &object);
		void serializableMapMemberArray(const char *name, std::basic_string<wchar_t> &object); //15
		void serializableMapMemberArray(const char *name, std::vector<char> &object);
		void serializableMapMemberArray(const char *name, std::vector<wchar_t> &object);
		void serializableMapMemberArray(const char *name, std::vector<int8_t> &object);
		void serializableMapMemberArray(const char *name, std::vector<uint8_t> &object);
		void serializableMapMemberArray(const char *name, std::vector<int16_t> &object);
		void serializableMapMemberArray(const char *name, std::vector<uint16_t> &object);
		void serializableMapMemberArray(const char *name, std::vector<int32_t> &object);
		void serializableMapMemberArray(const char *name, std::vector<uint32_t> &object);
		void serializableMapMemberArray(const char *name, std::vector<int64_t> &object);
		void serializableMapMemberArray(const char *name, std::vector<uint64_t> &object);

		void serializableMapMemberArray(const char *name, bool *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, char *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, wchar_t *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, int8_t *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, uint8_t *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, int16_t *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, uint16_t *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, int32_t *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, uint32_t *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, int64_t *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, uint64_t *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, float *object, int32_t arrlength);
		void serializableMapMemberArray(const char *name, double *object, int32_t arrlength);

		void serializableMapMemberList(const char *name, std::list< std::basic_string<char> >& object);
		void serializableMapMemberList(const char *name, std::list< std::basic_string<wchar_t> >& object);
		SerializableMemberInfo &serializableMapMemberList(const char *name, std::list<Serializable*>& object);
#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS
		SerializableMemberInfo &serializableMapMemberList(const char *name, std::list<JsCPPUtils::SmartPointer<Serializable> >& object);
#endif

	private:
		bool checkFlagsAll(int value, int type)
		{
			return (value & type) == type;
		}
	};
}
