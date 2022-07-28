#ifndef WIFICONFIGSERVER_H
#define WIFICONFIGSERVER_H

#include <WebServer.h>



class wifiConfigServer 
{
    WebServer* server;//(80);

    private :
        void handlePlain();

    public:

    void handleHtml(String path)
        {}
    void handlePostConfig();
    void handleGetConfig();
    void handleGetDeviceInfo();
    void handlePostAction();
    bool handleFileRead(String assetFile) ;

    void handleNotFound();

    void replyBadRequest(String msg) ;

     
    void setupServer(int port = 80);

    void loopItem(unsigned long lastTick);
};

#endif