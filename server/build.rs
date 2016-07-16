use std::process::Command;
use std::env;

fn main() {
    let lib_root = env::var("CARGO_MANIFEST_DIR").unwrap();
    Command::new("make").status().unwrap();

    println!("cargo:rustc-link-search=native={}/.libs", lib_root);
    println!("cargo:rustc-link-lib=static=freeciv");
    println!("cargo:rustc-link-lib=static=freeciv-srv");
    println!("cargo:rustc-link-lib=dylib=lua");
    println!("cargo:rustc-link-lib=dylib=curl");
    println!("cargo:rustc-link-lib=dylib=z");
    println!("cargo:rustc-link-lib=dylib=bz2");
    println!("cargo:rustc-link-lib=dylib=lzma");
    println!("cargo:rustc-link-lib=dylib=readline");
}
