// publisher_bindings.cpp
// C++ file to bind One API publisher functions to Python using Pybind11

#include <pybind11/pybind11.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>
#include <thread>
#include <chrono>
#include "auto_includes.h"  // Assuming this includes all the necessary One API headers
#include "one_base.h"

namespace py = pybind11;
// Create a message with various field types
OneCMsg_t createPublisherMessage(const std::string& service, const std::string& productKey, 
                               const std::string& instr, const std::string& msgType, 
                               bool deliverToOne) {
    
    // Use APP or FDU message type based on parameter
    const char* appMsgType = (msgType == "FDU") ? HMS_MSG_TYPE_FD_UPDT : HMS_MSG_TYPE_APPLICATION;
    
    OneCMsg_t msgHandle = oneCMCreateInstrMsg(0, service.c_str(), productKey.c_str(), 
                                             instr.c_str(), appMsgType, deliverToOne);
    
    if (!msgHandle) {
        throw std::runtime_error("Failed to create message");
    }
    
    // NOTE: Values for byte array fields are not copied, so the data pointer set for a byte array field must point
    // to memory, that stays valid as long as the message, which contains the field, is used.
    // Therefore these static variables are used to point to permanent memory:
    static HmsUint8_t byteArray1[4] = { 0x1, 0x2, 0x3, 0x4 };
    static const char* bytes = "abcdefghijk";

    // Date and time values
    HmsDate_t date1 = { 2025, 3, 6 };
    HmsDate_t date2;
    oneCFDateFromString("1977-08-31", &date2);

    HmsTime_t time1 = { 23, 45, 30, 123456, false, -90 };
    HmsTime_t time2;
    oneCFTimeFromString("12:13:14.000123+01:15", &time2);

    HmsDatetime_t datetime1 = { date1, time1 };
    HmsDatetime_t datetime2;
    oneCFDatetimeFromString("1988-08-31T17:45:25.123-01:30", &datetime2);

    HmsDecimal_t decimal1 = { -9223372036854775807, 8 };
    HmsDecimal_t decimal2;
    oneCFDecimalFromString("123456e-2", &decimal2);

    // Add fields to the message (similar to the original C code)
    oneCMAddNullStringField(msgHandle, "CurrentTitle", false); // Will be set before publish
    oneCMAddField(msgHandle, "CurrentNumber", false, HMS_INT, 0); // Will be set before publish
    oneCMAddField(msgHandle, "EmptyString", false, HMS_STRING, "");
    oneCMAddField(msgHandle, "NullString", false, HMS_STRING, 0);

    oneCMAddInt32FieldByValue(msgHandle, "int32", false, -4);
    oneCMAddUInt32FieldByValue(msgHandle, "uint32", false, 5);
    oneCMAddNullInt32Field(msgHandle, "null-int32", false);

    oneCMAddFloatFieldByValue(msgHandle, "float1", false, 1.01f);
    oneCMAddFloatFieldByValue(msgHandle, "float2", false, 2.02f);
    oneCMAddNullFloatField(msgHandle, "null-float", false);

    oneCMAddDoubleFieldByValue(msgHandle, "double1", false, 1.123);
    oneCMAddDoubleFieldByValue(msgHandle, "double2", false, 299000.0078);
    oneCMAddNullDoubleField(msgHandle, "null-double", false);

    oneCMAddBoolFieldByValue(msgHandle, "bool1", false, true);
    oneCMAddBoolFieldByValue(msgHandle, "bool2", false, false);
    oneCMAddNullBoolField(msgHandle, "null-bool", false);

    oneCMAddByteArrayField(msgHandle, "ByteArray1", false, byteArray1, 4);
    oneCMAddByteArrayField(msgHandle, "ByteArray2", false, (HmsUint8_t*)bytes, strlen(bytes) + 1);
    oneCMAddByteArrayField(msgHandle, "Null-ByteArray", false, 0, 0);

    oneCMAddDateField(msgHandle, "Date1", false, &date1);
    oneCMAddDateField(msgHandle, "Date2", false, &date2);
    oneCMAddNullDateField(msgHandle, "Null-Date", false);

    oneCMAddField(msgHandle, "Time1", false, HMS_TIME, &time1);
    oneCMAddField(msgHandle, "Time2", false, HMS_TIME, &time2);
    oneCMAddField(msgHandle, "Null-Time", false, HMS_TIME, 0);

    oneCMAddField(msgHandle, "Datetime1", false, HMS_DATETIME, &datetime1);
    oneCMAddField(msgHandle, "Datetime2", false, HMS_DATETIME, &datetime2);
    oneCMAddField(msgHandle, "Null-Datetime", false, HMS_DATETIME, 0);

    oneCMAddDecimalField(msgHandle, "Decimal1", false, &decimal1);
    oneCMAddDecimalField(msgHandle, "Decimal2", false, &decimal2);
    oneCMAddNullDecimalField(msgHandle, "Null-Decimal", false);

    // Return the prepared message
    return msgHandle;
}

// Update message fields (title and number) before publishing
void updatePublisherMessage(OneCMsg_t msg, const std::string& title, int number) {
    if (!msg) {
        throw std::runtime_error("Invalid message handle");
    }
    
    // Update the title field (index 0)
    oneCMSetStringFieldV(msg, 0, title.c_str(), true);
    
    // Update the number field (index 1)
    oneCMSetInt32FieldByValueV(msg, 1, number);
    
    // Set status information
    HmsInstrumentState_t status = (number % 2 == 0) ? HMS_INSTR_STATE_OK : HMS_INSTR_STATE_STALE;
    std::string statusText = "Status text: " + std::to_string(status) + " (" + 
                            std::string(HmsInstrumentStateToCstr(status)) + ")";
    
    oneCMSetStatus(msg, status);
    oneCMSetStatusText(msg, statusText.c_str());
    
    // Set sequence number and timestamp
    oneCMSetAppSeqNo(msg, 1000000000000 + number);
    oneCMSetAppTimestamp(msg, time(0));
}

// Print message to console for debugging
void printMessage(OneCMsg_t msg) {
    if (!msg) {
        std::cout << "Invalid message handle (null)" << std::endl;
        return;
    }
    
    std::cout << "\nMessage (at address " << msg << ") for publication:" << std::endl;
    std::cout << "----------------------------------------------------------------------------------------------" << std::endl;
    
    oneCMPrintMsg(stdout, msg, true, 3);
    
    std::cout << "----------------------------------------------------------------------------------------------" << std::endl;
}

// Create a session for publishing
OneCSession_t createPublisherSession(
    const std::string& connectString,
    const std::string& user,
    const std::string& password,
    const std::string& appProdKey,
    bool doSubscribe,
    OneCErrInfo_t& err) {
    
    OneCSessionParams_t sessionParams = OneCSessionParams_INIT;
    OneCUserCallbacks_t cbs = OneCUserCallbacks_INIT;
    
    // Set callbacks
    cbs.onData = onData;
    
    // Set session parameters
    sessionParams.connectString = connectString.c_str();
    sessionParams.user = user.c_str();
    sessionParams.password = password.c_str();
    sessionParams.appProdKey = appProdKey.c_str();
    sessionParams.sessionNoLocal = !doSubscribe; // Must be false to receive own published messages
    
    // Create the session
    OneCSession_t session = oneCCreateSession(&sessionParams, &cbs, &err);
    if (!session) {
        throw std::runtime_error(std::string("Failed to create session: ") + err.text);
    }
    
    return session;
}

// Subscribe to receive published messages
bool subscribeForPublishing(OneCSession_t session, const std::string& service, 
                         const std::string& instr, OneCErrInfo_t& err) {
    if (!session) {
        throw std::runtime_error("Invalid session handle");
    }
    
    std::cout << "Calling oneCSubscribeInstr(" << service << ", " << instr << ")" << std::endl;
    
    bool result = oneCSubscribeInstr(session, service.c_str(), instr.c_str(), &err);
    if (!result) {
        std::cerr << "Failed to subscribe: " << err.text << std::endl;
        return false;
    }
    
    std::cout << "oneCSubscribeInstr() OK" << std::endl;
    
    // Wait for subscription to be properly established
    mySleep("Waiting for subscription to be established", 100);
    
    return true;
}

// Publish a message
bool publishMessage(OneCSession_t session, OneCMsg_t msg, bool useCStruct, OneCErrInfo_t& err) {
    if (!session || !msg) {
        throw std::runtime_error("Invalid session or message handle");
    }
    
    bool result = false;
    
    if (useCStruct) {
        CInstrPublishParams_t params;
        oneCMFillPublishParams(msg, &params);
        
        std::cout << "Calling oneCPublish()... ";
        result = oneCPublish(session, &params, &err);
    } else {
        std::cout << "Calling oneCPublishMsg()... ";
        result = oneCPublishMsg(session, msg, &err);
    }
    
    if (result) {
        std::cout << "OK" << std::endl;
    } else {
        std::cerr << "Failed: " << err.text << std::endl;
    }
    
    return result;
}

// Unsubscribe from receiving published messages
bool unsubscribeFromPublishing(OneCSession_t session, const std::string& service, 
                            const std::string& instr, OneCErrInfo_t& err) {
    if (!session) {
        throw std::runtime_error("Invalid session handle");
    }
    
    std::cout << "Calling oneCUnsubscribeInstr(" << service << ", " << instr << ")" << std::endl;
    
    bool result = oneCUnsubscribeInstr(session, service.c_str(), instr.c_str(), &err);
    if (!result) {
        std::cerr << "Failed to unsubscribe: " << err.text << std::endl;
        return false;
    }
    
    std::cout << "OK" << std::endl;
    
    return true;
}

// Destroy a message
bool destroyMessage(OneCMsg_t msg) {
    if (!msg) {
        return false;
    }
    
    std::cout << "Calling oneCDestroyMsg(" << msg << ")" << std::endl;
    
    bool result = oneCMDestroyMsg(msg);
    if (result) {
        std::cout << "Message destroyed successfully" << std::endl;
    } else {
        std::cerr << "Failed to destroy message" << std::endl;
    }
    
    return result;
}

// Set TLS parameters for secure connections
void setTLSParams(const std::string& trustStoreDir) {
    oneCSetTLSTrustParams(trustStoreDir.c_str(), true);
}

