// Type inference with optional types (no 'let/var' noise)
x := 42              // infer type
y: u64 = 100         // explicit

// Functions - clean signature

fn calculate(base: i32, power: i32) -> i32 {
    result := 1
    for i in 0..power {
        result *= base
    }
    return result
}

// Conditionals - parens optional
if x > 10 {
    print("large")
}

// While loops
while condition {
    // ...
}

// C-style pointers (familiar)
fn modify(ptr: *i32) {
    *ptr = 42
}

// Inline assembly (critical for kernel)
fn read_cr3() -> u64 {
    asm {
        mov rax, cr3
        ret
    }
}

// Structs - clean and packed
struct Vec3 {
    x: f32
    y: f32
    z: f32
}

// Protocol - virtual classes?
protocol Vec3_F {
	Vec3 var
	fn get()
	fn store()
	...
}

// Methods
impl Vec3 {
    fn length() -> f32 {
        return sqrt(x*x + y*y + z*z)  // implicit self access
    }
}

// Compile-time execution
const PAGE_SIZE: usize = 4096;
const BUFFER_SIZE: usize = PAGE_SIZE * 4;  // computed at compile time

// Entry point
fn main() {
    print("Hello, World!")
}