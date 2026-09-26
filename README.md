# Vat7_AutoTest

Copyright (c) 2026 BrainInAVat7
License: AGPL-3.0
Version: 0.1.0-alpha

A simple automated testing framework for C.

The test framework works, and it's behavior is largely documented
in the header. The current version was lifted straight from a work
in progress personal project, of which this is one part. I found it
interesting and useful enough to start treating it as a standalone
project. I have removed some code that interfaced with the specifics
of the larger project and left the rest alone for now.

There are some oddities. For example, suite and test setup/teardown
was built before some changes were made to how tests are run, and it
is very likely they are broken or don't behave as one might expect.

Automatic test discovery is not supported. Tests must be registered
manually.

**To Do:** Write documentation, and lots more!
