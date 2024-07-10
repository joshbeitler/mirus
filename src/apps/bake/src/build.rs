use std::path::{Path, PathBuf};
use std::time::Instant;
use std::io::{self, Write};
use std::collections::{BTreeMap, HashMap};

use glob::glob;
use colored::*;
use futures::stream::{self, StreamExt};
use tokio::task;
use num_cpus;

use crate::config::{ProjectConfig, Tool};
use crate::dag::{Dag, get_recipe};
use crate::error::BakeError;

/// Executes the build process for the given project.
///
/// The build process will run each tool for each source file in the recipe's
/// sources list, and will output the results to the build directory. Optional
/// concurrency can be enabled for tools that support it.
pub async fn execute_build(
    project: &ProjectConfig,
    order: &[String],
    dag: &Dag,
    verbose: bool,
) -> Result<(), BakeError> {
    for recipe_name in order {
        if let Some(recipe) = get_recipe(dag, recipe_name) {
            let start_time = Instant::now();

            let output_dir = project.root_dir.join("build").join(recipe_name);
            tokio::fs::create_dir_all(&output_dir).await.map_err(|e| BakeError(e.to_string()))?;

            let expanded_sources = expand_sources(&recipe.sources, &project.root_dir)?;
            let ordered_tools: BTreeMap<_, _> = recipe.tools.iter().collect();

            for (tool_name, tool) in ordered_tools {
                print!("{}",
                    format!("     {} {} for {} ... ",
                        "Running".bold().green(),
                        tool_name.cyan(),
                        recipe_name.yellow()
                    )
                );
                io::stdout().flush().map_err(|e| BakeError(e.to_string()))?;

                if tool.concurrent {
                    let results = stream::iter(expanded_sources.iter())
                        .map(|source| {
                            let tool = tool.clone();
                            let root_dir = project.root_dir.clone();
                            let output_dir = output_dir.clone();
                            let source = source.clone();
                            task::spawn(async move {
                                run_tool_on_file(&tool, &[source], &root_dir, &output_dir, verbose).await
                            })
                        })
                        .buffer_unordered(num_cpus::get())
                        .collect::<Vec<_>>()
                        .await;

                    for result in results {
                        result.map_err(|e| BakeError(e.to_string()))??;
                    }
                } else {
                    run_tool_on_file(tool, &expanded_sources, &project.root_dir, &output_dir, verbose).await?;
                }

                println!("{}", "done".green());
            }

            let duration = start_time.elapsed();
            println!("{}",
                format!("   {} {} in {:.2}s",
                    "Compiled".green().bold(),
                    recipe_name.yellow(),
                    duration.as_secs_f32()
                )
            );
        }
    }

    Ok(())
}

/// Runs the given tool on the provided source files.
///
/// The tool will be run with the provided arguments, and the output will be
/// written to the output directory. If the tool fails to run, an error will be
/// returned.
///
/// Able to run multiple instances of a tool concurrently if the tool is
/// marked as such in the receipe.
async fn run_tool_on_file(
    tool: &Tool,
    sources: &[PathBuf],
    root_dir: &Path,
    output_dir: &Path,
    verbose: bool,
) -> Result<(), BakeError> {
    let args = format_arguments(&tool, &sources, output_dir)?;

    if verbose {
        println!("\nCommand: {} {:?}", tool.cmd, args);
    }

    let output = tokio::process::Command::new(&tool.cmd)
        .args(&args)
        .current_dir(root_dir)
        .output()
        .await
        .map_err(|e| BakeError(e.to_string()))?;

    if !output.status.success() {
        println!("{}", "failed".red());
        if verbose {
            io::stderr().write_all(&output.stderr).map_err(|e| BakeError(e.to_string()))?;
        }
        return Err(BakeError(format!("Failed to run {} for files {:?}", tool.cmd, sources)));
    }

    if verbose {
        io::stdout().write_all(&output.stdout).map_err(|e| BakeError(e.to_string()))?;
        io::stderr().write_all(&output.stderr).map_err(|e| BakeError(e.to_string()))?;
    }

    Ok(())
}


fn format_arguments(tool: &Tool, sources: &[PathBuf], output_dir: &Path) -> Result<Vec<String>, BakeError> {
    let mut replacements = HashMap::new();
    replacements.insert("sources", sources.iter().map(|p| p.to_string_lossy().into_owned()).collect::<Vec<_>>().join(" "));
    replacements.insert("output_dir", output_dir.to_string_lossy().into_owned());
    replacements.insert("output_file_stem", sources.first()
        .and_then(|p| p.file_stem())
        .ok_or(BakeError("Couldn't get output file stem".into()))?
        .to_string_lossy().into_owned());

    fn replace_and_expand(arg: &str, replacements: &HashMap<&str, String>) -> Result<Vec<String>, BakeError> {
        let mut result = arg.to_string();
        for (pattern, replacement) in replacements {
            result = result.replace(&format!("{{{}}}", pattern), replacement);
        }

        // Check if the argument contains any glob characters
        if result.contains('*') || result.contains('?') || result.contains('[') {
            let expanded: Vec<String> = glob(&result)
                .map_err(|e| BakeError(e.to_string()))?
                .filter_map(Result::ok)
                .map(|path| path.to_string_lossy().into_owned())
                .collect();

            if expanded.is_empty() {
                // If glob doesn't match anything, return the original string
                Ok(vec![result])
            } else {
                Ok(expanded)
            }
        } else {
            Ok(vec![result])
        }
    }

    let mut formatted_args = Vec::new();
    for arg in &tool.args {
        let replaced = replace_and_expand(arg, &replacements)?;
        formatted_args.extend(replaced);
    }

    Ok(formatted_args)
}

fn expand_sources(sources: &[String], root_dir: &Path) -> Result<Vec<PathBuf>, BakeError> {
    let mut expanded_sources = Vec::new();

    for source in sources {
        let full_pattern = root_dir.join(source);
        let matches: Vec<PathBuf> = glob(&full_pattern.to_string_lossy())
            .map_err(|e| BakeError(e.to_string()))?
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
