/**
 * one_base.cpp
 * Base functionality for One API bindings to Python using Pybind11
 */

 #include <pybind11/pybind11.h>
 #include <iostream>  
 #include <dlfcn.h>
 #include <string>
 #include <stdexcept>
 #include <cstring>
 #include <thread>
 #include <chrono>
 #include "auto_includes.h"  
 
 #define PRINT_ERR(apiCall) \
     std::cerr << "Error in " << apiCall << ": " << err.text << std::endl;
 
 namespace py = pybind11;
 
//  static ProgParams_t progParams = PUBSUB_PARAMS;
 
 void mySleep(const char* message, int millis) {
     if (message) {
         std::cout << message << ", sleeping " << millis << " millis ..." << std::endl;
     }
     std::this_thread::sleep_for(std::chrono::milliseconds(millis));
 }
 
 // Callback functions
 void onData(const OneCSessionParams_t* info, OneCSession_t sessionHandle,
             unsigned int queueIndex, const HmsDataInfo_t* data) {
     char thrName[256];
     char timeBuf[256];
 
     bcc_getThisThreadName(thrName, sizeof(thrName));
     bcc_timeString_de(timeBuf, sizeof(timeBuf), false);
 
     std::cout << "thread '" << thrName << "', " << timeBuf
               << " : received Message on session " << sessionHandle
               << ", appDataPointer = " << info->appDataPointer << "\n";
 
     oneCPrintHmsData(stdout, data, true, true, 3);
 }
 
 void onRawData(const OneCSessionParams_t* info, OneCSession_t sessionHandle, unsigned int queueIndex, const HmsDataInfo_t* data) {
     std::cout << "Raw Data received: " << data << std::endl;  
 }
 
 void onMonData(const OneCSessionParams_t* info, OneCSession_t sessionHandle, const HmsDataInfo_t* data) {
     char thrName[256];
     char timeBuf[256];
 
     bcc_getThisThreadName(thrName, sizeof(thrName));
     bcc_timeString_de(timeBuf, sizeof(timeBuf), false);
 
     std::cout << "thread '" << thrName << "', " << timeBuf 
               << " : Monitor message received on session " << sessionHandle << "\n";
 
     oneCPrintHmsData(stdout, data, true, true, 3);
 }
 
 void onSessionDisconnected(const OneCSessionParams_t* info, OneCSession_t sessionHandle) {
     std::cout << "Session disconnected." << std::endl;
 }
 
 void myLogFunction(OneCLogLevel_t level, OneCLogOrigin_t src, const char* logMsg) {
     char thrName[256] = "main";
     char timeBuf[256] = "";
     bcc_timeString_de(timeBuf, sizeof(timeBuf), false);
 
     std::cerr << "myLogFunction() thread '" << thrName << "', " << timeBuf
               << " : level = " << level << " (" << oneCLogLevelToCString(level)
               << "), source = " << src << ", logMsg = '" << logMsg << "'\n";
 }
 
 // Core API functions
 bool initializeOneAPI(int apiVerbosity, int solVerbosity, OneCErrInfo_t* err) {
     bool ret = oneCInitializeV(apiVerbosity, solVerbosity, myLogFunction, err);
     if (!ret) {
         std::cerr << "Error during initialization: " << err->text << std::endl;
     }
     return ret;
 }
 
 OneCSession_t createSession(
     const std::string& connectString,
     const std::string& user,
     const std::string& password,
     const std::string& appProdKey,
     OneCErrInfo_t& err,
     bool rawData) {
 
     OneCSessionParams_t sessionParams = OneCSessionParams_INIT;
     OneCUserCallbacks_t cbs = OneCUserCallbacks_INIT;
 
     // Set callbacks based on rawData flag
     if (rawData) {
         cbs.onData = nullptr;
         cbs.onRawData = onRawData;
     } else {
         cbs.onData = onData;
         cbs.onRawData = nullptr;
     }
     cbs.onMonData = onMonData;
     cbs.onSessionDisconnected = onSessionDisconnected;
 
     // Assign parameters from Python to sessionParams
     sessionParams.connectString = connectString.c_str();
     sessionParams.user = user.c_str();
     sessionParams.password = password.c_str();
     sessionParams.appProdKey = appProdKey.c_str();
 
     OneCSession_t session = oneCCreateSession(&sessionParams, &cbs, &err);
     if (!session) {
         std::cerr << "Session creation failed: " << err.text << std::endl;
         throw std::runtime_error("Failed to create session");
     }
     return session;
 }
 
 std::string getSessionString(OneCSession_t session, int bufSize, OneCErrInfo_t& err) {
     char* buf = new char[bufSize];
     int ret = oneCGetSessionString(session, bufSize, buf, &err);
     std::string result(buf);
     delete[] buf;
     return result;
 }
 
 void printSessionParams(OneCSession_t session, OneCErrInfo_t& err) {
     OneCSessionParams_t params;
     if(oneCGetSessionParams(session, &params, &err)) {
         std::cout << "Session Parameters: \n"
                   << "Connect String: " << params.connectString << "\n"
                   << "User: " << params.user << "\n"
                   << "App Key: " << params.appProdKey << "\n"
                   << "Direct Session: " << params.directSession << std::endl;
     }
 }
 
 void setTLSTrustParams(const std::string& trustStoreDir, bool useClientCertAuth) {
     oneCSetTLSTrustParams(trustStoreDir.c_str(), useClientCertAuth);
 }
 
 std::string getSessionMetrics(OneCSession_t session, OneCErrInfo_t& err) {
     CSessionMetrics_t metrics;
     char buf[1024];
     oneCGetSessionMetrics(session, &metrics, &err);
     oneCMetricsToString(&metrics, buf, sizeof(buf));
     return std::string(buf);
 }
 
 void destroySession(OneCSession_t session, OneCErrInfo_t* err) {
     if (session) {
         if (!oneCDestroySession(session, err)) {
             std::cerr << "Error during session destruction: " << err->text << std::endl;
         } 
     }
 }
 
 void shutdownOneAPI() {
     oneCShutdown();
 }
 
 std::string getText(const OneCErrInfo_t& err) {
     return std::string(err.text);
 }
 
 void setText(OneCErrInfo_t& err, const std::string& value) {
     std::strncpy(err.text, value.c_str(), sizeof(err.text) - 1);
     err.text[sizeof(err.text) - 1] = '\0';  // Ensure null-termination
 }
 
 // Helper function to print Data Products
 static void printDataProducts(const DataProduct_t* prods, int num) {
     if (!prods) {
         puts("no data products for non-MECS-session");
         return;
     }
 
     puts("######################################## data products ########################################");
     printf("number = %d, { <service>, <product key> }\n", num);
 
     for (size_t i = 0; i < (size_t)num; ++i) {
         printf("{ <%s>, <%s> }\n", prods[i].service, prods[i].productKey);
     }
 }
 