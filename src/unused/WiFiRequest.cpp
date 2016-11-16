/*
Request       = Request-Line              ; Section 5.1
                *(( general-header        ; Section 4.5
                 | request-header         ; Section 5.3
                 | entity-header ) CRLF)  ; Section 7.1
                CRLF
                [ message-body ]          ; Section 4.3

Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
*/

/*
because ESP8266WebServer.handleClient() took so little that toggling a speaker on/off left it always on I thought that the call was blocking and so I wrote this.
I spent a bunch of hours working on this so I can't bring myself to delete it yet.

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

  int headersLength = 1;
  String* headerNames = (String*) malloc(headersLength * sizeof(String));
  String* headerValues = (String*) malloc(headersLength * sizeof(String));
  int contentLength = 0;

  // Read headers
  int index=0;
  for(; body.peek() >= 0 && body.peek() != '\r'; index++){
    if(index == headersLength){
      headersLength *= 2;
      String* newHeaderNames = (String*) malloc(headersLength * sizeof(String));
      String* newHeaderValues = (String*) malloc(headersLength * sizeof(String));

      for(int i=0;i<(headersLength/2);i++){
        newHeaderNames[i] = headerNames[i];
        newHeaderValues[i] = headerValues[i];
      }
      delete[] headerNames;
      delete[] headerValues;

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
    this->_body = (byte*)  malloc(contentLength * sizeof(contentLength));
    this->_bodyLength = contentLength;
    body.readBytes(this->_body, this->_bodyLength);
  }else{
    this->_body = (byte*) malloc(0 * sizeof(contentLength));
    this->_bodyLength = 0;
  }
}

String WiFiRequest::getJsonString(){
  String json = "";
  json += "{ method = " + this->_method;
  json += ", uri = " +this->_uri;

}
