mod build;
mod config;
mod dag;

use crate::build::execute_build;
use crate::config::{load_config, ProjectConfig};
use crate::dag::{build_dag, topological_sort};
use colored::*;
use std::error::Error;
use std::time::Instant;

fn main() -> Result<(), Box<dyn Error>> {
    let build_file = "build.json";
    let root_dir = ".";

    let abs_root_dir = std::fs::canonicalize(root_dir)?;
    let config = load_config(&abs_root_dir.join(build_file))?;

    let project = ProjectConfig {
        build_config: config,
        root_dir: abs_root_dir,
    };

    let dag = build_dag(&project.build_config)?;
    let build_order = topological_sort(&dag)?;

    println!(
        "{}",
        format!(
            "{}{}",
            "   Compiling".green().bold(),
            " bake v0.1.0 (/path/to/your/project)"
        )
    );

    let start_time = Instant::now();

    execute_build(&project, &build_order, &dag)?;

    let total_duration = start_time.elapsed();
    println!(
        "{}",
        format!(
            "{} in {:.2}s",
            "     Finished".green().bold(),
            total_duration.as_secs_f32()
        )
    );

    Ok(())
}
