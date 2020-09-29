#include <core/string.h>

namespace kvm {

    std::string WideStringToString(const wchar_t* input) {
        return WideStringToString(std::wstring(input));
    }

    std::string WideStringToString(const std::wstring& input) {
        return std::string(input.begin(), input.end());
    }
}