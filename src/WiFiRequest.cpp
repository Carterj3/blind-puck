/*
Request       = Request-Line              ; Section 5.1
                *(( general-header        ; Section 4.5
                 | request-header         ; Section 5.3
                 | entity-header ) CRLF)  ; Section 7.1
                CRLF
                [ message-body ]          ; Section 4.3

Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
*/


// https://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html

#include "Arduino.h"
#include "WiFiRequest.h"

WiFiRequest::WiFiRequest(WiFiClient body){
  // Read request line
  String requestLine = body.readStringUntil('\r');
  body.read();

  int methodSp = requestLine.indexOf(' ', 0);
  int uriSp = requestLine.indexOf(' ', methodSp);
  int versionSp = requestLine.length();

  this->_method = requestLine.substring(0, methodSp);
  this->_uri = requestLine.substring(methodSp, uriSp);
  this->_version = requestLine.substring(uriSp, versionSp);

  String headerNames[1];
  int headersLength = 1;
  String headerValues[1];
  int contentLength = 0;

  // Read headers
  int index=0;
  for(; body.peek() >= 0 && body.peek() != '\r'; index++){
    if(index == headersLength){
      headersLength *= 2;
      String newHeaderNames[headersLength];
      String newHeaderValues[headersLength];

      for(int i=0;i<(headersLength/2);i++){
        newHeaderNames[i] = headerNames[i];
        newHeaderValues[i] = headerValues[i];
      }
      delete headerNames;
      delete headerValues;

      headerNames = newHeaderNames;
      headerValues = newHeaderValues;
    }
    headersLength = index;

    String header = body.readStringUntil('\r');
    body.read();

    int headerColon = header.indexOf(':',0);
    String headerName = header.substring(0, headerColon);
    headerName.trim();
    String headerValue = header.substring(headerColon, header.length());
    headerValue.trim();

    headerNames[index] = headerName;
    headerValues[index] = headerValue;

    if(headerName.equalsIgnoreCase("content-length")){
      contentLength = headerValue.toInt();
    }
  }

  this->_headersLength = headersLength;
  this->_headerNames = headerNames;
  this->_headerValues = headerValues;

  // Read body
  if(contentLength > 0){
    byte bodyContent[contentLength];
    body.readBytes(body, bodyContent, contentLength);
  }
}
