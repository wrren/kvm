#include <core/time.h>

namespace kvm {
  TimeSpacer::TimeSpacer() :
  m_last(std::chrono::system_clock::now())
  {}

  bool TimeSpacer::operator()(const std::chrono::milliseconds duration) {
    if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_last) >= duration) {
      m_last = std::chrono::system_clock::now();
      return true;
    }
    return false;
  }
}