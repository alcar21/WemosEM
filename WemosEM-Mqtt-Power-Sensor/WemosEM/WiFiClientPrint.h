#pragma once

#include <WiFiClient.h>
#include <Print.h>

template<size_t BUFFER_SIZE = 32>

class WiFiClientPrint : public Print
{
  public:
    WiFiClientPrint(WiFiClient client)
      : _client(client),
        _length(0)
    {
    }

    ~WiFiClientPrint()
    {
#ifdef DEBUG_ESP_PORT
      // Note: This is manual expansion of assertion macro
      if (_length != 0) {
        DEBUG_ESP_PORT.printf("\nassertion failed at " __FILE__ ":%d: " "_length == 0" "\n", __LINE__);
        // Note: abort() causes stack dump and restart of the ESP
        abort();
      }
#endif
    }

    virtual size_t write(uint8_t c) override
    {
      _buffer[_length++] = c;
      if (_length == BUFFER_SIZE) {
        flush();
      }
    }

    void flush()
    {
      if (_length != 0) {
        _client.write((const uint8_t*)_buffer, _length);
        _length = 0;
      }
    }

    void stop()
    {
      flush();
      _client.stop();
    }

  private:
    WiFiClient _client;
    uint8_t _buffer[BUFFER_SIZE];
    size_t _length;
};