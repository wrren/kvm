#include <networking/message/heartbeat.h>
#include <networking/message/types.h>

namespace kvm {
  Heartbeat::Heartbeat() :
  NetworkMessage(static_cast<NetworkMessage::Type>(NetworkMessageType::HEARTBEAT))
  {}
}