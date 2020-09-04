target("console")
  set_kind("binary")
  add_files("src/*.cpp", "src/usb/*.cpp", "src/ddc/*.cpp")
  add_includedirs("$(projectdir)/include")

  add_includedirs("$(projectdir)/deps/libusb/include")

  if is_os("windows") then
    add_files("src/platform/windows/*.cpp")
    add_linkdirs("deps/libusb/MS64/static")
    add_links("libusb-1.0")
  end

  if is_os("linux") then
    add_files("src/platform/linux/*.cpp")
  end

  if is_os("macosx") then
    add_files("src/platform/mac/*.cpp")
  end