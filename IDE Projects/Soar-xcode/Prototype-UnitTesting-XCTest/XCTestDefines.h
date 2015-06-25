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
	unittest.runner = new TestRunner(nullptr, nullptr, nullptr); \
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
	catch (AssertException& e) { \
		const char* string = e.what();\
		_XCTFailureHandler(self, YES, e.file(), e.line(), _XCTFailureFormat(_XCTAssertion_Fail, 0), @"%@", [NSString stringWithUTF8String:string]);\
	}\
}

#endif /* XCTestDefines_h */
