use crate::config::{BuildConfig, Target};
use petgraph::algo::toposort;
use petgraph::graph::DiGraph;
use std::collections::HashMap;
use std::error::Error;

pub type Dag = DiGraph<Target, ()>;

pub fn build_dag(config: &BuildConfig) -> Result<Dag, Box<dyn Error>> {
    let mut dag = DiGraph::new();
    let mut node_indices = HashMap::new();

    // First pass: add all targets as nodes
    for target in &config.targets {
        let node_index = dag.add_node(target.clone());
        node_indices.insert(target.name.clone(), node_index);
    }

    // Second pass: add edges for dependencies
    for target in &config.targets {
        if let Some(deps) = &target.dependencies {
            let target_index = node_indices[&target.name];
            for dep in deps {
                if let Some(&dep_index) = node_indices.get(dep) {
                    dag.add_edge(dep_index, target_index, ());
                } else {
                    return Err(
                        format!("Dependency {} of target {} not found", dep, target.name).into(),
                    );
                }
            }
        }
    }

    // Check for cycles
    if toposort(&dag, None).is_err() {
        return Err("Cycle detected in dependency graph".into());
    }

    Ok(dag)
}

pub fn topological_sort(dag: &Dag) -> Result<Vec<String>, Box<dyn Error>> {
    let sorted_indices = toposort(&dag, None).map_err(|_| "Cycle detected in dependency graph")?;

    Ok(sorted_indices
        .into_iter()
        .map(|index| dag[index].name.clone())
        .collect())
}

pub fn get_target<'a>(dag: &'a Dag, name: &str) -> Option<&'a Target> {
    dag.node_indices()
        .find(|&index| dag[index].name == name)
        .map(|index| &dag[index])
}
