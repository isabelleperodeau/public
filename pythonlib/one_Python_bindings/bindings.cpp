/**
 * bindings.cpp
 * Main bindings file that combines One API functionalities for Python using Pybind11
 */

#include <pybind11/pybind11.h>
#include <iostream>
#include <string>
#include "auto_includes.h"
 
 // Function declarations from one_base.cpp
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
 
 // Function declarations from one_subscriber.cpp
bool subscribeInstr(OneCSession_t session, const std::string& service, const std::string& instr, OneCErrInfo_t& err);
bool unsubscribeInstr(OneCSession_t session, const std::string& service, const std::string& instr, OneCErrInfo_t& err);
bool sendOnDemandFeedRequest(OneCSession_t session, const std::string& service, const std::string& instr, OneCErrInfo_t& err, bool async);
bool sendCacheRequest(OneCSession_t session, const std::string& service, const std::string& instr, const std::string& fields, OneCErrInfo_t& err, bool async);
bool sendOnBehalfRequest(OneCSession_t session, const std::string& service, const std::string& identifier, int reqType, OneCErrInfo_t& err, bool async);
bool endOnBehalfSubscription(OneCSession_t session, const std::string& service, const std::string& identifier, OneCErrInfo_t& err);
 
 // Function declarations from publisher_bindings.cpp
OneCMsg_t createPublisherMessage(const std::string& service, const std::string& productKey, 
                                  const std::string& instr, const std::string& msgType, 
                                  bool deliverToOne);
void updatePublisherMessage(OneCMsg_t msg, const std::string& title, int number);
void printMessage(OneCMsg_t msg);
OneCSession_t createPublisherSession(const std::string& connectString,
                                      const std::string& user,
                                      const std::string& password,
                                      const std::string& appProdKey,
                                      bool doSubscribe,
                                      OneCErrInfo_t& err);
bool subscribeForPublishing(OneCSession_t session, const std::string& service,
                               const std::string& instr, OneCErrInfo_t& err);
bool publishMessage(OneCSession_t session, OneCMsg_t msg, bool useCStruct, OneCErrInfo_t& err);
bool unsubscribeFromPublishing(OneCSession_t session, const std::string& service,
                                  const std::string& instr, OneCErrInfo_t& err);
bool destroyMessage(OneCMsg_t msg);
void setTLSParams(const std::string& trustStoreDir);
void myLogFunction(OneCLogLevel_t level, OneCLogOrigin_t src, const char* logMsg);
 
extern "C" const char* oneCGetVersionString();
extern "C" const char* oneCGetCppVersionString();
extern "C" const char* oneCGetSolVersionString();
 
 // Additional publisher wrappers (from publisher_bindings.cpp)
 // These wrappers expose functions exactly as in the original C file.
unsigned int py_oneCMGetMsgSize(OneCMsg_t msg) {
     return oneCMGetMsgSize(msg);
}
unsigned int py_oneCMGetFieldCount(OneCMsg_t msg) {
     return oneCMGetFieldCount(msg);
}
void py_oneCMResizeMessage(OneCMsg_t msg, unsigned int newSize) {
     oneCMResizeMessage(msg, newSize);
}
void py_oneCMSetFieldCount(OneCMsg_t msg, unsigned int fieldCount) {
     oneCMSetFieldCount(msg, fieldCount);
}
bool py_oneCMDestroyMsg(OneCMsg_t msg) {
     return oneCMDestroyMsg(msg);
}

void py_oneCMAddNullStringField(OneCMsg_t msgHandle, const std::string& fieldName, bool optional) {
      oneCMAddNullStringField(msgHandle, fieldName.c_str(), optional); 
}

void py_oneCMAddField(OneCMsg_t msgHandle, const std::string& fieldName, bool optional, HmsFieldType_t type, const void* value) {
      oneCMAddField(msgHandle, fieldName.c_str(), optional, type, value);
}
 

namespace py = pybind11;
 
PYBIND11_MODULE(one_python_api, m) {
     m.doc() = "Complete One API binding for Python";
 
     // Bind OneCErrInfo_t structure
     py::class_<OneCErrInfo_t>(m, "OneCErrInfo")
         .def(py::init<>())
         .def_property("text", &getText, &setText, "Error text description");
 
     // Core API functions
     m.def("initializeOneAPI", &initializeOneAPI, "Initialize the One API",
           py::arg("apiVerbosity"), py::arg("solVerbosity"), py::arg("err"));
     m.def("createSession", &createSession, 
           py::arg("connectString"), py::arg("user"), py::arg("password"), 
           py::arg("appProdKey"), py::arg("err"), py::arg("rawData"),
           "Create a new session with conditional callback");
     m.def("destroySession", &destroySession, 
           py::arg("session"), py::arg("err"), "Destroy a session");
     m.def("shutdownOneAPI", &shutdownOneAPI, "Shut down the One API");
     m.def("getSessionString", &getSessionString,
           py::arg("session"), py::arg("bufSize"), py::arg("err"),
           "Get session information as a string");
     m.def("printSessionParams", &printSessionParams,
           py::arg("session"), py::arg("err"), "Print session parameters");
     m.def("setTLSTrustParams", &setTLSTrustParams,
           py::arg("trustStoreDir"), py::arg("useClientCertAuth"),
           "Set TLS trust parameters");
     m.def("getSessionMetrics", &getSessionMetrics, 
           py::arg("session"), py::arg("err"), 
           py::return_value_policy::reference,
           "Get session metrics");
 
     // Subscriber functions
     m.def("subscribeInstr", &subscribeInstr, 
           py::arg("session"), py::arg("service"), py::arg("instrument"), py::arg("err"),
           "Subscribe to an instrument");
     m.def("unsubscribeInstr", &unsubscribeInstr,
           py::arg("session"), py::arg("service"), py::arg("instrument"), py::arg("err"),
           "Unsubscribe from an instrument");
     m.def("sendOnDemandFeedRequest", &sendOnDemandFeedRequest,
           py::arg("session"), py::arg("service"), py::arg("instrument"), py::arg("err"),
           py::arg("async") = false,
           "Send an on-demand feed request");
     m.def("sendCacheRequest", &sendCacheRequest,
           py::arg("session"), py::arg("service"), py::arg("instrument"),
           py::arg("fields"), py::arg("err"), py::arg("async") = false,
           "Send a cache request");
     m.def("sendOnBehalfRequest", &sendOnBehalfRequest,
           py::arg("session"), py::arg("service"), py::arg("identifier"),
           py::arg("reqType"), py::arg("err"), py::arg("async") = false,
           "Send an on-behalf request");
     m.def("endOnBehalfSubscription", &endOnBehalfSubscription,
           py::arg("session"), py::arg("service"), py::arg("identifier"), py::arg("err"),
           "End on-behalf subscription");
 
     // Publisher functions
     m.def("createPublisherSession", &createPublisherSession,
           py::arg("connectString"), py::arg("user"), py::arg("password"),
           py::arg("appProdKey"), py::arg("doSubscribe") = true, py::arg("err"),
           "Create a session for publishing messages");
     m.def("createPublisherMessage", &createPublisherMessage,
           py::arg("service"), py::arg("productKey"), py::arg("instrument"),
           py::arg("msgType") = "APP", py::arg("deliverToOne") = false,
           "Create a message with various field types for publishing");
     m.def("updatePublisherMessage", &updatePublisherMessage,
           py::arg("msg"), py::arg("title"), py::arg("number"),
           "Update key fields in the message before publishing");
     m.def("printMessage", &printMessage,
           py::arg("msg"), "Print message content to console");
     m.def("subscribeForPublishing", &subscribeForPublishing,
           py::arg("session"), py::arg("service"), py::arg("instrument"), py::arg("err"),
           "Subscribe to receive published messages");
     m.def("publishMessage", &publishMessage,
           py::arg("session"), py::arg("msg"), py::arg("useCStruct") = false, py::arg("err"),
           "Publish a message");
     m.def("unsubscribeFromPublishing", &unsubscribeFromPublishing,
           py::arg("session"), py::arg("service"), py::arg("instrument"), py::arg("err"),
           "Unsubscribe from receiving published messages");
     m.def("destroyMessage", &destroyMessage,
           py::arg("msg"), "Destroy a message");
     m.def("setTLSParams", &setTLSParams,
           py::arg("trustStoreDir") = "",
           "Set TLS parameters for secure connections");
 
     // Bind additional publisher wrappers exactly as in the original C file.
     m.def("oneCMGetMsgSize", &py_oneCMGetMsgSize, "Get message size", py::arg("msg"));
     m.def("oneCMGetFieldCount", &py_oneCMGetFieldCount, "Get message field count", py::arg("msg"));
     m.def("oneCMResizeMessage", &py_oneCMResizeMessage, "Resize message", py::arg("msg"), py::arg("newSize"));
     m.def("oneCMSetFieldCount", &py_oneCMSetFieldCount, "Set message field count", py::arg("msg"), py::arg("fieldCount"));
     m.def("oneCMDestroyMsg", &py_oneCMDestroyMsg, "Destroy message", py::arg("msg"));
 

     // Bind additional functions exported by the API.
     m.def("myLogFunction", &myLogFunction,
           py::arg("level"), py::arg("src"), py::arg("logMsg"),
           "Log function for One API");
     m.def("get_version_string", &oneCGetVersionString, "Retrieve the One API version string");
     m.def("get_cpp_version_string", &oneCGetCppVersionString, "Retrieve the One API C++ version string");
     m.def("get_solace_version_string", &oneCGetSolVersionString, "Retrieve the Solace version string");

     // // Bind additional publisher wrappers exactly as in the original C file.
     m.def("oneCMAddNullStringField", &oneCMAddNullStringField, "Add a null string field to a message",
           py::arg("msgHandle"), py::arg("fieldName"), py::arg("optional") = false);
     
     m.def("oneCMAddField", &oneCMAddField, "Add a field to a message",
           py::arg("msgHandle"), py::arg("fieldName"), py::arg("optional") = false, py::arg("type"), py::arg("value"));

 }
 