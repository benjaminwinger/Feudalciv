use std::process::Command;
use std::env;

fn main() {
    let lib_root = env::var("CARGO_MANIFEST_DIR").unwrap();
    Command::new("make").status().unwrap();

    println!("cargo:rustc-link-search=native={}/.libs", lib_root);
    println!("cargo:rustc-link-lib=static=scriptcore");
}
