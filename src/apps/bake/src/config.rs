use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::error::Error;
use std::fs::File;
use std::path::{Path, PathBuf};

#[derive(Debug, Deserialize, Serialize)]
pub struct BuildConfig {
    pub targets: Vec<Target>,
}

#[derive(Debug, Deserialize, Serialize, Clone)]
pub struct Target {
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
}

pub struct ProjectConfig {
    pub build_config: BuildConfig,
    pub root_dir: PathBuf,
}

pub fn load_config(filename: &Path) -> Result<BuildConfig, Box<dyn Error>> {
    let file = File::open(filename)?;
    let config: BuildConfig = serde_json::from_reader(file)?;
    Ok(config)
}
