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
 * @file	JSONObjectMapper.h
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2018/12/06
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#pragma once

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
		static void serializeTo(const Serializable *serialiable, rapidjson::Document &jsonDoc);
		static void deserializeJsonObject(Serializable *serialiable, const rapidjson::Value &jsonObject);

		static std::string serialize(const Serializable *serialiable)
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

	private:
		static bool checkFlagsAll(int value, int type)
		{
			return (value & type) == type;
		}
#endif
	};

}
