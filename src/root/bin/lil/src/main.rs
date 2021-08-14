use uwuline::Editor;

fn main() {
    let mut editor = Editor::new();

    loop {
        let s = editor.readline("> ");
        println!("received {:?}", s);
    }
}
