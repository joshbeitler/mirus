mod config;
mod dag;
mod build;
mod cli;
mod error;

use clap::Parser;
use cli::{Cli, Commands};

/// Main entry point for the bake tool.
///
/// Initializes the CLI parser and runs the appropriate command with args.
#[tokio::main]
async fn main() -> Result<(), error::BakeError> {
    let cli = Cli::parse();

    match &cli.command {
        Commands::Build { build_file, root_dir, verbose } => {
            cli::build(build_file, root_dir, *verbose).await
        }
        Commands::Clean { root_dir, verbose } => {
            cli::clean(root_dir, *verbose).await
        }
        Commands::ListRecipes { build_file } => {
            cli::list_recipes(build_file).await
        }
    }
}
