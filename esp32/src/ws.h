#include "AsyncTCP.h"

AsyncWebSocket ws("/ws/" __SECRET__);

using WsMsgCallback = void (*)(uint32_t, const char*);
WsMsgCallback wsMsgCallback = nullptr;

using WsConnectCallback = void (*)(uint32_t);
WsConnectCallback wsConnectCallback = nullptr;

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      client->client()->setRxTimeout(5);
      client->keepAlivePeriod(1);
      wsConnectCallback(client->id());
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
        if(strcmp((const char*)data, "ping") == 0)
        {
          break;
        }

        wsMsgCallback(client->id(), (const char*)data);
      }
      break;
    }
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

StaticTimer_t wsUpkeepTimerBuffer;
TimerHandle_t wsUpkeepTimerHandle;
void wsUpkeep(TimerHandle_t timerHandle)
{
}

void setupWs(AsyncWebServer& server, WsMsgCallback onMsg, WsConnectCallback onConnect)
{
  wsMsgCallback = onMsg;
  wsConnectCallback = onConnect;

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  wsUpkeepTimerHandle = xTimerCreateStatic("Ws Upkeep", pdMS_TO_TICKS(1000), pdTRUE, nullptr, &wsUpkeep, &wsUpkeepTimerBuffer);
}