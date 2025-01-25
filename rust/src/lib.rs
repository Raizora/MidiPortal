use rand::Rng; // Import the random number generator

#[repr(C)] // Ensures C-compatible memory layout
pub struct ColorWithOpacity {
    pub hue: f32,
    pub saturation: f32,
    pub value: f32,
    pub opacity: f32,
}

#[repr(C)] // Ensures C-compatible memory layout
pub struct Position {
    pub x: f32,
    pub y: f32,
}

/// Translates a MIDI note and velocity into a color with opacity.
#[no_mangle]
pub extern "C" fn midi_note_to_color_with_opacity(note: u8, velocity: u8) -> ColorWithOpacity {
    let hue = (note as f32) / 127.0; // Normalize note to hue
    let saturation = 1.0;
    let value = 1.0;
    let opacity = (velocity as f32) / 127.0; // Normalize velocity to opacity
    ColorWithOpacity {
        hue,
        saturation,
        value,
        opacity,
    }
}

/// Generates a random position.
#[no_mangle]
pub extern "C" fn generate_position() -> Position {
    let mut rng = rand::thread_rng();
    Position {
        x: rng.gen_range(0.0..1.0),
        y: rng.gen_range(0.0..1.0),
    }
}