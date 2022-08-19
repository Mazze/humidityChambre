
#ifndef IOTCALLBACKS_H
#define IOTCALLBACKS_H

#include "azure/iotc.h"
#include "globals.h"


void sendNewTelemetry(stateStruct* state);

void on_IOTCMessage(IOTContext ctx, IOTCallbackInfo* callbackInfo) ;

void on_IOTCSettings(IOTContext ctx, IOTCallbackInfo* callbackInfo) ;

void on_IOTCStatus(IOTContext ctx, IOTCallbackInfo* callbackInfo) ;

void on_IOTCCommand(IOTContext ctx, IOTCallbackInfo* callbackInfo) ;

void on_IOTCError(IOTContext ctx, IOTCallbackInfo* callbackInfo);

bool connect_client(const char* scopeId, const char* deviceId,
                    const char* deviceKey);

void iotc_loop();
#endif