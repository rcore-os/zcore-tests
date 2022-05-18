use nix::{
    fcntl::{open, OFlag},
    sys::stat::Mode,
};

fn main() {
    let name = std::env::args()
        .nth(1)
        .expect("Usage: uart {uart device name, e.g. \"ttyS0\"}.");

    let path = format!("/dev/{name}");

    println!("Try to open {path}.");
    let fd = open(path.as_str(), OFlag::O_RDWR, Mode::empty()).unwrap();
    println!("Open {path} as fd({fd}).");

    println!("Try to write \"Hello, world!\" to {name}.");
    nix::unistd::write(fd, "Hello, world!".as_bytes()).unwrap();
    println!("Write succeed.");

    println!("Try to read from {name}.");
    let mut buf = [0u8; 64];
    let len = nix::unistd::read(fd, &mut buf).unwrap();
    println!("Read {len} bytes.");
}
