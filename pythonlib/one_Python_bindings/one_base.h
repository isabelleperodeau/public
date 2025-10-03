#ifndef ONE_BASE_H
#define ONE_BASE_H

#include <string>
#include "auto_includes.h"

// Function declarations
void mySleep(const char* message, int millis);

void onData(const OneCSessionParams_t* info, OneCSession_t sessionHandle, 
            unsigned int queueIndex, const HmsDataInfo_t* data);

void onRawData(const OneCSessionParams_t* info, OneCSession_t sessionHandle, 
               unsigned int queueIndex, const HmsDataInfo_t* data);

void onMonData(const OneCSessionParams_t* info, OneCSession_t sessionHandle, 
               const HmsDataInfo_t* data);

void onSessionDisconnected(const OneCSessionParams_t* info, OneCSession_t sessionHandle);

void myLogFunction(OneCLogLevel_t level, OneCLogOrigin_t src, const char* logMsg);

bool initializeOneAPI(int apiVerbosity, int solVerbosity, OneCErrInfo_t* err);

OneCSession_t createSession(const std::string& connectString, const std::string& user, 
                            const std::string& password, const std::string& appProdKey, 
                            OneCErrInfo_t& err, bool rawData);

std::string getSessionString(OneCSession_t session, int bufSize, OneCErrInfo_t& err);

void printSessionParams(OneCSession_t session, OneCErrInfo_t& err);

void setTLSTrustParams(const std::string& trustStoreDir, bool useClientCertAuth);

std::string getSessionMetrics(OneCSession_t session, OneCErrInfo_t& err);

void destroySession(OneCSession_t session, OneCErrInfo_t* err);

void shutdownOneAPI();

std::string getText(const OneCErrInfo_t& err);

void setText(OneCErrInfo_t& err, const std::string& value);

#endif // ONE_BASE_H
