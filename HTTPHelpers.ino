boolean traceHttp = true;

String HTTP_STR = "http://";
String HTTPS_STR= "https://";

int processResponseCodeATFW(HTTPClient *http, int rc) {
  if (rc > 0) Serial << "Response Code: " << rc << endl;
  else Serial << "Error Code: " << rc << " = " << http->errorToString(rc).c_str() << endl;
  if (rc > 0) {
    Serial << "Payload: [" << http->getString() << "]" << endl;
    Serial << "CLOSED" << endl; // for compatibility with AT FW
  } else {
    Serial << "Failed" << endl;
  }
  return rc;
}

//int sendHTTP1(const char* host, const char* method, const char *url, Header* headers, const char* postData, boolean secure, boolean sendHeaders) {
//  HTTPClient http;
//  http.begin((secure ? HTTPS_STR : HTTP_STR) + host  + url);
//  int rc = http.sendRequest(method, (uint8_t*)postData, postData ? strlen(postData) : 0);
//  for (int i=0; i < headers.length; i++)  http.addHeader(headers[i][0], headers[i][1]);
//  Serial << "Response Code: " << rc << endl;
//  if (rc > 0) {
//    Serial << "Payload: [" << http.getString() << "]" << endl;
//    Serial << "CLOSED" << endl; // for compatibility with AT FW
//  } else {
//    Serial << "Failed" << endl;
//  }
//  return rc;
//}


