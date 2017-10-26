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
 * Stub for generating main boost.test module.
 * Original code taken from boost sources.
 */


#define BOOST_TEST_MODULE EthereumTests
#define BOOST_TEST_NO_MAIN
#include <boost/test/included/unit_test.hpp>

#include <clocale>
#include <cstdlib>
#include <iostream>
#include <test/tools/libtesteth/TestHelper.h>

using namespace boost::unit_test;

static std::ostringstream strCout;
std::streambuf* oldCoutStreamBuf;
std::streambuf* oldCerrStreamBuf;


static std::atomic_bool stopTravisOut;
void travisOut()
{
	int tickCounter = 0;
	while (!stopTravisOut)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		++tickCounter;
		if (tickCounter % 10 == 0)
			std::cout << ".\n" << std::flush;  // Output dot every 10s.
	}
}

/*
The equivalent of setlocale(LC_ALL, “C”) is called before any user code is run.
If the user has an invalid environment setting then it is possible for the call
to set locale to fail, so there are only two possible actions, the first is to
throw a runtime exception and cause the program to quit (default behaviour),
or the second is to modify the environment to something sensible (least
surprising behaviour).

The follow code produces the least surprising behaviour. It will use the user
specified default locale if it is valid, and if not then it will modify the
environment the process is running in to use a sensible default. This also means
that users do not need to install language packs for their OS.
*/
void setDefaultOrCLocale()
{
#if __unix__
	if (!std::setlocale(LC_ALL, ""))
	{
		setenv("LC_ALL", "C", 1);
	}
#endif
}

//Custom Boost Unit Test Main
int main( int argc, char* argv[] )
{
	std::string const dynamicTestSuiteName = "RandomTestCreationSuite";
	setDefaultOrCLocale();
	try
	{
		//Initialize options
	}
	catch (dev::test::Options::InvalidOption const& e)
	{
		std::cerr << e.what() << "\n";
		exit(1);
	}

	stopTravisOut = false;
	std::thread outputThread(travisOut);
	auto fakeInit = [](int, char*[]) -> boost::unit_test::test_suite* { return nullptr; };
	int result = unit_test_main(fakeInit, argc, argv);
	stopTravisOut = true;
	outputThread.join();
	dev::test::TestOutputHelper::get().printTestExecStats();
	return result;
}
