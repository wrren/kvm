#ifndef KVM_TIME_H
#define KVM_TIME_H

#include <chrono>

namespace kvm {
  /**
   * Utility class for spacing some actions out over time.
   */
  class TimeSpacer {
  public:

    /**
     * Default Constructor
     */
    TimeSpacer();

    /**
     * Determine whether the given duration passed since the last time an action was triggered. Updates
     * the internal time-point used for comparison if the given duration or more has passed.
     */
    bool operator()(const std::chrono::milliseconds duration);

  private:

    /// Last tick time
    std::chrono::time_point<std::chrono::system_clock> m_last;
  };

}

#endif // KVM_TIME_H