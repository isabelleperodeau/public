/**
 * one_subscriber.cpp
 * Subscriber functionality for One API bindings to Python using Pybind11
 */

 #include <pybind11/pybind11.h>
 #include <iostream>
 #include <string>
 #include <thread>
 #include <chrono>
 #include "auto_includes.h"
 #include "one_base.h"
 
 namespace py = pybind11;

// Subscriber-specific functions
 bool subscribeInstr(OneCSession_t session, const std::string& service, const std::string& instr, OneCErrInfo_t& err) {
     bool result = oneCSubscribeInstr(session, service.c_str(), instr.c_str(), &err);
     if (result) {
         std::cout << "Calling oneCSubscribeInstr(" << service << ", " << instr << ")\n";
         mySleep("oneCSubscribeInstr() called, waiting some time for messages", 5000);
     }
     return result;
 }
 
 bool unsubscribeInstr(OneCSession_t session, const std::string& service,
                      const std::string& instr, OneCErrInfo_t& err) {
     bool result = oneCUnsubscribeInstr(session, service.c_str(), instr.c_str(), &err);
     if (result) {
         std::cout << "Calling oneCUnsubscribeInstr(" << service << ", " << instr << ")\n";
         std::cout << "oneCUnsubscribeInstr() called, waiting some time to see whether receipt of messages has stopped, sleeping 2000 millis ...\n";
         std::this_thread::sleep_for(std::chrono::milliseconds(2000));
         std::cout << "sleep finished\n";
     }
     return result;
 }
 
 // Helper function to print App Products
 static void printAppProducts(const AppProduct_t* prods, int num) {
     if (!prods) {
         puts("no app products for non-MECS-session");
         return;
     }
 
     puts("######################################## app products ########################################");
     printf("number = %d, { <service>, <product key>, isFeed, isMonitorable, isOnBehalfFeed, isOnDemandFeed }\n", num);
 
     for (size_t i = 0; i < (size_t)num; ++i) {
         printf("{ <%s>, <%s>, %d, %d, %d, %d }\n",
                prods[i].service,
                prods[i].productKey,
                prods[i].isFeed,
                prods[i].isMonitorable,
                prods[i].isOnBehalfFeed,
                prods[i].isOnDemandFeed);
     }
 }
 
 bool sendOnDemandFeedRequest(OneCSession_t session, const std::string& service, 
     const std::string& instr, OneCErrInfo_t& err, bool async = false) {
     printf("Calling oneCOnDemandFeedRequest(%s, %s)\n", service.c_str(), instr.c_str());
 
     bool result = oneCOnDemandFeedRequest(session, service.c_str(), instr.c_str(), async, &err);
     if (result) {
         puts("oneCOnDemandFeedRequest() called");
     } else {
         std::cerr << "Failed to send on-demand feed request: " << err.text << std::endl;
     }
     return result;
 }
 
 bool sendCacheRequest(OneCSession_t session, const std::string& service, 
     const std::string& instr, const std::string& fields,
     OneCErrInfo_t& err, bool async = false) {
     printf("Calling oneCCacheRequest(%s, %s, %s)\n", 
     service.c_str(), instr.c_str(), fields.c_str());
 
     const char* fields_ptr = fields.empty() || fields == "0" ? nullptr : fields.c_str();
     bool result = oneCCacheRequest(session, service.c_str(), instr.c_str(), 
                     fields_ptr, async, &err);
     if (result) {
         puts("oneCCacheRequest() called");
     } else {
         std::cerr << "Cache request failed: " << err.text << std::endl;
     }
     return result;
 }
 
 bool sendOnBehalfRequest(OneCSession_t session, const std::string& service,
     const std::string& identifier, int reqType,
     OneCErrInfo_t& err, bool async = false) {
     printf("Calling oneCOnBehalfRequest(%s, %s, %d)\n",
     service.c_str(), identifier.c_str(), reqType);
 
     bool result = oneCOnBehalfRequest(session, service.c_str(), identifier.c_str(),
                     (HmsOnBehalfType_t)reqType, async, &err);
     if (result) {
         puts("oneCOnBehalfRequest() called");
     } else {
         std::cerr << "On-behalf request failed: " << err.text << std::endl;
     }
     return result;
 }
 
 bool endOnBehalfSubscription(OneCSession_t session, const std::string& service,
     const std::string& identifier, OneCErrInfo_t& err) {
     printf("Calling oneCEndOnBehalfSubscription(%s, %s)\n",
     service.c_str(), identifier.c_str());
 
     bool result = oneCEndOnBehalfSubscription(session, service.c_str(),
                             identifier.c_str(), &err);
     if (result) {
         puts("oneCEndOnBehalfSubscription() called");
     } else {
         std::cerr << "End on-behalf subscription failed: " << err.text << std::endl;
     }
     return result;
 }
