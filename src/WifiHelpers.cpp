
/*
0000   48 54 54 50 2f 31 2e 31 20 32 30 30 20 4f 4b 0d  HTTP/1.1 200 OK.
0010   0a 43 6f 6e 74 65 6e 74 2d 54 79 70 65 3a 20 61  .Content-Type: a
0020   70 70 6c 69 63 61 74 69 6f 6e 2f 6a 73 6f 6e 0d  pplication/json.
0030   0a 43 6f 6e 74 65 6e 74 2d 4c 65 6e 67 74 68 3a  .Content-Length:
0040   20 33 0d 0a 43 6f 6e 6e 65 63 74 69 6f 6e 3a 20   3..Connection:
0050   63 6c 6f 73 65 0d 0a 41 63 63 65 73 73 2d 43 6f  close..Access-Co
0060   6e 74 72 6f 6c 2d 41 6c 6c 6f 77 2d 4f 72 69 67  ntrol-Allow-Orig
0070   69 6e 3a 20 2a 0d 0a 0d 0a                       in: *....

0000   38 30 32                                         802
*/

#include <WiFiClient.h>

void handleWiFiClient(WiFiClient client){
  while(!client.available()){
    digitalWrite(4, HIGH);
    delay(1);

    digitalWrite(4 , LOW);
    delay(1);
  }

  String request = client.readStringUntil('\r');
  client.flush();

  String content = "803";
  String contentLength = String(content.length(), DEC);

  String response = "";
  response += "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: application/json\r\n";
  response += "Content-length : "+contentLength+"\r\n";
  response += "Connection: close\r\n";
  response += "Access-Control-Allow-Origin: *\r\n";
  response += "\r\n";
  response += content;

  client.write(response.c_str(), response.length());
}

int computeRollingAverage(int rollingAverage, int index, int newValue, int scale){
  return (rollingAverage*(index-1))/index + (scale*newValue)/index;
}
