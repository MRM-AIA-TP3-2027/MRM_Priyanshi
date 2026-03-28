// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from cashier_system:msg/Bill.idl
// generated code does not contain a copyright notice

#ifndef CASHIER_SYSTEM__MSG__DETAIL__BILL__STRUCT_H_
#define CASHIER_SYSTEM__MSG__DETAIL__BILL__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'item_name'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/Bill in the package cashier_system.
typedef struct cashier_system__msg__Bill
{
  rosidl_runtime_c__String item_name;
  int32_t quantity;
  float price;
} cashier_system__msg__Bill;

// Struct for a sequence of cashier_system__msg__Bill.
typedef struct cashier_system__msg__Bill__Sequence
{
  cashier_system__msg__Bill * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} cashier_system__msg__Bill__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CASHIER_SYSTEM__MSG__DETAIL__BILL__STRUCT_H_
