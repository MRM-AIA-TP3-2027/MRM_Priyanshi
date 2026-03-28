#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};


#[link(name = "cashier_system__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__cashier_system__msg__Bill() -> *const std::ffi::c_void;
}

#[link(name = "cashier_system__rosidl_generator_c")]
extern "C" {
    fn cashier_system__msg__Bill__init(msg: *mut Bill) -> bool;
    fn cashier_system__msg__Bill__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<Bill>, size: usize) -> bool;
    fn cashier_system__msg__Bill__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<Bill>);
    fn cashier_system__msg__Bill__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<Bill>, out_seq: *mut rosidl_runtime_rs::Sequence<Bill>) -> bool;
}

// Corresponds to cashier_system__msg__Bill
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct Bill {

    // This member is not documented.
    #[allow(missing_docs)]
    pub item_name: rosidl_runtime_rs::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub quantity: i32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub price: f32,

}



impl Default for Bill {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !cashier_system__msg__Bill__init(&mut msg as *mut _) {
        panic!("Call to cashier_system__msg__Bill__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for Bill {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cashier_system__msg__Bill__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cashier_system__msg__Bill__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cashier_system__msg__Bill__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for Bill {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for Bill where Self: Sized {
  const TYPE_NAME: &'static str = "cashier_system/msg/Bill";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__cashier_system__msg__Bill() }
  }
}


