use crate::config::ProjectConfig;
use crate::dag::{get_target, Dag};
use colored::*;
use glob::glob;
use std::error::Error;
use std::io::{self, Write};
use std::path::{Path, PathBuf};
use std::process::Command;
use std::time::Instant;

pub fn execute_build(
    project: &ProjectConfig,
    order: &[String],
    dag: &Dag,
) -> Result<(), Box<dyn Error>> {
    for target_name in order {
        if let Some(target) = get_target(dag, target_name) {
            let start_time = Instant::now();

            let output_dir = project.root_dir.join("build").join(target_name);
            std::fs::create_dir_all(&output_dir)?;

            let expanded_sources = expand_sources(&target.sources, &project.root_dir)?;

            for (tool_name, tool) in &target.tools {
                print!(
                    "{}",
                    format!(
                        "     {} {} for {} ... ",
                        "Running".bold().green(),
                        tool_name.cyan(),
                        target_name.yellow()
                    )
                );
                io::stdout().flush()?;

                let mut args = Vec::new();
                for arg in &tool.args {
                    match arg.as_str() {
                        "{sources}" => args.extend(
                            expanded_sources
                                .iter()
                                .map(|p| p.to_string_lossy().into_owned()),
                        ),
                        "{includes}" => {
                            if let Some(includes) = &target.includes {
                                for inc in includes {
                                    args.push(format!(
                                        "-I{}",
                                        project.root_dir.join(inc).display()
                                    ));
                                }
                            }
                        }
                        _ => args.push(arg.clone()),
                    }
                }

                let output = Command::new(&tool.cmd)
                    .args(&args)
                    .current_dir(&output_dir)
                    .output()?;

                if !output.status.success() {
                    println!("{}", "failed".red());
                    return Err(
                        format!("Failed to run {} for target {}", tool_name, target_name).into(),
                    );
                }

                println!("{}", "done".green());
            }

            let duration = start_time.elapsed();
            println!(
                "{}",
                format!(
                    "   {} {} in {:.2}s",
                    "Compiled".green().bold(),
                    target_name.yellow(),
                    duration.as_secs_f32()
                )
            );
        }
    }

    Ok(())
}

fn expand_sources(sources: &[String], root_dir: &Path) -> Result<Vec<PathBuf>, Box<dyn Error>> {
    let mut expanded_sources = Vec::new();

    for source in sources {
        let full_pattern = root_dir.join(source);
        let matches: Vec<PathBuf> = glob(&full_pattern.to_string_lossy())
            .expect("Failed to read glob pattern")
            .filter_map(Result::ok)
            .collect();

        if matches.is_empty() {
            expanded_sources.push(root_dir.join(source));
        } else {
            expanded_sources.extend(matches);
        }
    }

    Ok(expanded_sources)
}
