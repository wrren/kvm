#ifndef KVM_CLUSTER_H
#define KVM_CLUSTER_H

#include <display/display.h>

namespace kvm {
    class Cluster {
    public:

        /**
         * Default Constructor
         */
        Cluster();

        /**
         * Add a new node to the cluster.
         */
        void AddNode(const std::string& hostname);

        /**
         * Request that connected nodes trigger an input change to the specified display input.
         */
        void RequestInputChange(Display::Input input);

    private:
    };
}

#endif // KVM_CLUSTER_H