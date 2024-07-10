use std::time::Instant;

use colored::*;
use clap::{Parser, Subcommand};

use crate::config::{ProjectConfig, load_config};
use crate::dag::{build_dag, topological_sort};
use crate::build::execute_build;
use crate::error::BakeError;

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
pub struct Cli {
    #[command(subcommand)]
    pub command: Commands,
}

#[derive(Subcommand)]
pub enum Commands {
    /// Build the project
    Build {
        /// Build file to use
        #[arg(short, long, default_value = "build.json")]
        build_file: String,

        /// Project root directory
        #[arg(short, long, default_value = ".")]
        root_dir: String,

        /// Enable verbose output
        #[arg(short, long)]
        verbose: bool,
    },
    /// Clean the build artifacts
    Clean {
        /// Project root directory
        #[arg(short, long, default_value = ".")]
        root_dir: String,

        /// Enable verbose output
        #[arg(short, long)]
        verbose: bool,
    },
    /// List available recipes
    ListRecipes {
        /// Build file to use
        #[arg(short, long, default_value = "build.json")]
        build_file: String,
    },
}

/// Build the project using the specified build file and root directory.
pub async fn build(build_file: &str, root_dir: &str, verbose: bool) -> Result<(), BakeError> {
    let abs_root_dir = std::fs::canonicalize(root_dir).map_err(|e| BakeError(e.to_string()))?;
    let config = load_config(&abs_root_dir.join(build_file))?;

    let project = ProjectConfig {
        build_config: config,
        root_dir: abs_root_dir,
    };

    let dag = build_dag(&project.build_config)?;
    let build_order = topological_sort(&dag)?;

    println!("{}", format!("{}{}",
        "   Compiling".green().bold(),
        format!(" bake v0.1.0 ({})", project.root_dir.display())
    ));

    if verbose {
        println!("Build order: {:?}", build_order);
    }

    let start_time = Instant::now();

    execute_build(&project, &build_order, &dag, verbose).await?;

    let total_duration = start_time.elapsed();
    println!("{}", format!("{} in {:.2}s",
        "    Finished".green().bold(),
        total_duration.as_secs_f32()
    ));

    Ok(())
}

/// Clean the build artifacts in the specified root directory.
pub async fn clean(root_dir: &str, verbose: bool) -> Result<(), BakeError> {
    let abs_root_dir = std::fs::canonicalize(root_dir).map_err(|e| BakeError(e.to_string()))?;
    let build_dir = abs_root_dir.join("build");

    if build_dir.exists() {
        println!("{}", "   Cleaning".green().bold());
        tokio::fs::remove_dir_all(&build_dir).await.map_err(|e| BakeError(e.to_string()))?;
        println!("{}", "    Finished".green().bold());
        if verbose {
            println!("Removed directory: {}", build_dir.display());
        }
    } else {
        println!("Nothing to clean");
    }

    Ok(())
}

pub async fn list_recipes(build_file: &str) -> Result<(), BakeError> {
    let abs_root_dir = std::fs::canonicalize(".").map_err(|e| BakeError(e.to_string()))?;
    let config = load_config(&abs_root_dir.join(build_file))?;

    println!("{}", format!("{}{}",
        "   Recipes".green().bold(),
        format!(" bake v0.1.0 ({})", abs_root_dir.display())
    ));

    for recipe in config.recipes {
        println!("{}", recipe.name);
    }

    Ok(())
}
