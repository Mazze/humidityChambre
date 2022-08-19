
#ifndef SSLCERTSTORE_H
#define SSLCERTSTORE_H

class SSLCertStore 
{
    private:
        static SSLCertStore* _me;
        String myCerts ;
        SSLCertStore();
    public:
        static SSLCertStore* getInstance()
                                {if (_me== 0)
                                    {_me= new SSLCertStore ();}
                                return _me;};
        bool loadCertsFromFile(String Filename);
        const char* getCerts( );
};

#endif