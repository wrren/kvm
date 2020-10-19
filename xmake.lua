if not is_os("windows") then
  add_requires("libusb")
end

add_requires("catch2")

target("kvm")
  set_kind("binary")
  set_languages("cxx17")
  add_files("src/*.cpp", "src/core/**.cpp", "src/usb/**.cpp", "src/display/**.cpp", "src/networking/**.cpp")
  add_includedirs("$(projectdir)/include")
  add_rules("mode.debug")

  if is_os("windows") then
    add_files("src/platform/windows/*.cpp")
    add_defines("KVM_OS_WINDOWS", "UNICODE")
    add_syslinks("gdi32", "msimg32", "user32", "Dxva2", "Setupapi", "Advapi32")
  end

  if is_os("linux") then
    add_files("src/platform/linux/*.cpp")
    add_files("src/platform/unix/*.cpp")
    add_packages("libusb")
    add_defines("KVM_OS_LINUX")
  end

  if is_os("macosx") then
    add_files("src/platform/mac/*.cpp")
    add_files("src/platform/unix/*.cpp")
    add_packages("libusb")
    add_frameworks("CoreGraphics", "AppKit", "DriverKit", "IOKit", "CoreFoundation", "Foundation")
    add_defines("KVM_OS_MAC")
    add_ldflags("-lobjc")
  end

target("kvm_test")
  set_kind("binary")
  set_languages("cxx17")
  add_files("src/kvm.cpp", "src/core/**.cpp", "src/usb/**.cpp", "src/display/**.cpp", "src/networking/**.cpp", "test/**.cpp")
  add_includedirs("$(projectdir)/include")
  add_rules("mode.debug")
  add_packages("catch2")

  if is_os("windows") then
    add_files("src/platform/windows/*.cpp")
    add_defines("KVM_OS_WINDOWS", "UNICODE")
    add_syslinks("gdi32", "msimg32", "user32", "Dxva2", "Setupapi", "Advapi32")
  end

  if is_os("linux") then
    add_files("src/platform/linux/*.cpp")
    add_files("src/platform/unix/*.cpp")
    add_packages("libusb")
    add_defines("KVM_OS_LINUX")
  end

  if is_os("macosx") then
    add_files("src/platform/mac/*.cpp")
    add_files("src/platform/unix/*.cpp")
    add_packages("libusb")
    add_frameworks("CoreGraphics", "AppKit", "DriverKit", "IOKit", "CoreFoundation", "Foundation")
    add_defines("KVM_OS_MAC")
    add_ldflags("-lobjc")
  end