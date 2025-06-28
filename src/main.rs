use std::{iter::Peekable, str::Chars};

type Result<T> = std::result::Result<T, Box<dyn std::error::Error>>;

#[derive(Debug)]
enum Op {
    Inc(usize),
    Dec(usize),
    Left(usize),
    Right(usize),
    Input,
    Output(usize),
    JmpIfZero(usize),
    JmpIfNonZero(usize),
}

fn compact(c: char, chars: &mut Peekable<Chars<'_>>) -> usize {
    let mut count = 1;
    while let Some(&n) = chars.peek() {
        if c != n {
            break;
        }
        count += 1;
        chars.next();
    }
    count
}

fn compile(source: &str) -> Result<Vec<Op>> {
    let mut code = Vec::new();
    let mut stack = Vec::new();
    let mut chars = source.chars().peekable();
    while let Some(c) = chars.next() {
        match c {
            '+' => code.push(Op::Inc(compact(c, &mut chars))),
            '-' => code.push(Op::Dec(compact(c, &mut chars))),
            '>' => code.push(Op::Right(compact(c, &mut chars))),
            '<' => code.push(Op::Left(compact(c, &mut chars))),
            '.' => code.push(Op::Output(compact(c, &mut chars))),
            '[' => {
                stack.push(code.len());
                code.push(Op::JmpIfZero(0));
            }
            ']' => {
                let Some(index) = stack.pop() else {
                    return Err("Loop missmatch.".into());
                };
                code.push(Op::JmpIfNonZero(index + 1));
                code[index] = Op::JmpIfZero(code.len());
            }
            _ => {}
        }
    }
    Ok(code)
}

fn interpret(source: &str) -> Result<()> {
    let code = compile(source)?;
    for (index, op) in code.iter().enumerate() {
        println!("{index}: {op:?}");
    }

    let mut mem = [0u8; 30_000];
    let (mut dp, mut ip) = (0, 0);

    while ip < code.len() {
        match code[ip] {
            Op::Inc(n) => {
                mem[dp] += n as u8;
                ip += 1;
            }
            Op::Dec(n) => {
                mem[dp] -= n as u8;
                ip += 1;
            }
            Op::Left(n) => {
                if dp < n {
                    return Err("Memory underflow".into());
                }
                dp -= n;
                ip += 1;
            }
            Op::Right(n) => {
                dp += n;
                if dp >= mem.len() {
                    return Err("Memory overflow".into());
                }
                ip += 1;
            }
            Op::Input => todo!(),
            Op::Output(n) => {
                (0..n).for_each(|_| print!("{}", mem[dp] as char));
                ip += 1;
            }
            Op::JmpIfZero(n) => {
                if mem[dp] == 0 {
                    ip = n;
                } else {
                    ip += 1;
                }
            }
            Op::JmpIfNonZero(n) => {
                if mem[dp] != 0 {
                    ip = n;
                } else {
                    ip += 1;
                }
            }
        }
    }

    Ok(())
}

fn main() -> Result<()> {
    let args = std::env::args().collect::<Vec<String>>();
    if args.len() != 2 {
        eprintln!("Usage: bf [path]");
        std::process::exit(64);
    }

    let source = std::fs::read_to_string(&args[1])?;

    if let Err(e) = interpret(&source) {
        eprintln!("Error: {e}");
        std::process::exit(70);
    }

    Ok(())
}
