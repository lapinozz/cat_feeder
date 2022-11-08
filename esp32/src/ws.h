AsyncWebSocket ws("/ws/" __SECRET__);

using WsMsgCallback = void (*)(uint32_t, const char*);
WsMsgCallback wsMsgCallback;

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
    {
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
      {
        data[len] = 0;
        wsMsgCallback(client->id(), (const char*)data);
      }
      break;
    }
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void setupWs(AsyncWebServer& server, WsMsgCallback callback)
{
  wsMsgCallback = callback;

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
}

class AsyncWebSocketMessageBufferInline
{
  private:
    uint8_t* _data;
    size_t _len;
    bool _lock; 
    uint32_t _count;  

  public:
    AsyncWebSocketMessageBufferInline(uint8_t * data, size_t size)  
    :_data(data)
    ,_len(size)
    ,_lock(false)
    ,_count(0)
    {

    }
    void lock() { _lock = true; }
    void unlock() { _lock = false; }
    uint8_t * get() { return _data; }
    size_t length() { return _len; }
    uint32_t count() { return _count; }
};