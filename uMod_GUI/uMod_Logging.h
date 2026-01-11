#ifndef uMod_LOGGING_H_
#define uMod_LOGGING_H_

#include <wx/string.h>

void InitLogger(void);
void ShutdownLogger(void);
void LogMessage(const wxString &message);
wxString GetLogPath(void);

#endif /* uMod_LOGGING_H_ */
