use std::error::Error as StdError;
use std::fmt;

#[derive(Debug)]
pub struct BakeError(pub String);

impl fmt::Display for BakeError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.0)
    }
}

impl StdError for BakeError {}

impl From<Box<dyn StdError>> for BakeError {
    fn from(error: Box<dyn StdError>) -> Self {
        BakeError(error.to_string())
    }
}
