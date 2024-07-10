use std::collections::HashMap;
use std::fs::File;
use std::io::Read;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::error::BakeError;

#[derive(Debug, Deserialize, Serialize)]
pub struct BuildConfig {
    pub recipes: Vec<Recipe>,
}

#[derive(Debug, Deserialize, Serialize, Clone)]
pub struct Recipe {
    pub name: String,
    pub sources: Vec<String>,
    pub includes: Option<Vec<String>>,
    pub tools: HashMap<String, Tool>,
    pub dependencies: Option<Vec<String>>,
}

#[derive(Debug, Deserialize, Serialize, Clone)]
pub struct Tool {
    pub cmd: String,
    pub args: Vec<String>,
    #[serde(default)]
    pub concurrent: bool,
}

pub struct ProjectConfig {
    pub build_config: BuildConfig,
    pub root_dir: PathBuf,
}

/// Load a build configuration from a JSON file.
///
/// TODO: Needs validation and better error handling.
pub fn load_config(path: &Path) -> Result<BuildConfig, BakeError> {
    let mut file = File::open(path).map_err(|e| BakeError(e.to_string()))?;
    let mut contents = String::new();
    file.read_to_string(&mut contents)
        .map_err(|e| BakeError(e.to_string()))?;
    let config: BuildConfig =
        serde_json::from_str(&contents).map_err(|e| BakeError(e.to_string()))?;
    Ok(config)
}
