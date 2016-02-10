//int sendHTTP2(const char* host, const char* method, const char *url, const char* headers, const char* postData, boolean secure, boolean sendHeaders) {
//  WiFiClient *client = createHTTPClient(host, secure);
//  if (!client) return -1;
//  sendHTTPRequest(client, host, method, url, headers, postData, sendHeaders);      
//  if (waitForResponse(client, 3000)) {
//    int responseCode = getResponseCode(client);
//    delete client;
//    Serial << endl << "CLOSED" << endl;
//    return responseCode;
//  } else {
//    delete client;
//    Serial << "Timeout !" << endl;
//    return -2;
//  }
//}
//
//WiFiClient *createHTTPClient(const char* host, boolean secure) {
//  WiFiClient *client;
//  if (secure) client = new WiFiClientSecure();
//  else client = new WiFiClient();
//  const int httpPort = secure? 443 : 80;
//  
//  Serial << "Connecting to: " << host << ":" << httpPort << endl;
//  if (!client->connect(host, httpPort)) {
//    Serial.println("connection failed");
//    return NULL;
//  }
//  return client;
//}
//
//void sendHTTPRequest(WiFiClient *client, const char* host, const char* method, const char *url, const char* headers, const char* postData, boolean sendHeaders) {
//  String rq = buildRequestHeader(host, method, url, headers, postData, sendHeaders);
//  //  heap("after build request");
//  Serial << "Requesting [" << endl << rq << "]" << endl;
//  client->print(rq);
//  if (postData) client->print(postData);
//}
//
//String buildRequestHeader(const char* host, const char* method, const char *url, const char* headers, const char* postData, boolean sendHeaders) {
//  String rq = String(method) + " " + url + (sendHeaders?" HTTP/1.1":"") + "\r\n";
//  if (sendHeaders)
//     rq += String("Host: ") + host + "\r\n" + 
//           (postData? (String("Content-Length: ") + strlen(postData) + "\r\n") : "") +
//           (headers ? headers : "");
//   rq += "\r\n";
//   return rq;
//}
//
//boolean waitForResponse(WiFiClient *client, int timeout) {
//  Serial << "Waiting Respomse " << endl;
//  int t = 0;
//  while (!client->available()) {
//    if ((t += 10) > timeout) break;
//    delay(10);
//  }
//  Serial << "Response time: " << t << endl ;  
//  return t < timeout;
//}
//
//int getResponseCode(WiFiClient *client) {
//  Serial << endl;
//  char buf[201];
//  int responseCode = -1;
//  while(client->available()){
//    buf[client->readBytes(buf, 200)] = 0;
//    if (responseCode == -1) responseCode = detectResponseCode(buf);
//    if (traceHttp) Serial << (buf);
//  }
//  Serial << endl << "--------- done, rc=" << responseCode << endl;
//  return responseCode;  
//}
//
//int detectResponseCode(char *buf) {
//  char *ch = strchr(buf, ' ');
//  if (ch) {
//    char *ch2 = strchr(ch+1, ' ');
//    if (ch2) {
//      char res[4];
//      strncpy(res, ch+1, 3);
//      res[3] = 0;
//      return atoi(res);
//    }
//  }
//  return 0;
//}
