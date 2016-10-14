//
//  XCTestDefines.h
//  Soar-xcode
//
//  Created by Alex Turner on 6/25/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef XCTestDefines_h
#define XCTestDefines_h

#define TEST_SETUP(X) @interface XC_ ##X : XCTestCase \
{ \
\
	X unittest; \
} \
@end \
\
@implementation XC_ ##X \
\
- (void)setUp {\
	[super setUp];\
	unittest.TestCategory::runner = new TestRunner(nullptr, nullptr, nullptr); \
	unittest.setUp();\
}\
\
- (void)tearDown {\
	unittest.tearDown(false);\
	[super tearDown];\
}

#define XC_TEST(X) - (void) X { \
	try { \
		unittest.X(); \
	} \
    catch (const SoarAssertionException& e) { \
		NSLog(@"%s", unittest.TestCategory::runner->output.str().c_str()); \
		const char* string = e.what();\
		_XCTFailureHandler(self, YES, e.file(), e.line(), _XCTFailureFormat(_XCTAssertion_Fail, 0), @"%@", [NSString stringWithUTF8String:string]);\
	}\
	catch (...) { \
        std::exception_ptr e = std::current_exception(); \
        try { \
            if (e) \
                std::rethrow_exception(e); \
        } \
        catch (const std::exception& e) \
        { \
            NSLog(@"%s", unittest.TestCategory::runner->output.str().c_str()); \
            const char* string = e.what();\
            _XCTFailureHandler(self, YES, "", 0, _XCTFailureFormat(_XCTAssertion_Fail, 0), @"%@", [NSString stringWithUTF8String:string]);\
        } \
	} \
}

#endif /* XCTestDefines_h */
