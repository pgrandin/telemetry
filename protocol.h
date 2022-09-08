#pragma pack(1)

enum class MessageType : uint8_t {
    ACK = 0,
    HEARTBEAT = 1,
    GPS = 2,
    LAP = 3
};

struct message_t {
    MessageType type;
    int8_t rssi;
    uint8_t snr;
    float latitude;
    float longitude;
    int32_t checksum;
};

enum class MessageStatus : uint8_t {
    PENDING = 0,
    INFLIGHT = 1,
    ACKED = 2
};

struct queued_message_t {
    MessageStatus status;
    message_t msg;
};

