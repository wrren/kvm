if not is_os("windows") then
  add_requires("libusb")
end

target("console")
  set_kind("binary")
  set_languages("cxx17")
  add_files("src/*.cpp", "src/usb/*.cpp", "src/ddc/*.cpp")
  add_includedirs("$(projectdir)/include")

  if is_os("windows") then
    add_files("src/platform/windows/*.cpp")
    add_includedirs("$(projectdir)/deps/libusb/include/libusb-1.0")
    add_linkdirs("deps/libusb/MS64/static")
    add_links("libusb-1.0")
  end

  if is_os("linux") then
    add_files("src/platform/linux/*.cpp")
    add_packages("libusb")
  end

  if is_os("macosx") then
    add_files("src/platform/mac/*.cpp")
    add_packages("libusb")
  end