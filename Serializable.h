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
 * @file	Serializable.h
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2018/12/06
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#pragma once

#include <stdint.h>
#include <wchar.h>
#include <string>
#include <list>
#include <vector>
#include <exception>

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

	namespace internal {
		struct SerializableMemberInfo {
			enum EncapType {
				ETYPE_STDBASICSTRING = 1,
				ETYPE_STDVECTOR = 2,
				ETYPE_STDLIST = 3,
				ETYPE_SUBPAYLOAD = 4,
				ETYPE_SMARTPOINTER = 5,
				ETYPE_NATIVEARRAY = 0x0200,
				ETYPE_NULL = 0x8000,
				ETYPE_NATIVE = 0x0100,
				ETYPE_BOOL = 0x0001,
				ETYPE_SINT = 0x0010,
				ETYPE_UINT = 0x0020,
				ETYPE_FLOAT = 0x0044,
				ETYPE_DOUBLE = 0x0048,
				ETYPE_CHAR = 0x0081,
				ETYPE_WCHAR = 0x0082,
			};

			std::string name;
			std::list<EncapType> encaps;
			void *ptr;
			int32_t length;
			SerializableCreateFactory *createFactory;
			bool isNull;

			SerializableMemberInfo(const std::list<EncapType>& _encaps) {
				this->encaps = _encaps;
				this->ptr = NULL;
				this->length = 0;
				this->createFactory = NULL;
				this->isNull = false;
			}
		};

		class STypeCommon {
		public:
			friend class Serializable;
			SerializableMemberInfo _memberInfo;

		public:
			STypeCommon(const std::list<internal::SerializableMemberInfo::EncapType>& _encaps) :
			_memberInfo(_encaps)
			{
			}

			virtual ~STypeCommon() {

			}

			void setCreateFactory(SerializableCreateFactory *factory) {
				this->_memberInfo.createFactory = factory;
			}

			virtual void clear() = 0;

			void setNull() {
				_memberInfo.isNull = true;
			}
			void setNull(bool value) {
				_memberInfo.isNull = value;
			}
			const bool isNull() const {
				return _memberInfo.isNull;
			}

			bool operator!() const {
				return _memberInfo.isNull;
			}
		};
	}

	template <typename T>
	class STypeBase : public internal::STypeCommon
	{
	protected:
		T _value;

		STypeBase(const std::list<internal::SerializableMemberInfo::EncapType>& _encaps) :
			STypeCommon(_encaps)
		{
		}

	public:
		virtual ~STypeBase() { }

		void set(const T value) {
			this->_memberInfo.isNull = false;
			this->_value = value;
		}
		const T& get() const {
			return this->_value;
		}

		T& operator*() {
			this->_memberInfo.isNull = false;
			return this->_value;
		}
		const T& operator*() const {
			return this->_value;
		}

		STypeBase<T> &operator=(const STypeBase<T>& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull())
				this->_value = obj.get();
			return *this;
		}
	};
	template <typename T>
	class SRefTypeBase : public internal::STypeCommon
	{
	protected:
		T &_value;

		SRefTypeBase(const std::list<internal::SerializableMemberInfo::EncapType>& _encaps, T &refvalue) :
			STypeCommon(_encaps)
			, _value(refvalue)
		{
		}

	public:
		virtual ~SRefTypeBase() { }

		void set(const T value) {
			this->_memberInfo.isNull = false;
			this->_value = value;
		}
		const T get() const {
			return this->_value;
		}

		T& operator*() {
			this->_memberInfo.isNull = false;
			return this->_value;
		}
		const T& operator*() const {
			return this->_value;
		}

		SRefTypeBase<T> &operator=(const SRefTypeBase<T>& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull())
				this->_value = obj.get();
			return *this;
		}
	};
	template <typename T, int arraySize>
	class SArrayTypeBase : public internal::STypeCommon
	{
	protected:
		T _value[arraySize];

		SArrayTypeBase(int _ptype, const std::list<internal::SerializableMemberInfo::EncapType>& _encaps) :
			STypeCommon((internal::SerializableMemberInfo::ProtoType)_ptype, _encaps)
		{
		}

	public:
		virtual ~SArrayTypeBase() { }

		T& operator*() {
			this->_memberInfo.isNull = false;
			return this->_value;
		}
		const T& operator*() const {
			return this->_value;
		}
		void set(const T value) {
			this->_memberInfo.isNull = false;
			this->_value = value;
		}
		const T get() const {
			return this->_value;
		}

		SArrayTypeBase<T, arraySize> &operator=(const SArrayTypeBase<T, arraySize>& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull()) {
				memcpy(this->_value, obj.get(), sizeof(this->_value));
			}
			return *this;
		}
	};
	template <typename T, int arraySize>
	class SArrayTypeRefBase : public internal::STypeCommon
	{
	protected:
		T (&_value)[arraySize];

		SArrayTypeRefBase(int _ptype, const std::list<internal::SerializableMemberInfo::EncapType>& _encaps, T (&refvalue)[arraySize]) :
			STypeCommon((internal::SerializableMemberInfo::ProtoType)_ptype, _encaps)
			, _value(refvalue)
		{
		}

	public:
		virtual ~SArrayTypeRefBase() { }

		T& operator*() {
			this->_memberInfo.isNull = false;
			return this->_value;
		}
		const T& operator*() const {
			return this->_value;
		}
		void set(const T value) {
			this->_memberInfo.isNull = false;
			this->_value = value;
		}
		const T get() const {
			return this->_value;
		}

		SArrayTypeRefBase<T, arraySize> &operator=(const SArrayTypeRefBase<T, arraySize>& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull()) {
				memcpy(this->_value, obj.get(), sizeof(this->_value));
			}
			return *this;
		}
	};

	template<typename T>
	class SType
	{
	private:
		SType() {}
	};
	template<typename T>
	class SRefType
	{
	private:
		SRefType() {}
	};

	template<typename T, int arraySize>
	class SArrayType
	{
	private:
		SArrayType() {}
	};

	template <typename T>
	class SSerializableRefType;

	template <typename T>
	class SSerializableType : public internal::STypeCommon
	{
	protected:
		T _value;
		Serializable *_test;

		//internal::SerializableMemberInfo::ProtoType _ptype, const std::list<internal::SerializableMemberInfo::EncapType>& _encaps

	public:
		SSerializableType() : 
			STypeCommon({ internal::SerializableMemberInfo::ETYPE_SUBPAYLOAD })
		{
			this->_test = &this->_value;
			this->_memberInfo.ptr = &_value;
			this->_memberInfo.length = 1;
		}
		virtual ~SSerializableType() {}

		void clear() override {
			_value.serializableClearObjects();
		}

		T& operator*() {
			this->_memberInfo.isNull = false;
			return this->_value;
		}
		const T& operator*() const {
			return this->_value;
		}
		void set(const T value) {
			this->_memberInfo.isNull = false;
			this->_value = value;
		}
		const T &get() const {
			return this->_value;
		}
		void setNull() {
			this->_memberInfo.isNull = true;
		}
		void setNull(bool value) {
			this->_memberInfo.isNull = value;
		}
		const bool isNull() const {
			return this->_memberInfo.isNull;
		}

		SSerializableType<T> &operator=(const SSerializableType<T>& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull()) {
				std::vector<uint8_t> payload;
				obj._test->serialize(payload);
				this->_test->deserialize(payload);
			}
			return *this;
		}
		SSerializableType<T> &operator=(const SSerializableRefType<T>& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull()) {
				std::vector<uint8_t> payload;
				obj._test->serialize(payload);
				this->_test->deserialize(payload);
			}
			return *this;
		}
	};
	template <typename T>
	class SSerializableRefType : public internal::STypeCommon
	{
	protected:
		T &_value;
		Serializable *_test;

		//internal::SerializableMemberInfo::ProtoType _ptype, const std::list<internal::SerializableMemberInfo::EncapType>& _encaps

	public:
		SSerializableRefType(T &refvalue) :
			STypeCommon({ internal::SerializableMemberInfo::ETYPE_SUBPAYLOAD })
			, _value(refvalue)
		{
			this->_test = &this->_value;
			this->_memberInfo.ptr = &_value;
			this->_memberInfo.length = 1;
		}
		virtual ~SSerializableRefType() {}

		void clear() override {
			_value.serializableClearObjects();
		}

		T& operator*() {
			this->_memberInfo.isNull = false;
			return this->_value;
		}
		const T& operator*() const {
			return this->_value;
		}
		void set(const T value) {
			this->_memberInfo.isNull = false;
			this->_value = value;
		}
		const T get() const {
			return this->_value;
		}
		void setNull() {
			this->_memberInfo.isNull = true;
		}
		void setNull(bool value) {
			this->_memberInfo.isNull = value;
		}
		const bool isNull() const {
			return this->_memberInfo.isNull;
		}

		SSerializableRefType<T> &operator=(const SSerializableType<T>& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull()) {
				std::vector<uint8_t> payload;
				obj._test->serialize(payload);
				this->_test->deserialize(payload);
			}
			return *this;
		}
		SSerializableRefType<T> &operator=(const SSerializableRefType<T>& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull()) {
				std::vector<uint8_t> payload;
				obj._test->serialize(payload);
				this->_test->deserialize(payload);
			}
			return *this;
		}
	};

#if defined(HAS_JSCPPUTILS) && HAS_JSCPPUTILS

	template <typename T>
	class SSerializableRefType< JsCPPUtils::SmartPointer<T> >;

	template <typename T>
	class SSerializableType< JsCPPUtils::SmartPointer<T> > : public internal::STypeCommon
	{
	protected:
		JsCPPUtils::SmartPointer<T> _value;

	public:
		SSerializableType() :
			STypeCommon({ internal::SerializableMemberInfo::ETYPE_SMARTPOINTER, internal::SerializableMemberInfo::ETYPE_SUBPAYLOAD })
		{
			this->_memberInfo.ptr = &_value;
			this->_memberInfo.length = 1;
		}
		virtual ~SSerializableType() {}

		void clear() override {
			_value = NULL;
		}

		T& operator*() {
			this->_memberInfo.isNull = false;
			return this->_value;
		}
		const T& operator*() const {
			return this->_value;
		}
		void set(const T value) {
			this->_memberInfo.isNull = false;
			this->_value = value;
		}
		const T &get() const {
			return this->_value;
		}
		void setNull() {
			this->_memberInfo.isNull = true;
		}
		const bool isNull() {
			return this->_memberInfo.isNull;
		}
		SSerializableType< JsCPPUtils::SmartPointer<T> > &operator=(const SSerializableType< JsCPPUtils::SmartPointer<T> >& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull()) {
				this->set(obj.get());
			}
			return *this;
		}
		SSerializableType< JsCPPUtils::SmartPointer<T> > &operator=(const SSerializableRefType< JsCPPUtils::SmartPointer<T> >& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull()) {
				this->set(obj.get());
			}
			return *this;
		}
	};
	template <typename T>
	class SSerializableRefType< JsCPPUtils::SmartPointer<T> > : public internal::STypeCommon
	{
	protected:
		JsCPPUtils::SmartPointer<T> &_value;

	public:
		SSerializableRefType(JsCPPUtils::SmartPointer<T> &refvalue) :
			STypeCommon({ internal::SerializableMemberInfo::ETYPE_SMARTPOINTER, internal::SerializableMemberInfo::ETYPE_SUBPAYLOAD })
			, _value(refvalue)
		{
			this->_memberInfo.ptr = &_value;
			this->_memberInfo.length = 1;
		}
		virtual ~SSerializableRefType() {}

		void clear() override {
			_value = NULL;
		}

		T& operator*() {
			this->_memberInfo.isNull = false;
			return this->_value;
		}
		const T& operator*() const {
			return this->_value;
		}
		void set(const T value) {
			this->_memberInfo.isNull = false;
			this->_value = value;
		}
		const T get() const {
			return this->_value;
		}
		void setNull() {
			this->_memberInfo.isNull = true;
		}
		const bool isNull() const {
			return this->_memberInfo.isNull;
		}
		SSerializableRefType< JsCPPUtils::SmartPointer<T> > &operator=(const SSerializableType< JsCPPUtils::SmartPointer<T> >& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull()) {
				this->set(obj.get());
			}
			return *this;
		}
		SSerializableRefType< JsCPPUtils::SmartPointer<T> > &operator=(const SSerializableRefType< JsCPPUtils::SmartPointer<T> >& obj) {
			this->setNull(obj.isNull());
			if (!obj.isNull()) {
				this->set(obj.get());
			}
			return *this;
		}
	};
#endif

#define __JSRPC_SERIALIZABLE_GENSTYPE(CTYPE, ETYPE) \
	template<> \
	class SType<CTYPE> : public STypeBase<CTYPE> { \
	public: \
		SType() : \
		STypeBase({ (internal::SerializableMemberInfo::EncapType)((ETYPE) | internal::SerializableMemberInfo::EncapType::ETYPE_NATIVE) }) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
		void clear() override { _value = (CTYPE)0; } \
		SType<CTYPE>& operator=(const CTYPE& value) { \
			this->_memberInfo.isNull = false; \
			this->_value = value; \
			return *this; \
		} \
	}; \
	template<> \
	class SRefType<CTYPE> : public SRefTypeBase<CTYPE> { \
	public: \
		SRefType(CTYPE &refvalue) : \
		SRefTypeBase({ (internal::SerializableMemberInfo::EncapType)((ETYPE) | internal::SerializableMemberInfo::EncapType::ETYPE_NATIVE) }, refvalue) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
		void clear() override { _value = (CTYPE)0; } \
		SRefType<CTYPE>& operator=(const CTYPE& value) { \
			this->_memberInfo.isNull = false; \
			this->_value = value; \
			return *this; \
		} \
	}; \

	/*
#define __JSRPC_SERIALIZABLE_GENSARRAYTYPE(CPPTYPE, ...) \
	template<> \
	class SType<CPPTYPE> : public STypeBase<CPPTYPE> { \
	public: \
		SType() : \
		STypeBase({ __VA_ARGS__ }) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
	}
	*/

#define __JSRPC_SERIALIZABLE_GENSARRAYTYPE_STRING(CTYPE, ETYPE) \
	template<> \
	class SType< std::basic_string<CTYPE> > : public STypeBase< std::basic_string<CTYPE> > { \
	public: \
		SType() : \
		STypeBase({ internal::SerializableMemberInfo::ETYPE_STDBASICSTRING, (internal::SerializableMemberInfo::EncapType)(ETYPE) }) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
		void clear() override { _value.clear(); } \
		SType< std::basic_string<CTYPE> >& operator=(const std::basic_string<CTYPE>& value) { \
			this->_memberInfo.isNull = false; \
			this->_value = value; \
			return *this; \
		} \
		std::basic_string<CTYPE>& operator->() { \
			this->_memberInfo.isNull = false; \
			return this->_value; \
		} \
	}; \
	template<> \
	class SRefType< std::basic_string<CTYPE> > : public SRefTypeBase< std::basic_string<CTYPE> > { \
	public: \
		SRefType(std::basic_string<CTYPE> &refvalue) : \
		SRefTypeBase({ internal::SerializableMemberInfo::ETYPE_STDBASICSTRING, (internal::SerializableMemberInfo::EncapType)(ETYPE) }, refvalue) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
		void clear() override { _value.clear(); } \
		SRefType< std::basic_string<CTYPE> >& operator=(const std::basic_string<CTYPE>& value) { \
			this->_memberInfo.isNull = false; \
			this->_value = value; \
			return *this; \
		} \
		std::basic_string<CTYPE>& operator->() { \
			this->_memberInfo.isNull = false; \
			return this->_value; \
		} \
	};

#define __JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(CTYPE, ETYPE) \
	template<> \
	class SType< std::vector<CTYPE> > : public STypeBase< std::vector<CTYPE> > { \
	public: \
		SType() : \
		STypeBase({ internal::SerializableMemberInfo::ETYPE_STDVECTOR, (internal::SerializableMemberInfo::EncapType)(ETYPE) }) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
		void clear() override { _value.clear(); } \
		std::vector<CTYPE>& operator->() { \
			this->_memberInfo.isNull = false; \
			return this->_value; \
		} \
	}; \
	template<> \
	class SRefType< std::vector<CTYPE> > : public SRefTypeBase< std::vector<CTYPE> > { \
	public: \
		SRefType(std::vector<CTYPE> &refvalue) : \
		SRefTypeBase({ internal::SerializableMemberInfo::ETYPE_STDVECTOR, (internal::SerializableMemberInfo::EncapType)(ETYPE) }, refvalue) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
		void clear() override { _value.clear(); } \
		std::vector<CTYPE>& operator->() { \
			this->_memberInfo.isNull = false; \
			return this->_value; \
		} \
	};

#define __JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(CTYPE, ETYPE) \
	template<> \
	class SType< std::list<std::vector<CTYPE> > > : public STypeBase< std::list<std::vector<CTYPE> > > { \
	public: \
		SType() : \
		STypeBase({ internal::SerializableMemberInfo::ETYPE_STDLIST, internal::SerializableMemberInfo::ETYPE_STDVECTOR, (internal::SerializableMemberInfo::EncapType)(ETYPE) }) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
		void clear() override { _value.clear(); } \
		SType< std::list<std::vector<CTYPE> > >& operator=(const std::list<std::vector<CTYPE> >& value) { \
			this->_memberInfo.isNull = false; \
			this->_value = value; \
			return *this; \
		} \
		std::list<std::vector<CTYPE> >& operator->() { \
			this->_memberInfo.isNull = false; \
			return this->_value; \
		} \
	}; \
	template<> \
	class SRefType< std::list<std::vector<CTYPE> > > : public SRefTypeBase< std::list<std::vector<CTYPE> > > { \
	public: \
		SRefType(std::list<std::vector<CTYPE> > &refvalue) : \
		SRefTypeBase({ internal::SerializableMemberInfo::ETYPE_STDLIST, internal::SerializableMemberInfo::ETYPE_STDVECTOR, (internal::SerializableMemberInfo::EncapType)(ETYPE) }, refvalue) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
		void clear() override { _value.clear(); } \
		SRefType< std::list<std::vector<CTYPE> > >& operator=(const std::list<std::vector<CTYPE> >& value) { \
			this->_memberInfo.isNull = false; \
			this->_value = value; \
			return *this; \
		} \
		std::list<std::vector<CTYPE> >& operator->() { \
			this->_memberInfo.isNull = false; \
			return this->_value; \
		} \
	};

#define __JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_STRING(CTYPE, ETYPE) \
	template<> \
	class SType< std::list<std::basic_string<CTYPE> > > : public STypeBase< std::list<std::basic_string<CTYPE> > > { \
	public: \
		SType() : \
		STypeBase({ internal::SerializableMemberInfo::ETYPE_STDLIST, internal::SerializableMemberInfo::ETYPE_STDBASICSTRING, (internal::SerializableMemberInfo::EncapType)(ETYPE) }) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
		void clear() override { _value.clear(); } \
		SType< std::list<std::basic_string<CTYPE> > >& operator=(const std::list<std::basic_string<CTYPE> >& value) { \
			this->_memberInfo.isNull = false; \
			this->_value = value; \
			return *this; \
		} \
		std::list<std::basic_string<CTYPE> >& operator->() { \
			this->_memberInfo.isNull = false; \
			return this->_value; \
		} \
	}; \
	template<> \
	class SRefType< std::list<std::basic_string<CTYPE> > > : public SRefTypeBase< std::list<std::basic_string<CTYPE> > > { \
	public: \
		SRefType(std::list<std::basic_string<CTYPE> > &refvalue) : \
		SRefTypeBase({ internal::SerializableMemberInfo::ETYPE_STDLIST, internal::SerializableMemberInfo::ETYPE_STDBASICSTRING, (internal::SerializableMemberInfo::EncapType)(ETYPE) }, refvalue) { \
			this->_memberInfo.ptr = &_value; \
			this->_memberInfo.length = 1; \
		} \
		void clear() override { _value.clear(); } \
		SRefType< std::list<std::basic_string<CTYPE> > >& operator=(const std::list<std::basic_string<CTYPE> >& value) { \
			this->_memberInfo.isNull = false; \
			this->_value = value; \
			return *this; \
		} \
		std::list<std::basic_string<CTYPE> >& operator->() { \
			this->_memberInfo.isNull = false; \
			return this->_value; \
		} \
	};

	template<> 
	class SType< std::list< JsCPPUtils::SmartPointer<Serializable> > > : public STypeBase< std::list< JsCPPUtils::SmartPointer<Serializable> > > {
	public:
		SType() :
			STypeBase({ internal::SerializableMemberInfo::ETYPE_STDLIST, internal::SerializableMemberInfo::ETYPE_SMARTPOINTER, internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD }) {
			this->_memberInfo.ptr = &_value;
			this->_memberInfo.length = 1;
		}
		void clear() override { _value.clear(); }
		SType< std::list<JsCPPUtils::SmartPointer<Serializable> > >& operator=(const std::list<JsCPPUtils::SmartPointer<Serializable> >& value) {
			this->_memberInfo.isNull = false;
			this->_value = value;
			return *this;
		}
		std::list<JsCPPUtils::SmartPointer<Serializable> >& operator->() {
			this->_memberInfo.isNull = false;
			return this->_value;
		}
	};
	template<>
	class SRefType< std::list< JsCPPUtils::SmartPointer<Serializable> > > : public SRefTypeBase< std::list< JsCPPUtils::SmartPointer<Serializable> > > {
	public:
		SRefType(std::list< JsCPPUtils::SmartPointer<Serializable> > &refvalue) :
			SRefTypeBase({ internal::SerializableMemberInfo::ETYPE_STDLIST, internal::SerializableMemberInfo::ETYPE_SMARTPOINTER, internal::SerializableMemberInfo::EncapType::ETYPE_SUBPAYLOAD }, refvalue) {
			this->_memberInfo.ptr = &_value;
			this->_memberInfo.length = 1;
		}
		void clear() override { _value.clear(); }
		SRefType< std::list<JsCPPUtils::SmartPointer<Serializable> > >& operator=(const std::list<JsCPPUtils::SmartPointer<Serializable> >& value) {
			this->_memberInfo.isNull = false;
			this->_value = value;
			return *this;
		}
		std::list<JsCPPUtils::SmartPointer<Serializable> >& operator->() {
			this->_memberInfo.isNull = false;
			return this->_value;
		}
	};

#define __JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(CTYPE, ETYPE) \
	template<int arraySize> \
	class SArrayType<CTYPE, arraySize> : public SArrayTypeBase<CTYPE, arraySize> { \
	public: \
		SArrayType() : \
		SArrayTypeBase({ internal::SerializableMemberInfo::ETYPE_NATIVEARRAY | ((internal::SerializableMemberInfo::EncapType)(ETYPE)) }) { \
			this->_memberInfo.ptr = _value; \
			this->_memberInfo.length = arraySize; \
		} \
		void clear() override { memset(_value, 0, sizeof(_value)); } \
	};

	__JSRPC_SERIALIZABLE_GENSTYPE(bool, internal::SerializableMemberInfo::ETYPE_BOOL)
	__JSRPC_SERIALIZABLE_GENSTYPE(char, internal::SerializableMemberInfo::ETYPE_CHAR)
	__JSRPC_SERIALIZABLE_GENSTYPE(wchar_t, internal::SerializableMemberInfo::ETYPE_WCHAR)
	__JSRPC_SERIALIZABLE_GENSTYPE(int8_t, internal::SerializableMemberInfo::ETYPE_SINT | 1)
	__JSRPC_SERIALIZABLE_GENSTYPE(uint8_t, internal::SerializableMemberInfo::ETYPE_UINT | 1)
	__JSRPC_SERIALIZABLE_GENSTYPE(int16_t, internal::SerializableMemberInfo::ETYPE_SINT | 2)
	__JSRPC_SERIALIZABLE_GENSTYPE(uint16_t, internal::SerializableMemberInfo::ETYPE_UINT | 2)
	__JSRPC_SERIALIZABLE_GENSTYPE(int32_t, internal::SerializableMemberInfo::ETYPE_SINT | 4)
	__JSRPC_SERIALIZABLE_GENSTYPE(uint32_t, internal::SerializableMemberInfo::ETYPE_UINT | 4)
	__JSRPC_SERIALIZABLE_GENSTYPE(int64_t, internal::SerializableMemberInfo::ETYPE_SINT | 8)
	__JSRPC_SERIALIZABLE_GENSTYPE(uint64_t, internal::SerializableMemberInfo::ETYPE_UINT | 8)
	__JSRPC_SERIALIZABLE_GENSTYPE(float, internal::SerializableMemberInfo::ETYPE_FLOAT)
	__JSRPC_SERIALIZABLE_GENSTYPE(double, internal::SerializableMemberInfo::ETYPE_DOUBLE)
	
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(bool, internal::SerializableMemberInfo::ETYPE_BOOL)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(char, internal::SerializableMemberInfo::ETYPE_CHAR)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(wchar_t, internal::SerializableMemberInfo::ETYPE_WCHAR)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(int8_t, internal::SerializableMemberInfo::ETYPE_SINT | 1)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(uint8_t, internal::SerializableMemberInfo::ETYPE_UINT | 1)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(int16_t, internal::SerializableMemberInfo::ETYPE_SINT | 2)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(uint16_t, internal::SerializableMemberInfo::ETYPE_UINT | 2)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(int32_t, internal::SerializableMemberInfo::ETYPE_SINT | 4)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(uint32_t, internal::SerializableMemberInfo::ETYPE_UINT | 4)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(int64_t, internal::SerializableMemberInfo::ETYPE_SINT | 8)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(uint64_t, internal::SerializableMemberInfo::ETYPE_UINT | 8)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(float, internal::SerializableMemberInfo::ETYPE_FLOAT)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPEWITHNATIVELENGTH(double, internal::SerializableMemberInfo::ETYPE_DOUBLE)
	
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_STRING(char, internal::SerializableMemberInfo::ETYPE_CHAR)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_STRING(wchar_t, internal::SerializableMemberInfo::ETYPE_WCHAR)

	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(bool, internal::SerializableMemberInfo::ETYPE_BOOL)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(char, internal::SerializableMemberInfo::ETYPE_CHAR)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(wchar_t, internal::SerializableMemberInfo::ETYPE_WCHAR)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(int8_t, internal::SerializableMemberInfo::ETYPE_SINT | 1)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(uint8_t, internal::SerializableMemberInfo::ETYPE_UINT | 1)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(int16_t, internal::SerializableMemberInfo::ETYPE_SINT | 2)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(uint16_t, internal::SerializableMemberInfo::ETYPE_UINT | 2)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(int32_t, internal::SerializableMemberInfo::ETYPE_SINT | 4)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(uint32_t, internal::SerializableMemberInfo::ETYPE_UINT | 4)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(int64_t, internal::SerializableMemberInfo::ETYPE_SINT | 8)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(uint64_t, internal::SerializableMemberInfo::ETYPE_UINT | 8)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(float, internal::SerializableMemberInfo::ETYPE_FLOAT)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_VECTOR(double, internal::SerializableMemberInfo::ETYPE_DOUBLE)

	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_STRING(char, internal::SerializableMemberInfo::ETYPE_CHAR)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_STRING(wchar_t, internal::SerializableMemberInfo::ETYPE_WCHAR)

	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(bool, internal::SerializableMemberInfo::ETYPE_BOOL)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(char, internal::SerializableMemberInfo::ETYPE_CHAR)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(wchar_t, internal::SerializableMemberInfo::ETYPE_WCHAR)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(int8_t, internal::SerializableMemberInfo::ETYPE_SINT | 1)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(uint8_t, internal::SerializableMemberInfo::ETYPE_UINT | 1)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(int16_t, internal::SerializableMemberInfo::ETYPE_SINT | 2)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(uint16_t, internal::SerializableMemberInfo::ETYPE_UINT | 2)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(int32_t, internal::SerializableMemberInfo::ETYPE_SINT | 4)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(uint32_t, internal::SerializableMemberInfo::ETYPE_UINT | 4)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(int64_t, internal::SerializableMemberInfo::ETYPE_SINT | 8)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(uint64_t, internal::SerializableMemberInfo::ETYPE_UINT | 8)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(float, internal::SerializableMemberInfo::ETYPE_FLOAT)
	__JSRPC_SERIALIZABLE_GENSARRAYTYPE_LIST_VECTOR(double, internal::SerializableMemberInfo::ETYPE_DOUBLE)

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
		std::list<internal::STypeCommon*> m_members;

	protected:
#if (__cplusplus >= 201103) || (__cplusplus == 199711) || (defined(HAS_MOVE_SEMANTICS) && HAS_MOVE_SEMANTICS == 1)
		explicit Serializable(Serializable&& _ref)
		{
			assert(false);
		}
#endif

	private:
		Serializable(const Serializable &obj) {
			assert(false);
		}

	public:
		Serializable(const char *name, int64_t serialVersionUID);
		virtual ~Serializable();

		Serializable& operator=(const Serializable& obj) {
			assert(this->m_name == obj.m_name);
			assert(this->m_serialVersionUID == obj.m_serialVersionUID);
			std::vector<uint8_t> payload;
			obj.serialize(payload);
			this->deserialize(payload);
			return *this;
		}

		const std::list<internal::STypeCommon*> &serializableMembers() const { return m_members; }

		void serialize(std::vector<unsigned char>& payload) const throw(UnavailableTypeException);
		void deserialize(const std::vector<unsigned char>& payload) throw (ParseException);

		void serializableClearObjects();

		std::string serializableGetName() {
			return m_name;
		}
		int64_t serializableGetSerialVersionUID() {
			return m_serialVersionUID;
		}

	protected:
		internal::STypeCommon &serializableMapMember(const char *name, internal::STypeCommon &object);

	private:
		bool checkFlagsAll(int value, int type) const
		{
			return (value & type) == type;
		}
	};
}
