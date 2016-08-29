#ifndef UNITTEST_H
#define UNITTEST_H

#include <iostream>
#include <list>
#include <stdexcept>
#include <string>
#include <sstream>

#define DO_ALIAS_TESTS
#define DO_CLIPARSER_TESTS
//#define DO_CHUNKING_TESTS
#define DO_ELEMENTXML_TESTS
#define DO_EPMEM_TESTS
#define DO_FULL_TESTS
#define DO_IO_TESTS
#define DO_SMEM_TESTS
#define DO_MISC_TESTS
#define DO_MULTIAGENT_TESTS
#define DO_SANITY_TESTS
#define DO_TOKENIZER_TESTS
//#define SKIP_SLOW_TESTS
#define SKIP_JAVA_DEBUGGER_TEST

#define CPPUNIT_STR_STR(x) #x
#define CPPUNIT_STR(x) CPPUNIT_STR_STR(x)

class CPPUnit_Assert_Failure : public std::runtime_error
{
    public:
        explicit CPPUnit_Assert_Failure(const std::string& msg = "CPPUnit Assertion Failure")
            : std::runtime_error(msg)
        {
        }
};

class CPPUnit_Function
{
    public:
        virtual ~CPPUnit_Function() {}
        virtual void operator()() = 0;
};

#define CPPUNIT_ASSERT(x) if(!(x)) { \
        std::ostringstream oss; \
        oss << "CPPUnit Assertion Failure " << " in file " << __FILE__ << ':' << __LINE__ << " : " <<  CPPUNIT_STR(x); \
        throw CPPUnit_Assert_Failure(oss.str()); \
    }
#define CPPUNIT_ASSERT_MESSAGE(m, x) if(!(x)) { \
        std::ostringstream oss; \
        oss << "CPPUnit Assertion Failure " << " in file " << __FILE__ << ':' << __LINE__ << " : " << CPPUNIT_STR(x); \
        oss << std::endl << m; \
        throw CPPUnit_Assert_Failure(oss.str()); \
    }

#define CPPUNIT_TEST_SUITE(x) typedef x test_t; \
    class Member_Function : public CPPUnit_Function { \
        public: \
            Member_Function(test_t * const &this_, void (test_t::*fun_)()) : m_this(this_), m_fun(fun_) {} \
            void operator()() {(m_this->*m_fun)();} \
        private: \
            test_t * m_this; \
            void (test_t::*m_fun)(); \
    }; \
    friend class Register_##x; \
    std::string get_class_name() const {return CPPUNIT_STR(x);} \
    std::list<std::pair<std::string, CPPUnit_Function *> > get_tests() { \
        std::list<std::pair<std::string, CPPUnit_Function *> > tests;
#define CPPUNIT_TEST(x) \
    tests.push_back(std::make_pair(CPPUNIT_STR(x), new Member_Function(this, &test_t::x)));
#define CPPUNIT_TEST_SUITE_END() \
    return tests; \
    }

#define CPPUNIT_TEST_SUITE_REGISTRATION(x) \
    static class Register_##x { \
        public: \
            Register_##x() { \
                CPPUNIT_NS::TestFactoryRegistry::getRegistry().giveTest(&m_test_class); \
            } \
            x m_test_class; \
    } g_registration_##x;

class TestFactoryRegistry;

class CPPUNIT_NS
{
    public:
        class TestListener
        {
            public:
                virtual void tell(const bool&) = 0;
        };

        class TestResult
        {
            public:
                void addListener(TestListener* const& listener)
                {
                    m_listeners.push_back(listener);
                }

                void tell(const bool& result)
                {
                    for (std::list<TestListener*>::iterator lt = m_listeners.begin(), lend = m_listeners.end(); lt != lend; ++lt)
                    {
                        (*lt)->tell(result);
                    }
                }

            private:
                std::list<TestListener*> m_listeners;
        };

        class TestCase
        {
                virtual std::string get_class_name() const = 0;
                virtual std::list<std::pair<std::string, CPPUnit_Function*> > get_tests() = 0;
                virtual void setUp() {}
                virtual void tearDown() {}

            public:
                virtual void run(TestResult& result)
                {
                    std::list<std::pair<std::string, CPPUnit_Function*> > tests = get_tests();
                    for (std::list<std::pair<std::string, CPPUnit_Function*> >::iterator tt = tests.begin(), tend = tests.end(); tt != tend; ++tt)
                    {
                        std::cout << this->get_class_name() << "::" << tt->first;
                        std::cout.flush();
                        this->setUp();
                        try
                        {
                            (*tt->second)();
                            result.tell(true);
                        }
                        catch (CPPUnit_Assert_Failure& caf)
                        {
                            result.tell(false);
                            /* The following line will print out the result of the last command, which can
                             * have some error info, but seems to have a lot of incidental output.
                             * These errors aren't that useful and can be distractingly large.  If there's
                             * a problem, we always have to manually run the test agent anyway and can see
                             * what is really going on. */
//                            std::cerr << caf.what() << std::endl;
                        }
                        this->tearDown();
                        delete tt->second;
                    }
                }
        };

        class TestResultCollector : public TestListener
        {
            public:
                TestResultCollector()
                    : m_successes(0),
                      m_failures(0)
                {
                }

                bool wasSuccessful() const
                {
                    return !m_failures;
                }
                size_t successes() const
                {
                    return m_successes;
                }
                size_t failures() const
                {
                    return m_failures;
                }

            private:
                void tell(const bool& result)
                {
                    ++(result ? m_successes : m_failures);
                }

                size_t m_successes;
                size_t m_failures;
        };

        class BriefTestProgressListener : public TestListener
        {
                void tell(const bool& result)
                {
                    std::cout << " : " << (result ? "OK" : "<=--- FAILED ---=>") << std::endl;
                }
        };

        class TestRunner
        {
            public:
                void addTest(TestCase* const test)
                {
                    tests.push_back(test);
                }

                void run(TestResult& result)
                {
                    for (std::list<TestCase*>::iterator tt = tests.begin(), tend = tests.end(); tt != tend; ++tt)
                    {
                        (*tt)->run(result);
                    }
                }

            private:
                std::list<TestCase*> tests;
        };

        class CompilerOutputter
        {
            public:
                CompilerOutputter(const TestResultCollector* const trc, std::ostream& os)
                    : m_trc(trc),
                      m_os(&os)
                {
                }

                void write() const
                {
                    if (m_trc->wasSuccessful())
                    {
                        *m_os << "OK (" << m_trc->successes() << ')' << std::endl;
                    }
                    else
                    {
                        *m_os << "FAILURE (" << m_trc->successes() << " successful, " << m_trc->failures() << " failed)" << std::endl;
                    }
                }

            private:
                const TestResultCollector* m_trc;
                std::ostream* m_os;
        };

        class TestFactoryRegistry
        {
                TestFactoryRegistry(const TestFactoryRegistry&);
                TestFactoryRegistry operator=(const TestFactoryRegistry&);

                TestFactoryRegistry() {}
                ~TestFactoryRegistry() {}

                class TestCases : public TestCase
                {
                        TestCases(const TestCases&);
                        TestCases operator =(const TestCases&);

                        TestCases() {}
                        ~TestCases() {}

                        std::string get_class_name() const
                        {
                            return "";
                        }
                        std::list<std::pair<std::string, CPPUnit_Function*> > get_tests()
                        {
                            return std::list<std::pair<std::string, CPPUnit_Function*> >();
                        }

                    public:
                        static TestCases& get_TestCases()
                        {
                            static TestCases g_TestCases;
                            return g_TestCases;
                        }

                        void run(TestResult& result_)
                        {
                            TestResult& result = result_;
                            for (std::list<TestCase*>::iterator tt = tests.begin(), tend = tests.end(); tt != tend; ++tt)
                            {
                                (*tt)->run(result);
                            }
                        }

                        std::list<TestCase*> tests;
                };

            public:
                static TestFactoryRegistry& getRegistry()
                {
                    static TestFactoryRegistry g_TestFactoryRegistry;
                    return g_TestFactoryRegistry;
                }

                TestCase* makeTest() const
                {
                    return &TestCases::get_TestCases();
                }

                void giveTest(TestCase* const test)
                {
                    TestCases::get_TestCases().tests.push_back(test);
                }

            private:
        };

};

#endif
