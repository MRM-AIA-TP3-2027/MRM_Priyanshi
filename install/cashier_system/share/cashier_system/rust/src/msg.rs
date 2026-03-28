#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};



// Corresponds to cashier_system__msg__Bill

// This struct is not documented.
#[allow(missing_docs)]

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct Bill {

    // This member is not documented.
    #[allow(missing_docs)]
    pub item_name: std::string::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub quantity: i32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub price: f32,

}



impl Default for Bill {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::Bill::default())
  }
}

impl rosidl_runtime_rs::Message for Bill {
  type RmwMsg = super::msg::rmw::Bill;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        item_name: msg.item_name.as_str().into(),
        quantity: msg.quantity,
        price: msg.price,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        item_name: msg.item_name.as_str().into(),
      quantity: msg.quantity,
      price: msg.price,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      item_name: msg.item_name.to_string(),
      quantity: msg.quantity,
      price: msg.price,
    }
  }
}


