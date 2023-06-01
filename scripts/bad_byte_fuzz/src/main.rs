use rand::Rng;

fn has_bad_byte(qword: u64) -> bool {
    // check every byte in the qword
    for i in 0..8 {
        let b = (qword >> (i * 8)) & 0xff;
        if matches!(b, 0xc3 | 0xc2 | 0xcb | 0xca | 0xcf) {
            return true;
        }
    }
    false
}

fn split_into_two_safe(qword: u64) -> Option<(u64, u64)> {
    let mut left = qword / 2;
    let mut right = qword - left;

    // set the heuristic to the original value
    let mut heuristic = qword;

    // now, we try extra hard to fix it B)
    let mut left_bad = has_bad_byte(left);
    let mut right_bad = has_bad_byte(right);
    let mut i = 0;
    while left_bad || right_bad {
        if (has_bad_byte(left + right)) {
            println!("left: 0x{left:x}, right: 0x{right:x}");
        }
        if heuristic == 0 {
            return None;
        } else {
            heuristic /= 2;
        }

        if i % 2 == 0 {
            // try to fix the left side
            let l1 = left / 2;
            let l2 = left - l1;
            left = l1;
            right += l2;
        } else {
            // try to fix the right side
            let r1 = right / 2;
            let r2 = right - r1;
            right = r1;
            left += r2;
        }

        left_bad = has_bad_byte(left);
        right_bad = has_bad_byte(right);
        i += 1;
    }
    Some((left, right))
}

fn main() {
    let bad1 = 0x000000000000c300;
    let bad2 = 0x000000000000c200;
    let bad3 = 0xc200;
    let bad4 = 0xc3;
    let bad5 = 0xc3432343;
    let good1 = 0x0000000004343434;
    let good2 = 0x0000000000000000;
    let good3 = 0x0000000000000001;
    let good4 = 0x000432406;
    assert!(has_bad_byte(bad1));
    assert!(has_bad_byte(bad2));
    assert!(has_bad_byte(bad3));
    assert!(has_bad_byte(bad4));
    assert!(has_bad_byte(bad5));
    assert!(!has_bad_byte(good1));
    assert!(!has_bad_byte(good2));
    assert!(!has_bad_byte(good3));
    assert!(!has_bad_byte(good4));

    // fuzz the algo
    let mut rng = rand::thread_rng();
    let max = 100000000;
    for i in 0..max {
        if i % 1000000 == 0 {
            println!("fuzzing: {}/{}", i, max);
        }
        let b = rng.gen_range(0..3000000);
        let (left, right) = match split_into_two_safe(b) {
            Some(s) => s,
            None => {
                println!("unsplittable: 0x{b:x}");
                continue;
            }
        };
        assert!(left + right == b);
        assert!(!has_bad_byte(left));
        assert!(!has_bad_byte(right));
    }
}
