use std::process::Command;
use std::env;

fn main() {
    let lib_root = env::var("CARGO_MANIFEST_DIR").unwrap();
    Command::new("make").status().unwrap();

    println!("cargo:rustc-link-search=native={}/.libs", lib_root);
    println!("cargo:rustc-link-lib=static=freeciv");
    println!("cargo:rustc-link-lib=dylib=MagickWand-6.Q16");
    println!("cargo:rustc-link-lib=dylib=MagickCore-6.Q16");
}
