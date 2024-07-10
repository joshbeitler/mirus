use std::collections::HashMap;

use petgraph::algo::toposort;
use petgraph::graph::{DiGraph, EdgeIndex};
use petgraph::visit::EdgeRef;

use crate::config::{BuildConfig, Recipe};
use crate::error::BakeError;

pub type Dag = DiGraph<Recipe, usize>;

pub fn build_dag(config: &BuildConfig) -> Result<Dag, BakeError> {
    let mut dag = DiGraph::new();
    let mut node_indices = HashMap::new();

    // First pass: add all recipes as nodes
    for (index, recipe) in config.recipes.iter().enumerate() {
        let node_index = dag.add_node(recipe.clone());
        node_indices.insert(recipe.name.clone(), (node_index, index));
    }

    // Second pass: add edges for dependencies with weights
    for (recipe_index, recipe) in config.recipes.iter().enumerate() {
        if let Some(deps) = &recipe.dependencies {
            let recipe_node_index = node_indices[&recipe.name].0;
            for dep in deps {
                if let Some(&(dep_node_index, dep_index)) = node_indices.get(dep) {
                    // Use the difference in indices as weight
                    let weight = recipe_index - dep_index;
                    dag.add_edge(dep_node_index, recipe_node_index, weight);
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

pub fn topological_sort(dag: &Dag) -> Result<Vec<String>, BakeError> {
    let mut sorted_indices =
        toposort(&dag, None).map_err(|_| BakeError("Cycle detected in dependency graph".into()))?;

    // Sort the nodes based on their edges' weights
    sorted_indices.sort_by_key(|&index| {
        dag.edges_directed(index, petgraph::Direction::Incoming)
            .map(|edge| *edge.weight())
            .sum::<usize>()
    });

    Ok(sorted_indices
        .into_iter()
        .map(|index| dag[index].name.clone())
        .collect())
}

pub fn get_recipe<'a>(dag: &'a Dag, name: &str) -> Option<&'a Recipe> {
    dag.node_indices()
        .find(|&index| dag[index].name == name)
        .map(|index| &dag[index])
}
