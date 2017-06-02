==========================
Generating Consensus Tests
==========================

Consensus Tests
===============

`Consensus tests`_ are test cases for all Ethereum implementations.
The test cases are distributed in the "filled" form, which contains, for example, the expected state root hash after transactions.
The filled test cases are usually not written by hand, but generated from "test filler" files.
``testeth`` executable in cpp-ethereum can convert test fillers into test cases.

When you add a test case in the consensus test suite, you are supposed to push both the filler and the filled test cases into the ``tests`` repository.

.. _`Consensus tests`: https://github.com/ethereum/tests

Generating a GeneralStateTest Case
==================================

Designing a Test Case
---------------------

For creating a new GeneralStateTest case, you need:

* something
  something
  something

Converting a GeneralStateTest Case into a BlockchainTest Case
=============================================================




Generating a BlockchainTest Case
================================

(To be described.)
