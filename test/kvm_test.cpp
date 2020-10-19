#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <kvm.h>
#include <networking/message/types.h>
#include <networking/message/change_input_request.h>

using namespace kvm;

TEST_CASE("messages are serialized and deserialized correctly", "[networking]") {
  NetworkBuffer buffer;

  Display display("GSM", 1111, 2222, "Test Display");
  Display::InputMap changes;
  changes[display] = Display::Input::HDMI1;
  
  ChangeInputRequest in(changes);
  in.Serialize(buffer);

  buffer.Reset();

  REQUIRE(NetworkMessage::IsContainedIn(in.GetType(), buffer));

  ChangeInputRequest out;
  out.Deserialize(buffer);

  REQUIRE(out.GetInputMap().size() == 1);
  REQUIRE(out.GetInputMap().begin()->first == display);
  REQUIRE(out.GetInputMap().begin()->second == Display::Input::HDMI1);

}