
#include <portability.h>
#include "cli_CommandLineInterface.h"
#include <fstream>
#include <assert.h>
#include "cli_Commands.h"
#include "cli_CommandData.h"
#include "sml_Names.h"
#include "sml_Events.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoCLog(const eLogMode mode, const std::string* pFilename, const std::string* pToAdd, bool silent) {
    std::ios_base::openmode openmode = std::ios_base::out;

    switch (mode) {
        case LOG_NEWAPPEND:
            openmode |= std::ios_base::app;
            // falls through

        case LOG_NEW:
            if (!pFilename) break; // handle as just a query

            if (m_pLogFile)
                return SetError("Log already open: " + m_LogFilename);

            {
                std::string filename = *pFilename;

                m_pLogFile = new std::ofstream(filename.c_str(), openmode);
                if (!m_pLogFile)
                    return SetError("Failed to open " + filename);

                m_LogFilename = filename;
            }
            break;

        case LOG_ADD:
            if (!m_pLogFile) return SetError("Log is not open.");
            (*m_pLogFile) << *pToAdd << std::endl;
            return true;

        case LOG_CLOSE:
            if (!m_pLogFile) return SetError("Log is not open.");

            delete m_pLogFile;
            m_pLogFile = 0;
            m_LogFilename.clear();
            break;

        default:
        case LOG_QUERY:
            break;
    }

    if (!silent) {
        if (m_RawOutput) {
            m_Result << "Log file ";
            if (IsLogOpen()) {
                m_Result << "'" + m_LogFilename + "' open.";
            } else {
                m_Result << "closed.";
            }

        } else {
            const char* setting = IsLogOpen() ? sml_Names::kTrue : sml_Names::kFalse;
            AppendArgTagFast(sml_Names::kParamLogSetting, sml_Names::kTypeBoolean, setting);

            if (m_LogFilename.size()) AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, m_LogFilename);
        }
    }
    return true;
}
