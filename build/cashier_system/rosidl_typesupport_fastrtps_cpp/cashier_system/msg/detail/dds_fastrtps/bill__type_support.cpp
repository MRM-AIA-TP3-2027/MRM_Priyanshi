// generated from rosidl_typesupport_fastrtps_cpp/resource/idl__type_support.cpp.em
// with input from cashier_system:msg/Bill.idl
// generated code does not contain a copyright notice
#include "cashier_system/msg/detail/bill__rosidl_typesupport_fastrtps_cpp.hpp"
#include "cashier_system/msg/detail/bill__struct.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_fastrtps_cpp/identifier.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_fastrtps_cpp/wstring_conversion.hpp"
#include "fastcdr/Cdr.h"


// forward declaration of message dependencies and their conversion functions

namespace cashier_system
{

namespace msg
{

namespace typesupport_fastrtps_cpp
{

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cashier_system
cdr_serialize(
  const cashier_system::msg::Bill & ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  // Member: item_name
  cdr << ros_message.item_name;
  // Member: quantity
  cdr << ros_message.quantity;
  // Member: price
  cdr << ros_message.price;
  return true;
}

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cashier_system
cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  cashier_system::msg::Bill & ros_message)
{
  // Member: item_name
  cdr >> ros_message.item_name;

  // Member: quantity
  cdr >> ros_message.quantity;

  // Member: price
  cdr >> ros_message.price;

  return true;
}  // NOLINT(readability/fn_size)

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cashier_system
get_serialized_size(
  const cashier_system::msg::Bill & ros_message,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // Member: item_name
  current_alignment += padding +
    eprosima::fastcdr::Cdr::alignment(current_alignment, padding) +
    (ros_message.item_name.size() + 1);
  // Member: quantity
  {
    size_t item_size = sizeof(ros_message.quantity);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: price
  {
    size_t item_size = sizeof(ros_message.price);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  return current_alignment - initial_alignment;
}

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cashier_system
max_serialized_size_Bill(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  size_t last_member_size = 0;
  (void)last_member_size;
  (void)padding;
  (void)wchar_size;

  full_bounded = true;
  is_plain = true;


  // Member: item_name
  {
    size_t array_size = 1;

    full_bounded = false;
    is_plain = false;
    for (size_t index = 0; index < array_size; ++index) {
      current_alignment += padding +
        eprosima::fastcdr::Cdr::alignment(current_alignment, padding) +
        1;
    }
  }

  // Member: quantity
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint32_t);
    current_alignment += array_size * sizeof(uint32_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint32_t));
  }

  // Member: price
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint32_t);
    current_alignment += array_size * sizeof(uint32_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint32_t));
  }

  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = cashier_system::msg::Bill;
    is_plain =
      (
      offsetof(DataType, price) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static bool _Bill__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  auto typed_message =
    static_cast<const cashier_system::msg::Bill *>(
    untyped_ros_message);
  return cdr_serialize(*typed_message, cdr);
}

static bool _Bill__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  auto typed_message =
    static_cast<cashier_system::msg::Bill *>(
    untyped_ros_message);
  return cdr_deserialize(cdr, *typed_message);
}

static uint32_t _Bill__get_serialized_size(
  const void * untyped_ros_message)
{
  auto typed_message =
    static_cast<const cashier_system::msg::Bill *>(
    untyped_ros_message);
  return static_cast<uint32_t>(get_serialized_size(*typed_message, 0));
}

static size_t _Bill__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_Bill(full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}

static message_type_support_callbacks_t _Bill__callbacks = {
  "cashier_system::msg",
  "Bill",
  _Bill__cdr_serialize,
  _Bill__cdr_deserialize,
  _Bill__get_serialized_size,
  _Bill__max_serialized_size
};

static rosidl_message_type_support_t _Bill__handle = {
  rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
  &_Bill__callbacks,
  get_message_typesupport_handle_function,
};

}  // namespace typesupport_fastrtps_cpp

}  // namespace msg

}  // namespace cashier_system

namespace rosidl_typesupport_fastrtps_cpp
{

template<>
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_EXPORT_cashier_system
const rosidl_message_type_support_t *
get_message_type_support_handle<cashier_system::msg::Bill>()
{
  return &cashier_system::msg::typesupport_fastrtps_cpp::_Bill__handle;
}

}  // namespace rosidl_typesupport_fastrtps_cpp

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, cashier_system, msg, Bill)() {
  return &cashier_system::msg::typesupport_fastrtps_cpp::_Bill__handle;
}

#ifdef __cplusplus
}
#endif
