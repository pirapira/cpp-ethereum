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

#pragma once

#include <thread>
#include <future>
#include <functional>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>

#include <libethashseal/Ethash.h>
#include <libethashseal/GenesisInfo.h>
#include <test/tools/libtestutils/Common.h>

#include <test/tools/libtesteth/JsonSpiritHeaders.h>
#include <test/tools/libtesteth/Options.h>
#include <test/tools/libtesteth/ImportTest.h>
#include <test/tools/libtesteth/TestOutputHelper.h>

namespace dev
{

namespace eth
{

class Client;

void mine(Client& c, int numBlocks);
void connectClients(Client& c1, Client& c2);
}

namespace test
{

struct ValueTooLarge: virtual Exception {};
struct MissingFields : virtual Exception {};
bigint const c_max256plus1 = bigint(1) << 256;

// helping functions
std::string prepareVersionString();
std::string prepareLLLCVersionString();
std::vector<boost::filesystem::path> getJsonFiles(boost::filesystem::path const& _dirPath, std::string const& _particularFile = {});
std::string netIdToString(eth::Network _netId);
eth::Network stringToNetId(std::string const& _netname);
bool isDisabledNetwork(eth::Network _net);
std::vector<eth::Network> const& getNetworks();
u256 toInt(json_spirit::mValue const& _v);
byte toByte(json_spirit::mValue const& _v);
void replaceLLLinState(json_spirit::mObject& _o);
std::string compileLLL(std::string const& _code);
std::string executeCmd(std::string const& _command);
bytes importCode(json_spirit::mObject const& _o);
bytes importData(json_spirit::mObject const& _o);
bytes importByteArray(std::string const& _str);
void checkHexHasEvenLength(std::string const&);
void copyFile(boost::filesystem::path const& _source, boost::filesystem::path const& _destination);
void checkOutput(bytesConstRef _output, json_spirit::mObject const& _o);
void checkStorage(std::map<u256, u256> _expectedStore, std::map<u256, u256> _resultStore, Address _expectedAddr);
void updateEthashSeal(dev::eth::BlockHeader& _header, h256 const& _mixHash, dev::eth::Nonce const& _nonce);
RLPStream createRLPStreamFromTransactionFields(json_spirit::mObject const& _tObj);
json_spirit::mObject fillJsonWithTransaction(eth::Transaction const& _txn);

//Fill Test Functions
bool createRandomTest();	//returns true if succeed, false if there was an error;
void doRlpTests(json_spirit::mValue const& _input);

/// Allows observing test execution process.
/// This class also provides methods for registering and notifying the listener
class Listener
{
public:
	virtual ~Listener() = default;

	virtual void suiteStarted(std::string const&) {}
	virtual void testStarted(std::string const& _name) = 0;
	virtual void testFinished(int64_t _gasUsed) = 0;

	static void registerListener(Listener& _listener);
	static void notifySuiteStarted(std::string const& _name);
	static void notifyTestStarted(std::string const& _name);
	static void notifyTestFinished(int64_t _gasUsed);

	/// Test started/finished notification RAII helper
	class ExecTimeGuard
	{
		int64_t m_gasUsed = -1;
	public:
		ExecTimeGuard(std::string const& _testName) { notifyTestStarted(_testName);	}
		~ExecTimeGuard() { notifyTestFinished(m_gasUsed); }
		ExecTimeGuard(ExecTimeGuard const&) = delete;
		ExecTimeGuard& operator=(ExecTimeGuard) = delete;
		void setGasUsed(int64_t _gas) { m_gasUsed = _gas; }
	};
};

}
}
