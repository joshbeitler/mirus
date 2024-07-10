use std::collections::HashMap;

use petgraph::algo::toposort;
use petgraph::graph::DiGraph;

use crate::config::{BuildConfig, Recipe};
use crate::error::BakeError;

pub type Dag = DiGraph<Recipe, ()>;

/// Builds a directed acyclic graph (DAG) from a build configuration.
///
/// The DAG allows us to determine the order in which recipes should be built
/// to power the automatic dependency resolution in the build system.
pub fn build_dag(config: &BuildConfig) -> Result<Dag, BakeError> {
    let mut dag = DiGraph::new();
    let mut node_indices = HashMap::new();

    // First pass: add all recipes as nodes
    for recipe in &config.recipes {
        let node_index = dag.add_node(recipe.clone());
        node_indices.insert(recipe.name.clone(), node_index);
    }

    // Second pass: add edges for dependencies
    for recipe in &config.recipes {
        if let Some(deps) = &recipe.dependencies {
            let recipe_index = node_indices[&recipe.name];
            for dep in deps {
                if let Some(&dep_index) = node_indices.get(dep) {
                    dag.add_edge(dep_index, recipe_index, ());
                } else {
                    return Err(BakeError(format!(
                        "Dependency {} of target {} not found",
                        dep, recipe.name
                    )));
                }
            }
        }
    }

    // Check for cycles
    if toposort(&dag, None).is_err() {
        return Err(BakeError("Cycle detected in dependency graph".into()));
    }

    Ok(dag)
}

/// Topologically sorts the DAG to determine the order in which recipes should
/// be built.
///
/// This function returns a list of recipe names in the order they should be
/// built.
pub fn topological_sort(dag: &Dag) -> Result<Vec<String>, BakeError> {
    let sorted_indices =
        toposort(&dag, None).map_err(|_| BakeError("Cycle detected in dependency graph".into()))?;

    Ok(sorted_indices
        .into_iter()
        .map(|index| dag[index].name.clone())
        .collect())
}

/// Retrieves a recipe from the DAG by name.
pub fn get_recipe<'a>(dag: &'a Dag, name: &str) -> Option<&'a Recipe> {
    dag.node_indices()
        .find(|&index| dag[index].name == name)
        .map(|index| &dag[index])
}
