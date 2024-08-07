use std::fs::OpenOptions;
use std::io::{self, BufRead, BufReader, Seek, SeekFrom};
use std::path::Path;
use std::thread;
use std::time::Duration;

use clap::Parser;
use colored::*;
use serde_json::Value;
use unicode_width::UnicodeWidthStr;

#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = None)]
struct Args {
    #[clap(short, long)]
    file: String,
}

fn main() -> io::Result<()> {
    let args = Args::parse();
    let path = Path::new(&args.file);

    // Open the file, creating it if it doesn't exist
    let file = OpenOptions::new()
        .read(true)
        .write(true)
        .create(true)
        .open(path)?;

    let mut reader = BufReader::new(file);

    println!("Tailing file: {}", args.file);
    if reader.get_ref().metadata()?.len() == 0 {
        println!("File was empty, created a new one. Waiting for log entries...");
    } else {
        println!("Waiting for new log entries...");
    }

    let mut position = 0;
    loop {
        let mut line = String::new();
        reader.seek(SeekFrom::Start(position))?;
        let bytes_read = reader.read_line(&mut line)?;

        if bytes_read > 0 {
            position += bytes_read as u64;
            if let Ok(json) = serde_json::from_str::<Value>(&line) {
                print_log_entry(&json);
            }
        } else {
            thread::sleep(Duration::from_millis(100));
        }
    }
}

fn print_log_entry(entry: &Value) {
    let level = entry["level"].as_str().unwrap_or("UNKNOWN");
    let component = entry["component"].as_str().unwrap_or("N/A");
    let message = entry["message"].as_str().unwrap_or("N/A");

    let level_color = match level {
        "INFO" => "blue",
        "WARNING" => "yellow",
        "ERROR" => "red",
        _ => "white",
    };

    println!(
        "{} {:<7} {:<20} {}",
        "│".color(level_color),
        level.color(level_color).bold(),
        component.dimmed(),
        message
    );

    if let Some(data) = entry.get("data") {
        print_json_table(data, level_color, vec!["data"]);
    }
}

fn print_json_table(value: &Value, color: &str, path: Vec<&str>) {
    match value {
        Value::Object(obj) => {
            print_table_header(&path, color);
            print_object_table(obj, color);

            for (key, val) in obj {
                match val {
                    Value::Object(_) | Value::Array(_) => {
                        let mut new_path = path.clone();
                        new_path.push(key);
                        print_json_table(val, color, new_path);
                    }
                    _ => {}
                }
            }
        }
        Value::Array(arr) => {
            print_table_header(&path, color);
            print_array_table(arr, color);
        }
        _ => println!("{}", format_value(value)),
    }
}

fn print_table_header(path: &[&str], color: &str) {
    let header = path.join(".");
    println!("\n{}", header.color(color).bold());
}

fn print_object_table(obj: &serde_json::Map<String, Value>, color: &str) {
    let mut keys: Vec<&str> = obj.keys().map(|s| s.as_str()).collect();
    keys.sort();

    let max_key_width = keys.iter().map(|k| k.width()).max().unwrap_or(0);
    let max_value_width = keys
        .iter()
        .map(|&k| format_value(&obj[k]).width())
        .max()
        .unwrap_or(0);

    print_table_border(max_key_width, max_value_width, '┌', '┬', '┐', color);
    print_table_row("Key", "Value", max_key_width, max_value_width, color);
    print_table_border(max_key_width, max_value_width, '├', '┼', '┤', color);

    for key in keys {
        let value = &obj[key];
        let formatted_value = match value {
            Value::Object(o) => format!("{{{}}}", o.len()).color(color).to_string(),
            Value::Array(a) => format!("[{}]", a.len()).color(color).to_string(),
            _ => format_value(value),
        };
        print_table_row(key, &formatted_value, max_key_width, max_value_width, color);
    }

    print_table_border(max_key_width, max_value_width, '└', '┴', '┘', color);
}

fn print_array_table(arr: &Vec<Value>, color: &str) {
    if arr.is_empty() {
        return;
    }

    if let Some(Value::Object(_)) = arr.get(0) {
        // If the array contains objects, print them as a wide table
        print_wide_array_table(arr, color);
    } else {
        // Otherwise, print as a simple list
        for (index, item) in arr.iter().enumerate() {
            println!("{}: {}", index, format_value(item));
        }
    }
}

fn print_wide_array_table(arr: &Vec<Value>, color: &str) {
    let mut all_keys: Vec<String> = Vec::new();
    for item in arr.iter() {
        if let Value::Object(obj) = item {
            for key in obj.keys() {
                if !all_keys.contains(key) {
                    all_keys.push(key.clone());
                }
            }
        }
    }
    all_keys.sort();

    let column_widths: Vec<usize> = all_keys
        .iter()
        .map(|key| {
            arr.iter()
                .filter_map(|item| item.as_object())
                .filter_map(|obj| obj.get(key))
                .map(|v| format_value(v).width())
                .max()
                .unwrap_or(key.width())
                .max(key.width())
        })
        .collect();

    print_wide_table_border(&column_widths, '┌', '┬', '┐', color);
    print_wide_table_row(&all_keys, &column_widths, color);
    print_wide_table_border(&column_widths, '├', '┼', '┤', color);

    for item in arr {
        if let Value::Object(obj) = item {
            let row: Vec<String> = all_keys
                .iter()
                .map(|key| obj.get(key).map_or("".to_string(), |v| format_value(v)))
                .collect();
            print_wide_table_row(&row, &column_widths, color);
        }
    }

    print_wide_table_border(&column_widths, '└', '┴', '┘', color);
}

fn print_table_border(
    key_width: usize,
    value_width: usize,
    left: char,
    middle: char,
    right: char,
    color: &str,
) {
    println!(
        "{}{}{}{}{}{}",
        left.to_string().color(color),
        "─".repeat(key_width + 2).color(color),
        middle.to_string().color(color),
        "─".repeat(value_width + 2).color(color),
        right.to_string().color(color),
        ""
    );
}

fn print_table_row(key: &str, value: &str, key_width: usize, value_width: usize, color: &str) {
    println!(
        "{} {:<key_width$} {} {:<value_width$} {}",
        "│".color(color),
        key,
        "│".color(color),
        value,
        "│".color(color),
    );
}

fn print_wide_table_border(widths: &[usize], left: char, middle: char, right: char, color: &str) {
    print!("{}", left.to_string().color(color));
    for (i, &width) in widths.iter().enumerate() {
        print!("{}", "─".repeat(width + 2).color(color));
        if i < widths.len() - 1 {
            print!("{}", middle.to_string().color(color));
        }
    }
    println!("{}", right.to_string().color(color));
}

fn print_wide_table_row(values: &[String], widths: &[usize], color: &str) {
    print!("{}", "│".color(color));
    for (value, &width) in values.iter().zip(widths) {
        print!(" {:<width$} {}", value, "│".color(color));
    }
    println!();
}

fn format_value(value: &Value) -> String {
    match value {
        Value::Null => "null".to_string(),
        Value::Bool(b) => b.to_string(),
        Value::Number(n) => n.to_string(),
        Value::String(s) => s.clone(),
        Value::Array(a) => format!("[{}]", a.len()),
        Value::Object(o) => format!("{{{}}}", o.len()),
    }
}
