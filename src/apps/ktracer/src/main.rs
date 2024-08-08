use std::fs::File;
use std::io::{BufRead, BufReader, Seek, SeekFrom};
use std::sync::mpsc::{channel, Receiver, Sender};
use std::thread;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

use eframe::egui;
use serde_json::Value;
use structopt::StructOpt;

#[derive(Debug, StructOpt)]
#[structopt(name = "ktracer", about = "A GUI log viewer for Mirus")]
struct Opt {
    #[structopt(short, long, parse(from_os_str))]
    file: std::path::PathBuf,
}

#[derive(Clone, Debug)]
struct LogEntry {
    timestamp: SystemTime,
    level: String,
    component: String,
    message: String,
    data: Option<Value>,
}

struct LogViewer {
    log_entries: Vec<LogEntry>,
    selected_entry: Option<usize>,
    rx: Receiver<LogEntry>,
    search_term: String,
}

impl LogViewer {
    fn new(rx: Receiver<LogEntry>) -> Self {
        Self {
            log_entries: Vec::new(),
            selected_entry: None,
            rx,
            search_term: String::new(),
        }
    }

    fn update(&mut self) {
        while let Ok(entry) = self.rx.try_recv() {
            self.log_entries.push(entry);
        }
    }
}

impl eframe::App for LogViewer {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        self.update();

        egui::SidePanel::left("log_list").show(ctx, |ui| {
            ui.heading("Log Entries");
            ui.add(egui::TextEdit::singleline(&mut self.search_term).hint_text("Search..."));

            egui::ScrollArea::vertical().show(ui, |ui| {
                for (index, entry) in self.log_entries.iter().enumerate().rev() {
                    if self.search_term.is_empty()
                        || entry
                            .message
                            .to_lowercase()
                            .contains(&self.search_term.to_lowercase())
                    {
                        let label = format!(
                            "{} [{}] {} - {}",
                            format_time(entry.timestamp),
                            entry.level,
                            entry.component,
                            entry.message
                        );
                        if ui
                            .selectable_label(self.selected_entry == Some(index), label)
                            .clicked()
                        {
                            self.selected_entry = Some(index);
                        }
                    }
                }
            });
        });

        egui::CentralPanel::default().show(ctx, |ui| {
            if let Some(index) = self.selected_entry {
                if let Some(entry) = self.log_entries.get(index) {
                    ui.heading("Log Entry Details");
                    ui.horizontal(|ui| {
                        ui.label(format!("Timestamp: {}", format_time(entry.timestamp)));
                        ui.label(format!("Level: {}", entry.level));
                        ui.label(format!("Component: {}", entry.component));
                    });
                    ui.label(format!("Message: {}", entry.message));
                    if let Some(data) = &entry.data {
                        ui.collapsing("Data", |ui| {
                            ui.label(format_json(data));
                        });
                    }
                }
            } else {
                ui.centered_and_justified(|ui| {
                    ui.heading("Select a log entry to view details");
                });
            }
        });

        ctx.request_repaint();
    }
}

fn format_time(time: SystemTime) -> String {
    time.duration_since(UNIX_EPOCH)
        .map(|d| {
            let secs = d.as_secs();
            format!(
                "{:02}:{:02}:{:02}",
                (secs / 3600) % 24,
                (secs / 60) % 60,
                secs % 60
            )
        })
        .unwrap_or_else(|_| "Invalid time".to_string())
}

fn format_json(value: &Value) -> String {
    match value {
        Value::Object(map) => {
            let mut result = String::new();
            for (key, value) in map {
                result.push_str(&format!("{}: {}\n", key, format_json(value)));
            }
            result
        }
        Value::Array(arr) => {
            format!(
                "[{}]",
                arr.iter().map(format_json).collect::<Vec<_>>().join(", ")
            )
        }
        _ => value.to_string(),
    }
}

fn parse_log_entry(line: &str) -> Option<LogEntry> {
    let parsed: Value = serde_json::from_str(line).ok()?;
    let obj = parsed.as_object()?;

    Some(LogEntry {
        timestamp: SystemTime::now(),
        level: obj.get("level")?.as_str()?.to_string(),
        component: obj.get("component")?.as_str()?.to_string(),
        message: obj.get("message")?.as_str()?.to_string(),
        data: obj.get("data").cloned(),
    })
}

fn tail_log(file_path: std::path::PathBuf, tx: Sender<LogEntry>) {
    let file = File::open(&file_path).expect("Failed to open file");
    let mut reader = BufReader::new(file);
    let mut line = String::new();

    loop {
        match reader.read_line(&mut line) {
            Ok(0) => {
                thread::sleep(Duration::from_millis(100));
                reader.seek(SeekFrom::Current(0)).unwrap();
            }
            Ok(_) => {
                if let Some(entry) = parse_log_entry(&line) {
                    tx.send(entry).unwrap();
                }
                line.clear();
            }
            Err(e) => eprintln!("Error reading line: {}", e),
        }
    }
}

fn main() -> Result<(), eframe::Error> {
    let opt = Opt::from_args();
    let (tx, rx) = channel();

    thread::spawn(move || {
        tail_log(opt.file, tx);
    });

    let options = eframe::NativeOptions {
        initial_window_size: Some(egui::vec2(1024.0, 768.0)),
        ..Default::default()
    };
    eframe::run_native(
        "ktracer im already tracer",
        options,
        Box::new(|_cc| Box::new(LogViewer::new(rx))),
    )
}
