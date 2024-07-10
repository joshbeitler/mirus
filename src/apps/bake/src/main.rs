mod build;
mod cli;
mod config;
mod dag;

use clap::Parser;
use cli::{Cli, Commands};
use std::error::Error;

fn main() -> Result<(), Box<dyn Error>> {
    let cli = Cli::parse();

    match &cli.command {
        Commands::Build {
            build_file,
            root_dir,
            verbose,
        } => cli::build(build_file, root_dir, *verbose),
        Commands::Clean { root_dir, verbose } => cli::clean(root_dir, *verbose),
    }
}
