// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from cashier_system:msg/Bill.idl
// generated code does not contain a copyright notice

#ifndef CASHIER_SYSTEM__MSG__DETAIL__BILL__STRUCT_HPP_
#define CASHIER_SYSTEM__MSG__DETAIL__BILL__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__cashier_system__msg__Bill __attribute__((deprecated))
#else
# define DEPRECATED__cashier_system__msg__Bill __declspec(deprecated)
#endif

namespace cashier_system
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct Bill_
{
  using Type = Bill_<ContainerAllocator>;

  explicit Bill_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->item_name = "";
      this->quantity = 0l;
      this->price = 0.0f;
    }
  }

  explicit Bill_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : item_name(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->item_name = "";
      this->quantity = 0l;
      this->price = 0.0f;
    }
  }

  // field types and members
  using _item_name_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _item_name_type item_name;
  using _quantity_type =
    int32_t;
  _quantity_type quantity;
  using _price_type =
    float;
  _price_type price;

  // setters for named parameter idiom
  Type & set__item_name(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->item_name = _arg;
    return *this;
  }
  Type & set__quantity(
    const int32_t & _arg)
  {
    this->quantity = _arg;
    return *this;
  }
  Type & set__price(
    const float & _arg)
  {
    this->price = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    cashier_system::msg::Bill_<ContainerAllocator> *;
  using ConstRawPtr =
    const cashier_system::msg::Bill_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<cashier_system::msg::Bill_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<cashier_system::msg::Bill_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      cashier_system::msg::Bill_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<cashier_system::msg::Bill_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      cashier_system::msg::Bill_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<cashier_system::msg::Bill_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<cashier_system::msg::Bill_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<cashier_system::msg::Bill_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__cashier_system__msg__Bill
    std::shared_ptr<cashier_system::msg::Bill_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__cashier_system__msg__Bill
    std::shared_ptr<cashier_system::msg::Bill_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Bill_ & other) const
  {
    if (this->item_name != other.item_name) {
      return false;
    }
    if (this->quantity != other.quantity) {
      return false;
    }
    if (this->price != other.price) {
      return false;
    }
    return true;
  }
  bool operator!=(const Bill_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Bill_

// alias to use template instance with default allocator
using Bill =
  cashier_system::msg::Bill_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace cashier_system

#endif  // CASHIER_SYSTEM__MSG__DETAIL__BILL__STRUCT_HPP_
