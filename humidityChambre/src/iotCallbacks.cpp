#include "iotCallbacks.h"
#include "configObjectStruct.h"
#include "azure/common/string_buffer.h"
#include "azure/common/iotc_json.h"
#include"logging.h"

#define MAXPAYLOADSIZE 1800

static IOTContext g_iotContex ;

void iotc_loop()
{
    iotc_do_work(g_iotContex);
    delay(100);  
}

void sendNewTelemetry(stateStruct* state)
{
  Serial.print("Start Telemetry");
  char msg[256] = {0};
  int pos = 0, errorCode = 0;     
 pos = snprintf(msg, sizeof(msg) - 1, "{\"%s\": %3f,\"%s\": %3f",
                   tHumidity,state->humidity ,tTemperature,state->temperature);
                   
 errorCode = iotc_send_telemetry(g_iotContex, msg, pos);          
  if (errorCode != 0) {
    LOG_ERROR("Sending message has failed with error code %d", errorCode);
 }
}
void reportNewState(stateStruct* state)
{   
   char msg [256];
   uint pos; 
   pos = snprintf(msg, sizeof(msg) - 1, 
                    "{\"%s\":%d,\"%s\":%d,\"%s\":%d}",
                                pDoorOpenState,state->isDoorOpen,
                                pVaporiserOnState,state->vaporizerOn,
                                pDeHumidifierOnState, state->DehumidifyOn);
   LOG_VERBOSE("PropertySend: %s",msg);
   state->stateNeedsReport=false;
   auto errorCode = iotc_send_property(g_iotContex, msg, pos); 
   sendNewTelemetry( state);
}

void reportNewProperties()
{   
    LOG_VERBOSE("reportNewProperties:");
    char msg [MAXPAYLOADSIZE];   
    char buffer [300] = {0};
    uint pos; 
    pos = snprintf(msg, sizeof(msg), "{");
    for (int i =0;i<numberOfTag;i++)
    {
        // LOG_VERBOSE("Setting : %s",settingTags[i]);
        if (!getConfigFromSettingAsString(settingTags[i], buffer, sizeof(buffer)))
            continue;
        if (buffer[0]==0)
            continue;
        // LOG_VERBOSE("value : %s",buffer);
        pos += snprintf(msg+pos, sizeof(msg)-pos , 
                "\"%s\":%s,",settingTags[i],buffer );                
        // LOG_VERBOSE("Pos : %d; Msg: %s",pos,msg);
    }
    if (pos >2)
    {
        msg[pos-1]='}';
        LOG_VERBOSE("PropertySend: %s",msg);
        auto errorCode = iotc_send_property(g_iotContex, msg, pos);         
    }
    else
        LOG_VERBOSE("Didn't find settings to report");
}
void on_IOTCMessage(IOTContext ctx, IOTCallbackInfo* callbackInfo) 
{
    // AzureIOT::StringBuffer buffer;
    // if (callbackInfo->payloadLength > 0) {
    //     buffer.initialize(callbackInfo->payload, callbackInfo->payloadLength);
    // }
    // LOG_VERBOSE("- [%s] Message was received. Payload => %s\n",
    //          callbackInfo->eventName, buffer.getLength() ? *buffer : "EMPTY");
}



void on_IOTCSettings(IOTContext ctx, IOTCallbackInfo* callbackInfo) 
{
    // Process Settings and properties
    LOG_VERBOSE("Got Setting");
    jsobject_t object;
    jsobject_initialize(&object, callbackInfo->payload,
                     callbackInfo->payloadLength);
    LOG_VERBOSE( callbackInfo->payload);
    if (strcmp(callbackInfo->tag, "twin") == 0)
    {
        //We have an connection estatbilshed and get the settins at the start up
        const char *echoTemplate = "{\"%s\":%s}";
        bool update =false;
        char msg[300] = {0};
        int pos = 0, errorCode = 0;
        char value[256] = {0};
        jsobject_t desired;// This should be need to adobed
        jsobject_t reported;// Make sure it is the same as before
        jsobject_get_object_by_name(&object, "reported", &reported);
        jsobject_get_object_by_name(&object, "desired", &desired);
        // List all token 
        jsobject_t* uo =& desired;
        LOG_VERBOSE("Number of token %d",desired.tokenCount);
        for (int i = 1; i < uo->tokenCount; i+=2) {              
            //test all token
            bool update = false; 
            char propertyName[30] = {0}; 
            
            jsmntok_t token = uo->tokens[i ];
            unsigned n = (token.end - token.start);
            const char* start = (uo->json + token.start); 
            if (*start == '$' )
                continue;
            
            jsmntok_t tokenData = uo->tokens[i+1 ];    
            unsigned nD = (tokenData.end - tokenData.start);
            const char* startD = (uo->json + tokenData.start);
            memset(propertyName,0,sizeof(propertyName));
            memset(value,0,sizeof(value));
            memcpy(propertyName, start, n);            
            value[0] = ' ';            
            LOG_VERBOSE(" Token propertyName %s ", propertyName);
            
            
            char* buffer =msg;
            if (getConfigFromSettingAsString(propertyName, buffer, sizeof(buffer)))
            {                
                // The propertyName is know, lets deal with it                                        
                // LOG_VERBOSE("Soll: %s ; haben: %s", startD,buffer);
                char* reportedValue = jsobject_get_string_by_name(&reported, propertyName);
                if (strncmp(startD, buffer, nD) != 0) 
                {
                    // The desire is different from the config
                    if ( *(startD-1) == '\"')
                    {
                        // Include the " in the json                    
                        startD--;
                        nD+=2;
                    }
                    memset(value,0,sizeof(value));
                    memcpy(value, startD, nD);
                    update = true;
                    LOG_VERBOSE("%s => value %s \n", propertyName,value);                    
                }
                else if ((reportedValue!=NULL) || (strncmp(startD, reportedValue, nD) != 0) )
                {                    
                    //We have not reported the currently used value
                    IOTC_FREE( reportedValue); 
                    memset(value,0,sizeof(value));
                    memcpy(value, buffer, strlen(buffer));
                    update = true;
                    LOG_VERBOSE(" => value %s \n", value);
                }          
            //     break;    
            //     }
            // }
                if (update)
                {                    
                    pos = snprintf(msg, sizeof(msg) - 1, "{\"%s\":%s}",
                                propertyName,value );
                    // LOG_VERBOSE("Json to send :%s",msg);
                    //auto errorCode = iotc_send_property(g_iotContex, msg, pos); 
                    if ((value[0]=='\"')&&(value[strlen(value)-1]=='\"'))
                    {
                        uint len =strlen(value);
                        memcpy(value,value+1,len-2);
                        // memcpy(value,msg,strlen(value-2));
                        value[len-2]=0;
                        LOG_VERBOSE("Removed \" :|%s|",value);     
                    }
                    LOG_VERBOSE("Value to save :|%s|",value);                
                    setConfigFromSettingAsString(propertyName, value,&g_configObject);                    
                }
            }
            else
            {
                //Remove unused settings Doesn not work, probably only for report values

                // pos = snprintf(msg, sizeof(msg) - 1, "{\"%s\":null}",
                //                 propertyName );
                // LOG_VERBOSE("Json to send :%s",msg);
                // auto errorCode = iotc_send_property(g_iotContex, msg, pos);

            }
        }
        reportNewProperties();

        //Check all the report only 
    
        LOG_VERBOSE("settingTagsReportOnly size :%d",numberOfProperties);
        //No need to report is all one by one.
        reportNewState(&currentState);
        // for (int k=0;k<numberOfProperties;k++)
        // {
        //     LOG_VERBOSE("%s",propertyTags[k]);
        //     char* reportedValue = jsobject_get_string_by_name(&reported,propertyTags[k] );
            
        //     getStateAsString(propertyTags[k],value, sizeof(value));
        //     if ((reportedValue!=NULL)&&(strcmp(reportedValue,value)!=0))
        //     {
        //         if ((reportedValue[0]=='\"')&&(reportedValue[strlen(reportedValue)-1]=='\"'))
        //             pos = snprintf(msg, sizeof(msg) - 1, "{\"%s\":\"%s\"}",
        //                         propertyTags[k],value );
        //         else
        //         {
        //             pos = snprintf(msg, sizeof(msg) - 1, "{\"%s\":%s}",
        //                         propertyTags[k],value );
        //         }
        //         LOG_VERBOSE("Json to send :%s",msg);
        //         auto errorCode = iotc_send_property(g_iotContex, msg, pos);
        //     }
        //     else if (reportedValue==NULL)
        //     {
        //         LOG_VERBOSE("Report non previous");
        //         if (((value[0]>=48)&&(value[0]<=39))||((value[0]==45)))
        //             pos = snprintf(msg, sizeof(msg) - 1, "{\"%s\":\"%s\"}",
        //                         propertyTags[k],value );
        //         else
        //         {
        //             pos = snprintf(msg, sizeof(msg) - 1, "{\"%s\":%s}",
        //                         propertyTags[k],value );
        //         }
        //         LOG_VERBOSE("Json to send :%s",msg);
        //         auto errorCode = iotc_send_property(g_iotContex, msg, pos);
        //     }
        // }
        LOG_VERBOSE("Done Rport");

    }
    else
    {   
        LOG_VERBOSE("Setting While running");
        char* value =jsobject_get_string_by_name(&object, callbackInfo->tag);
        if (value !=NULL)
        {        
            setConfigFromSettingAsString(callbackInfo->tag, value,&g_configObject);
            IOTC_FREE (value);
        }

    }
    LOG_VERBOSE("Update Current config");
    printConfig(&g_configObject);
    // getStoredConfig(&g_configObject );
}

void on_IOTCStatus(IOTContext ctx, IOTCallbackInfo* callbackInfo) 
{
    LOG_VERBOSE("Received a ConnectionStatus");
    if (strcmp(callbackInfo->eventName, "ConnectionStatus") == 0) {
        LOG_VERBOSE("Is connected ? %s (%d)",
               callbackInfo->statusCode == IOTC_CONNECTION_OK ? "YES" : "NO",
               callbackInfo->statusCode);
        // isConnected = callbackInfo->statusCode == IOTC_CONNECTION_OK;
        currentState.isAzureConnected = callbackInfo->statusCode == IOTC_CONNECTION_OK;
        currentState.needReconnectAzure = !currentState.isAzureConnected;        
        Serial.println("currentState.isAzureConnected");
        Serial.println(currentState.isAzureConnected)    ;    
        return;
    }
}

void on_IOTCCommand(IOTContext ctx, IOTCallbackInfo* callbackInfo) {}

void on_IOTCError(IOTContext ctx, IOTCallbackInfo* callbackInfo)
{
    LOG_ERROR("IOTC Error ");
}

bool connect_client(const char* scopeId, const char* deviceId,
                    const char* deviceKey)
{
    // initialize iotc context (per device client)
    int errorCode = iotc_init_context(&g_iotContex);
    if (errorCode != 0) {
        LOG_ERROR("Error initializing IOTC. Code %d", errorCode);
        return false ;
    }

    iotc_set_logging(IOTC_LOGGING_API_ONLY);

    // set up event callbacks. they are all declared under the ESP8266.ino file
    // for simplicity, track all of them from the same callback function
    iotc_on(g_iotContex, "MessageSent", on_IOTCMessage, NULL);
    iotc_on(g_iotContex, "Command", on_IOTCCommand, NULL);
    iotc_on(g_iotContex, "ConnectionStatus", on_IOTCStatus, NULL);
    iotc_on(g_iotContex, "SettingsUpdated", on_IOTCSettings, NULL);
    iotc_on(g_iotContex, "Error", on_IOTCError, NULL);
    // connect to Azure IoT
    Serial.print("|"); Serial.print(scopeId); Serial.println("|");
    Serial.print("|"); Serial.print(deviceId); Serial.println("|");
    Serial.print("|"); Serial.print(deviceKey); Serial.println("|");
    errorCode = iotc_connect(g_iotContex, scopeId, deviceKey, deviceId,
                            IOTC_CONNECT_SYMM_KEY);
    if (errorCode != 0) {
        LOG_ERROR("Error @ iotc_connect. Code %d", errorCode);
        return false ;
    }
    return true;
}
