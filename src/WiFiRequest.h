#ifndef WiFiRequest_h
#define WiFiRequest_h

#include "Arduino.h"
#include <WiFiClient.h>

class WiFiRequest
{
  public:
    WiFiRequest(WiFiClient body);

    String getMethod();
    String getUri();
    String getVersion();
    String getHeaderOrDefault(String name, String defaultValue);
    String getParameterOrDefault(String name, String defaultValue);
    String getBody();
  private:
    String _method;
    String _uri;
    String _version;
    String _headerNames[];
    String _headerValues[];
    int _headersLength;

    String _parameterNames[];
    String _parameterValues[];
    int _parametersLength;
    byte _body[];
};

#endif
