#pragma pack(1)

enum class MessageType : uint8_t {
    ACK = 0,
    HEARTBEAT = 1,
    DATA = 2
};

struct message_t {
    MessageType type;
    int8_t rssi;
    uint8_t snr;
    float latitude;
    float longitude;
};
