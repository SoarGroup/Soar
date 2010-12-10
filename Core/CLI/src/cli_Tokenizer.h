/////////////////////////////////////////////////////////////////////////////
// Tokenizer / Parser for Soar commands and source files.
//
// Author: Jonathan Voigt <voigtjr@gmail.com>
// Date: 2010
//
// Essentially implements a simple Tcl parser. Takes a string and farily 
// efficiently converts it in to a series of callbacks with arguments 
// separated in to a vector of strings (what Tcl refers to as "words").
//
// Here are the rules, copied from the Tcl language summary (man tcl), with 
// modifications [in square brackets] as they apply to this parser:
// 
// [1] A Tcl script is a string containing one or more commands. Semi-colons
// and newlines are command separators unless quoted as described below. 
// Close brackets are command terminators during command substitution (see 
// below) unless quoted. [Square brackets have no special meaning in this
// parser.]
//
// [2] A command is evaluated in two steps. First, the Tcl interpreter breaks
// the command into words and performs substitutions as described below. 
// These substitutions are performed in the same way for all commands. [The
// first word of the command has no special meaning in this parser.] All of 
// the words of the command are passed to the command procedure [via a 
// callback interface]. The command procedure is free to interpret each of 
// its words in any way it likes, such as an integer, variable name, list, or
// Tcl script. Different commands interpret their words differently.
//
// [3] Words of a command are separated by white space (except for newlines,
// which are command separators).
//
// [4] If the first character of a word is double-quote (") then the word is
// terminated by the next double-quote character. If semi-colons, close 
// brackets, or white space characters (including newlines) appear between 
// the quotes then they are treated as ordinary characters and included in 
// the word. Backslash substitution [but not command substitution or variable
// substitution] is performed on the characters between the quotes as 
// described below. The double-quotes are not retained as part of the word.
//
// [5] If the first character of a word is an open brace ({) then the word is
// terminated by the matching close brace (}). Braces nest within the word: 
// for each additional open brace there must be an additional close brace 
// (however, if an open brace or close brace within the word is quoted with a 
// backslash then it is not counted in locating the matching close brace). No
// substitutions are performed on the characters between the braces except 
// for backslash-newline substitutions described below, nor do semi-colons, 
// newlines, close brackets, or white space receive any special 
// interpretation. The word will consist of exactly the characters between 
// the outer braces, not including the braces themselves.
//
// [6] [Square brackets are not special in this parser. No command
// substitution.]
//
// [7] [Dollar signs are not special in this parser. No variable 
// substitution.]
//
// [8] If a backslash (\) appears within a word then backslash substitution
// occurs. In all cases but those described below the backslash is dropped 
// and the following character is treated as an ordinary character and 
// included in the word. This allows characters such as double quotes, and
// close brackets [and dollar signs but they aren't special characters in
// this parser] to be included in words without triggering special 
// processing. The following table lists the backslash sequences that are 
// handled specially, along with the value that replaces each sequence.
//
// \a Audible alert (bell) (0x7).
// \b Backspace (0x8).
// \f Form feed (0xc).
// \n Newline (0xa).
// \r Carriage-return (0xd).
// \t Tab (0x9).
// \v Vertical tab (0xb).
// \<newline>whiteSpace
//     A single space character replaces the backslash, newline, and all 
//     spaces and tabs after the newline. This backslash sequence is unique 
//     in that [...] it will be replaced even when it occurs between braces, 
//     and the resulting space will be treated as a word separator if it 
//     isn't in braces or quotes.
// \\ Backslash (``\'').
// [Not implemented: \ooo The digits ooo (one, two, or three of them) give 
// the octal value of the character.]
// [Not implemented: \xhh The hexadecimal digits hh give the hexadecimal 
// value of the character. Any number of digits may be present.]
//
// Backslash substitution is not performed on words enclosed in braces, 
// except for backslash-newline as described above.
//
// [9] If a hash character (#) appears at a point where Tcl is expecting the
// first character of the first word of a command, then the hash character 
// and the characters that follow it, up through the next newline, are 
// treated as a comment and ignored. The comment character only has 
// significance when it appears at the beginning of a command.
// 
// [10] [Talks about details regarding substitution and generally does not 
// apply to this parser.]
//
// [11] [Talks about details regarding substitution and generally does not 
// apply to this parser.]
/////////////////////////////////////////////////////////////////////////////

#ifndef CLI_TOKENIZER_H
#define CLI_TOKENIZER_H

#include <string>
#include <vector>

//#define PRINT_CALLBACKS 1
#ifdef PRINT_CALLBACKS
#include <iostream>
#endif

namespace cli
{
    class TokenizerCallback
    {
    public:
        virtual ~TokenizerCallback() {}
        virtual bool HandleCommand(std::vector<std::string>& argv) = 0;
    };

    class TokenizerCurrent
    {
    private:
        int line;
        int offset;
        const char* current;

    public:
        TokenizerCurrent()
        {
            SetCurrent(0);
        }
        TokenizerCurrent(const char* initial)
        {
            SetCurrent(initial);
        }
        virtual ~TokenizerCurrent() {}
        
        void SetCurrent(const char* initial)
        {
            current = initial;
            if (current && *current)
            {
                line = 1;
                offset = 1;
            }
            else
            {
                line = 0;
                offset = 0;
            }
        }

        void Fail()
        {
            current = 0;
        }

        void Increment()
        {
            if (*current == '\n')
            {
                line += 1;
                offset = 0;
            }
            current += 1;
            offset += 1;
        }

        int GetLine()
        {
            return line;
        }

        int GetOffset()
        {
            return offset;
        }

        bool Good()
        {
            return current != 0;
        }

        bool Bad()
        {
            return !current;
        }

        char Get()
        {
            return *current;
        }

        bool Eof()
        {
            return !*current;
        }
    };

    class Tokenizer
    {
    private:
        TokenizerCurrent current;
        TokenizerCallback* callback;
        int commandStartLine;
        const char* error;

    public:
        Tokenizer()
            : callback(0), error(0) 
        {}
        virtual ~Tokenizer() {}

        void SetHandler(TokenizerCallback* callback)
        {
            this->callback = callback;
        }

        bool Evaluate(const char* const input)
        {
            current.SetCurrent(input);
            commandStartLine = 1;
            error = 0;

            while (current.Good())
            {
                if (current.Eof())
                    break;
                ParseCommand();
            }
            return current.Good();
        }

        int GetCommandLineNumber()
        {
            return commandStartLine;
        }

        int GetCurrentLineNumber()
        {
            return current.GetLine();
        }

        const char* GetErrorString()
        {
            return error;
        }

        int GetOffset()
        {
            return current.GetOffset();
        }

    private:
        void ParseCommand()
        {
            std::vector< std::string > argv;
            SkipWhitespaceAndComments();
            while (ParseWord(argv))
                SkipWhitespace();

            if (current.Bad())
                return;

            if (argv.empty())
                return;

#ifdef PRINT_CALLBACKS
            std::cout << "\n[";
            for (int i = 0; i < argv.size(); i++)
                std::cout << argv[i] << ",";
            std::cout << "]" << std::endl;
#endif
            if (callback)
            {
                if (!callback->HandleCommand(argv))
                {
                    error = "callback returned error";
                    current.Fail();
                }
            }
        }

        bool ParseWord(std::vector< std::string >& argv)
        {
            if (current.Eof())
                return false;

            std::string word;
            if (argv.empty())
                commandStartLine = current.GetLine();

            switch (current.Get())
            {
            case ';':
                break;

            case '"':
                argv.push_back(word);
                ReadQuotedString(argv);
                break;

            case '{':
                argv.push_back(word);
                ReadBraces(argv);
                break;

            default:
                argv.push_back(word);
                ReadNormalWord(argv);
                break;
            }

            return !AtEndOfCommand();
        }

        void ReadNormalWord(std::vector< std::string >& argv)
        {
            do
            {
                char c = current.Get();
                bool semi = false;
                switch (c)
                {
                case '\\':
                    c = ParseEscapeSequence();
                    if (current.Bad())
                        return;
                    break;

                case ';':
                    semi = true;
                    // falls to be consumed
                default:
                    current.Increment();
                    break;
                }

                if (semi)
                    return;

                argv.back().push_back(c);
            }
            while (!current.Eof() && !isspace(current.Get()));
        }

        bool AtEndOfCommand()
        {
            while (current.Good())
            {
                switch (current.Get())
                {
                case '\n':
                case ';':
                    current.Increment();
                case 0:
                    return true;

                default:
                    break;
                }

                if (!isspace(current.Get()))
                    return false;

                current.Increment();
            }
            return true;
        }

        void ReadQuotedString(std::vector< std::string >& argv)
        {
            current.Increment(); // consume "

            while (current.Get() != '"')
            {
                switch (current.Get())
                {
                case 0: 
                    error = "unexpected eof";
                    current.Fail();
                    return;

                case '\\':
                    {
                        char c = ParseEscapeSequence();
                        if (current.Bad()) return;
                        argv.back().push_back(c);
                    }
                    break;

                default:
                    argv.back().push_back(current.Get());
                    current.Increment();
                    break;
                }
            }

            current.Increment(); // consume "
        }

        char ParseEscapeSequence()
        {
            current.Increment(); // consume backslash

            // future work? newline, octal, hex, wide hex

            char ret = 0;
            bool increment = true;
            switch (current.Get())
            {
            case 0:
                error = "unexpected eof";
                current.Fail();
                return 0;
            case 'a':
                ret = '\a';
                break;
            case 'b':
                ret = '\b';
                break;
            case 'f':
                ret = '\f';
                break;
            case 'n':
                ret = '\n';
                break;
            case 'r':
                ret = '\r';
                break;
            case 't':
                ret = '\t';
                break;
            case 'v':
                ret = '\v';
                break;
            case '\n':
                SkipWhitespace();
                ret = ' ';
                increment = false; // SkipWhitespace leaves us past where we want to be
                break;
            default:
                ret = current.Get();
                break;
            }
            if (increment)
                current.Increment();
            return ret;
        }

        void ReadBraces(std::vector< std::string >& argv)
        {
            current.Increment(); // consume brace;
            int depth = 1;
            while (depth)
            {
                bool increment = true;
                switch (current.Get())
                {
                case 0:
                    error = "unexpected eof, unmatched opening brace";
                    current.Fail();
                    return;

                case '\\':
                    // special case for backslash-newline substitution
                    current.Increment();
                    if (current.Get() == '\n')
                    {
                        SkipWhitespace();
                        increment = false;
                        argv.back().push_back(' ');
                        break;
                    }

                    // current is escaped but no substitution
                    argv.back().push_back('\\');
                    argv.back().push_back(current.Get());
                    break;

                case '}':
                    depth -= 1;
                    if (depth) // consume only the final closing brace
                        argv.back().push_back(current.Get());
                    break;

                case '{':
                    depth += 1;
                    // falls through
                default:
                    argv.back().push_back(current.Get());
                    break;
                }
                if (increment)
                    current.Increment();
            }
        }

        void SkipWhitespaceAndComments()
        {
            SkipWhitespace();
            switch (current.Get())
            {
            case '0':
                return;
            case '#':
                SkipToEndOfLine();
                SkipWhitespaceAndComments();
                return;
            default:
                break;
            }
        }

        void SkipWhitespace()
        {
            while (!current.Eof() && isspace(current.Get()))
                current.Increment();
        }

        void SkipToEndOfLine()
        {
            while (current.Get() != '\n')
            {
                current.Increment();
                if (current.Eof())
                    break;
            }
        }
    };
}

#endif // CLI_TOKENIZER_H
