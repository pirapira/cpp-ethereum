==========================
Generating Consensus Tests
==========================

Consensus Tests
===============

`Consensus tests`_ are test cases for all Ethereum implementations.
The test cases are distributed in the "filled" form, which contains, for example, the expected state root hash after transactions.
The filled test cases are usually not written by hand, but generated from "test filler" files.
``testeth`` executable in cpp-ethereum can convert test fillers into test cases.

When you add a test case in the consensus test suite, you are supposed to push both the filler and the filled test cases into the ```tests`` repository`__.

.. _`Consensus tests`: https://github.com/ethereum/tests

__ `Consensus tests`_

Generating a GeneralStateTest Case
==================================

Designing a Test Case
---------------------

For creating a new GeneralStateTest case, you need:

* environmental parameters
* a transaction
* a state before the transaction (pre-state)
* some expectations about the state after the transaction

For an idea, peek into an existing test filler under ``src/GeneralStateFiller`` in the tests repository.

Usually, when a test is about an instruction, the pre-state contains a contract with
a code containing the instruction.  Typically, the contract stores a value in the storage,
so that the instruction's behavior is visible in the storage in the expectation.

The code can be written in EVM bytecode or in LLL.

Writing a Test Filler
---------------------

A new test filler needs to be alone in a new test filler file.  A single filler file is not supposed to contain multiple tests.  ``testeth`` tool still accepts multiple test fillers in a single test filler file, but this might change.

In the tests repository, the test filler files for GeneralStateTests live under ``src/GeneralStateTestsFiller`` directory.
The directory has many subdirectories.  You need to choose one of the subdirectories or create one.  The name of the filler file needs to end with ``Filler.json``.  For example, we might want to create a new directory ``src/GeneralStateTestsFiller/stReturnDataTest`` with a new filler file ``returndatacopy_initialFiller.json``.

The easiest way to start is to copy an existing filler file.  The first thing to change is the name of the test in the beginning of the file. The name of the test should coincide with the file name except ``Filler.json``. For example, in the file we created above, the filler file contains the name of the test ``returndatacopy_initial``.  The overall structure of ``returndatacopy_initialFiller.json`` should be:

.. code::

   {
       "returndatacopy_initial" : {
          "env" : { ... }
		  "expect" : [ ... ]
		  "pre" " { ... }
		  "transaction" : { ... }
       }
   }

where ``...`` indicates omissions.

``env`` field contains some parameters in a straightforward way.

``pre`` field describes the pre-state account-wise:

.. code::

     "pre" : {
        "0x0f572e5295c57f15886f9b263e2f6d2d6c7b5ec6" : {
            "balance" : "0x0de0b6b3a7640000",
            "code" : "{ (MSTORE 0 0x112233445566778899aabbccddeeff) (RETURNDATACOPY 0 0 32) (SSTORE 0 (MLOAD 0)) }",
            "code" : "0x306000526020600060003e600051600055",
            "nonce" : "0x00",
            "storage" : {
                "0x00" : "0x01"
            }
        }
     }


As specified in the Yellow Paper, an account contains a balance, a code, a nonce and a storage.

Notice the ``code`` field is duplicated.  If many fields exist under the same name, the last one is used.
In this particular case, the LLL compiler was not ready to parse the new instruction ``RETURNDATACOPY`` so a compiled runtime bytecode is added as the second ``code`` field.

This particular test expected to see ``0`` in the first slot in the storage.  In order to make this change visible, the pre-state has ``1`` there.


Writing a Test Filler
---------------------

Converting a GeneralStateTest Case into a BlockchainTest Case
=============================================================




Generating a BlockchainTest Case
================================

(To be described.)
