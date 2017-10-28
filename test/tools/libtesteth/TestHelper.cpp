/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file
 * Helper functions to work with json::spirit and test files
 */

#include <include/BuildInfo.h>
#include <libethashseal/EthashCPUMiner.h>
#include <test/tools/libtesteth/TestHelper.h>
#include <test/tools/libtesteth/TestOutputHelper.h>
#include <test/tools/libtesteth/Options.h>
#include <cryptopp/config.h>

#if !defined(_WIN32)
#include <stdio.h>
#endif
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem/path.hpp>
#include <libethereum/Client.h>
#include <string>

using namespace std;
using namespace dev::eth;
namespace fs = boost::filesystem;

namespace dev
{
namespace eth
{


}

namespace test
{


string netIdToString(eth::Network _netId)
{
	switch(_netId)
	{
		case eth::Network::FrontierTest: return "Frontier";
		case eth::Network::HomesteadTest: return "Homestead";
		case eth::Network::EIP150Test: return "EIP150";
		case eth::Network::EIP158Test: return "EIP158";
		case eth::Network::ByzantiumTest: return "Byzantium";
		case eth::Network::ConstantinopleTest: return "Constantinople";
		case eth::Network::FrontierToHomesteadAt5: return "FrontierToHomesteadAt5";
		case eth::Network::HomesteadToDaoAt5: return "HomesteadToDaoAt5";
		case eth::Network::HomesteadToEIP150At5: return "HomesteadToEIP150At5";
		case eth::Network::EIP158ToByzantiumAt5: return "EIP158ToByzantiumAt5";
		case eth::Network::TransitionnetTest: return "TransitionNet";
		default: return "other";
	}
	return "unknown";
}

eth::Network stringToNetId(string const& _netname)
{
	//Networks that used in .json tests
	static std::vector<eth::Network> const networks {{
		eth::Network::FrontierTest,
		eth::Network::HomesteadTest,
		eth::Network::EIP150Test,
		eth::Network::EIP158Test,
		eth::Network::ByzantiumTest,
		eth::Network::ConstantinopleTest,
		eth::Network::FrontierToHomesteadAt5,
		eth::Network::HomesteadToDaoAt5,
		eth::Network::HomesteadToEIP150At5,
		eth::Network::EIP158ToByzantiumAt5,
		eth::Network::TransitionnetTest
	}};

	for (auto const& net : networks)
		if (netIdToString(net) == _netname)
			return net;

	BOOST_ERROR(TestOutputHelper::get().testName() + " network not found: " + _netname);
	return eth::Network::FrontierTest;
}

bool isDisabledNetwork(eth::Network _net)
{
	Options const& opt = Options::get();
	if (opt.all || opt.filltests || opt.createRandomTest || !opt.singleTestNet.empty())
	{
		if (_net == eth::Network::ConstantinopleTest)
			return true;
		return false;
	}
	switch (_net)
	{
		case eth::Network::FrontierTest:
		case eth::Network::HomesteadTest:
		case eth::Network::ConstantinopleTest:
		case eth::Network::FrontierToHomesteadAt5:
		case eth::Network::HomesteadToDaoAt5:
		case eth::Network::HomesteadToEIP150At5:
			return true;
		default:
		break;
	}
	return false;
}

std::vector<eth::Network> const& getNetworks()
{
	//Networks for the test case execution when filling the tests
	static std::vector<eth::Network> const networks {{
		eth::Network::FrontierTest,
		eth::Network::HomesteadTest,
		eth::Network::EIP150Test,
		eth::Network::EIP158Test,
		eth::Network::ByzantiumTest,
		eth::Network::ConstantinopleTest
	}};
	return networks;
}

u256 toInt(json_spirit::mValue const& _v)
{
	switch (_v.type())
	{
	case json_spirit::str_type: return u256(_v.get_str());
	case json_spirit::int_type: return (u256)_v.get_uint64();
	case json_spirit::bool_type: return (u256)(uint64_t)_v.get_bool();
	case json_spirit::real_type: return (u256)(uint64_t)_v.get_real();
	default: cwarn << "Bad type for scalar: " << _v.type();
	}
	return 0;
}

byte toByte(json_spirit::mValue const& _v)
{
	switch (_v.type())
	{
	case json_spirit::str_type: return (byte)stoi(_v.get_str());
	case json_spirit::int_type: return (byte)_v.get_uint64();
	case json_spirit::bool_type: return (byte)_v.get_bool();
	case json_spirit::real_type: return (byte)_v.get_real();
	default: cwarn << "Bad type for scalar: " << _v.type();
	}
	return 0;
}

bytes importByteArray(std::string const& _str)
{
	checkHexHasEvenLength(_str);
	return fromHex(_str.substr(0, 2) == "0x" ? _str.substr(2) : _str, WhenError::Throw);
}

bytes importData(json_spirit::mObject const& _o)
{
	bytes data;
	if (_o.at("data").type() == json_spirit::str_type)
		data = importByteArray(_o.at("data").get_str());
	else
		for (auto const& j: _o.at("data").get_array())
			data.push_back(toByte(j));
	return data;
}

void replaceLLLinState(json_spirit::mObject& _o)
{
	for (auto& account: _o.count("alloc") ? _o["alloc"].get_obj() : _o.count("accounts") ? _o["accounts"].get_obj() : _o)
	{
		auto obj = account.second.get_obj();
		if (obj.count("code") && obj["code"].type() == json_spirit::str_type)
		{
			string code = obj["code"].get_str();
			obj["code"] = compileLLL(code);
		}
		account.second = obj;
	}
}

std::vector<boost::filesystem::path> getJsonFiles(boost::filesystem::path const& _dirPath, std::string const& _particularFile)
{
	vector<boost::filesystem::path> jsonFiles;
	if (!_particularFile.empty())
	{
		boost::filesystem::path file = _dirPath / (_particularFile + ".json");
		if (boost::filesystem::exists(file))
			jsonFiles.push_back(file);
	}
	else
	{
		using Bdit = boost::filesystem::directory_iterator;
		for (Bdit it(_dirPath); it != Bdit(); ++it)
			if (boost::filesystem::is_regular_file(it->path()) && it->path().extension() == ".json")
					jsonFiles.push_back(it->path());
	}
	return jsonFiles;
}

std::string executeCmd(std::string const& _command)
{
#if defined(_WIN32)
	BOOST_ERROR("executeCmd() has not been implemented for Windows.");
	return "";
#else
	string out;
	char output[1024];
	FILE *fp = popen(_command.c_str(), "r");
	if (fp == NULL)
		BOOST_ERROR("Failed to run " + _command);
	if (fgets(output, sizeof(output) - 1, fp) == NULL)
		BOOST_ERROR("Reading empty result for " + _command);
	else
	{
		while(true)
		{
			out += string(output);
			if (fgets(output, sizeof(output) - 1, fp) == NULL)
				break;
		}
	}

	int exitCode = pclose(fp);
	if (exitCode != 0)
		BOOST_ERROR("The command '" + _command + "' exited with " + toString(exitCode) + " code.");
	return boost::trim_copy(out);
#endif
}

string compileLLL(string const& _code)
{
	if (_code == "")
		return "0x";
	if (_code.substr(0,2) == "0x" && _code.size() >= 2)
	{
		checkHexHasEvenLength(_code);
		return _code;
	}

#if defined(_WIN32)
	BOOST_ERROR("LLL compilation only supported on posix systems.");
	return "";
#else
	boost::filesystem::path path(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path());
	string cmd = string("lllc ") + path.string();
	writeFile(path.string(), _code);
	string result = executeCmd(cmd);
	boost::filesystem::remove(path);
	result = "0x" + result;
	checkHexHasEvenLength(result);
	return result;
#endif
}

void checkHexHasEvenLength(string const& _str)
{
	if (_str.size() % 2)
		BOOST_ERROR(TestOutputHelper::get().testName() + " An odd-length hex string represents a byte sequence: " + _str);
}

bytes importCode(json_spirit::mObject const& _o)
{
	bytes code;
	if (_o.count("code") == 0)
		return code;
	if (_o.at("code").type() == json_spirit::str_type)
		if (_o.at("code").get_str().find("0x") != 0)
			code = fromHex(compileLLL(_o.at("code").get_str()));
		else
			code = importByteArray(_o.at("code").get_str());
	else if (_o.at("code").type() == json_spirit::array_type)
	{
		code.clear();
		for (auto const& j: _o.at("code").get_array())
			code.push_back(toByte(j));
	}
	return code;
}

void checkOutput(bytesConstRef _output, json_spirit::mObject const& _o)
{
	int j = 0;
	auto expectedOutput = _o.at("out").get_str();

	if (expectedOutput.find("#") == 0)
		BOOST_CHECK(_output.size() == toInt(expectedOutput.substr(1)));
	else if (_o.at("out").type() == json_spirit::array_type)
		for (auto const& d: _o.at("out").get_array())
		{
			BOOST_CHECK_MESSAGE(_output[j] == toInt(d), "Output byte [" << j << "] different!");
			++j;
		}
	else if (expectedOutput.find("0x") == 0)
		BOOST_CHECK(_output.contentsEqual(fromHex(expectedOutput.substr(2))));
	else
		BOOST_CHECK(_output.contentsEqual(fromHex(expectedOutput)));
}

void checkStorage(map<u256, u256> const& _expectedStore, map<u256, u256> const& _resultStore, Address const& _expectedAddr)
{
	for (auto&& expectedStorePair : _expectedStore)
	{
		auto& expectedStoreKey = expectedStorePair.first;
		auto resultStoreIt = _resultStore.find(expectedStoreKey);
		if (resultStoreIt == _resultStore.end())
			BOOST_ERROR(_expectedAddr << ": missing store key " << expectedStoreKey);
		else
		{
			auto& expectedStoreValue = expectedStorePair.second;
			auto& resultStoreValue = resultStoreIt->second;
			BOOST_CHECK_MESSAGE(expectedStoreValue == resultStoreValue, _expectedAddr << ": store[" << expectedStoreKey << "] = " << resultStoreValue << ", expected " << expectedStoreValue);
		}
	}
	BOOST_CHECK_EQUAL(_resultStore.size(), _expectedStore.size());
	for (auto&& resultStorePair: _resultStore)
	{
		if (!_expectedStore.count(resultStorePair.first))
			BOOST_ERROR(_expectedAddr << ": unexpected store key " << resultStorePair.first);
	}
}


namespace
{
	Listener* g_listener;
}

void Listener::registerListener(Listener& _listener)
{
	g_listener = &_listener;
}

void Listener::notifySuiteStarted(std::string const& _name)
{
	if (g_listener)
		g_listener->suiteStarted(_name);
}

void Listener::notifyTestStarted(std::string const& _name)
{
	if (g_listener)
		g_listener->testStarted(_name);
}

void Listener::notifyTestFinished(int64_t _gasUsed)
{
	if (g_listener)
		g_listener->testFinished(_gasUsed);
}



} } // namespaces
