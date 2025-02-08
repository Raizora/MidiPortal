//! MIDI message processing module v0.1.1
//! Part of MidiPortal Rust Engine
use crate::{MidiError, RustMidiStats};
use std::panic::{catch_unwind, AssertUnwindSafe};

pub(crate) fn process_message(
    data: &[u8], 
    timestamp: f64, 
    stats: &mut RustMidiStats
) -> Result<(), MidiError> {
    if data.is_empty() {
        return Err(MidiError::InvalidData("Empty MIDI message".into()));
    }

    match data[0] {
        0xF8 => process_timing_clock(timestamp, stats),  // MIDI Clock
        0x90 => process_note_on(data[1], data[2], data[0] & 0x0F, timestamp, stats),
        0x80 => process_note_off(data[1], data[2], data[0] & 0x0F, timestamp, stats),
        0xB0 => process_cc(data[1], data[2], data[0] & 0x0F, timestamp, stats),
        0xE0 => {
            if data.len() >= 3 {
                process_pitch_bend(data[1], data[2], data[0] & 0x0F, timestamp, stats)
            } else {
                Err(MidiError::InvalidData("Incomplete pitch bend message".into()))
            }
        },
        0xFA => process_start(timestamp, stats),        // Start
        0xFC => process_stop(timestamp, stats),         // Stop
        _ => Ok(()) // Other message types
    }
}

fn parse_note_data(data: &[u8]) -> Result<(u8, u8), MidiError> {
    if data.len() < 3 {
        return Err(MidiError::InvalidData("Incomplete note message".into()));
    }
    Ok((data[1], data[2])) // note number, velocity
}

fn parse_cc_data(data: &[u8]) -> Result<(u8, u8), MidiError> {
    if data.len() < 3 {
        return Err(MidiError::InvalidData("Incomplete CC message".into()));
    }
    Ok((data[1], data[2])) // controller number, value
}

fn process_note_on(note: u8, velocity: u8, channel: u8, timestamp: f64, stats: &mut RustMidiStats) -> Result<(), MidiError> {
    if note > 127 || channel > 15 {
        return Err(MidiError::InvalidData("Note or channel out of range".into()));
    }

    match catch_unwind(AssertUnwindSafe(|| {
        if velocity > 0 {
            stats.note_tracker.note_on(note, channel, velocity, timestamp);
        } else {
            // Note-on with velocity 0 is equivalent to note-off
            stats.note_tracker.note_off(note, channel, timestamp);
        }
    })) {
        Ok(_) => Ok(()),
        Err(_) => Err(MidiError::ProcessingError("Note on processing failed".into()))
    }
}

fn process_note_off(note: u8, _velocity: u8, channel: u8, timestamp: f64, stats: &mut RustMidiStats) -> Result<(), MidiError> {
    if note > 127 || channel > 15 {
        return Err(MidiError::InvalidData("Note or channel out of range".into()));
    }

    match catch_unwind(AssertUnwindSafe(|| {
        stats.note_tracker.note_off(note, channel, timestamp)
    })) {
        Ok(_) => Ok(()),
        Err(_) => Err(MidiError::ProcessingError("Note off processing failed".into()))
    }
}

fn process_cc(controller: u8, value: u8, channel: u8, timestamp: f64, stats: &mut RustMidiStats) -> Result<(), MidiError> {
    match controller {
        1 => process_modulation_wheel(value, channel, timestamp, stats),
        74 => process_mpe_brightness(value, channel, timestamp, stats),
        11 => process_mpe_expression(value, channel, timestamp, stats),
        _ => Ok(())
    }
}

fn process_timing_clock(timestamp: f64, stats: &mut RustMidiStats) -> Result<(), MidiError> {
    stats.clock_count += 1;
    
    if stats.last_clock_time > 0.0 {
        let interval = timestamp - stats.last_clock_time;
        
        // MIDI clock is 24 pulses per quarter note
        let instant_bpm = 60.0 / (interval * 24.0);
        
        // Update stats with smoothing
        stats.current_bpm = (stats.current_bpm * 0.9) + (instant_bpm * 0.1);
        
        // Update running average
        stats.average_bpm = ((stats.average_bpm * (stats.clock_count as f64 - 1.0)) 
                            + instant_bpm) / stats.clock_count as f64;
        
        // Calculate jitter (timing variation)
        let expected_interval = 60.0 / (stats.current_bpm * 24.0);
        let jitter = (interval - expected_interval).abs();
        stats.jitter = (stats.jitter * 0.9) + (jitter * 0.1);
    }
    
    stats.last_clock_time = timestamp;
    Ok(())
}

// Add parsing functions for new message types
fn parse_pitch_bend_data(data: &[u8]) -> Result<(u8, u8), MidiError> {
    if data.len() < 3 {
        return Err(MidiError::InvalidData("Incomplete pitch bend message".into()));
    }
    Ok((data[1], data[2])) // LSB, MSB
}

// MPE-specific processing
fn process_mpe_modulation(_value: u8, _channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    let _normalized = _value as f64 / 127.0;
    Ok(())
}

fn process_mpe_brightness(_value: u8, _channel: u8, _timestamp: f64, _stats: &mut RustMidiStats) -> Result<(), MidiError> {
    let _normalized = _value as f64 / 127.0;
    Ok(())
}

fn process_mpe_expression(_value: u8, _channel: u8, _timestamp: f64, _stats: &mut RustMidiStats) -> Result<(), MidiError> {
    let _normalized = _value as f64 / 127.0;
    Ok(())
}

// Standard CC processing
fn process_modulation_wheel(_value: u8, _channel: u8, _timestamp: f64, _stats: &mut RustMidiStats) -> Result<(), MidiError> {
    let _normalized = _value as f64 / 127.0;
    Ok(())
}

fn process_pitch_bend(lsb: u8, msb: u8, channel: u8, timestamp: f64, stats: &mut RustMidiStats) -> Result<(), MidiError> {
    let combined = ((msb as u16) << 7) | (lsb as u16);
    let normalized = (combined as f64 - 8192.0) / 8192.0; // -1.0 to +1.0
    stats.note_tracker.update_pitch_bend(channel, normalized, timestamp);
    Ok(())
}

fn process_poly_aftertouch(_note: u8, _pressure: u8, _channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    let _normalized = _pressure as f64 / 127.0;
    Ok(())
}

fn process_channel_pressure(_pressure: u8, _channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    let _normalized = _pressure as f64 / 127.0;
    Ok(())
}

// System messages
fn process_mtc_quarter_frame(data: &[u8], _timestamp: f64) -> Result<(), MidiError> {
    if data.len() < 2 {
        return Err(MidiError::InvalidData("MTC message too short".into()));
    }
    let _mtc_data = data[1];
    let _message_type = _mtc_data >> 4;
    let _value = _mtc_data & 0x0F;
    Ok(())
}

fn process_song_position(data: &[u8], _timestamp: f64) -> Result<(), MidiError> {
    if data.len() < 3 {
        return Err(MidiError::InvalidData("Song position message too short".into()));
    }
    let _position = ((data[2] as u16) << 7) | (data[1] as u16);
    Ok(())
}

fn process_volume(_value: u8, _channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    let _normalized = _value as f64 / 127.0;
    Ok(())
}

fn process_pan(_value: u8, _channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    let _normalized = (_value as f64 - 64.0) / 64.0;
    Ok(())
}

fn process_sustain(_value: u8, _channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    let _is_on = _value >= 64;
    Ok(())
}

#[allow(dead_code)]
fn process_brightness(_value: u8, _channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    let _normalized = _value as f64 / 127.0;
    Ok(())
}

#[allow(dead_code)]
fn process_reverb(_value: u8, _channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    let _normalized = _value as f64 / 127.0;
    Ok(())
}

fn process_chorus(_value: u8, _channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    let _normalized = _value as f64 / 127.0;
    Ok(())
}

fn process_rpn_nrpn(controller: u8, value: u8, channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    // Keep these variables without underscore since they're used in the match
    static mut RPN_MSB: [u8; 16] = [0; 16];
    static mut RPN_LSB: [u8; 16] = [0; 16];
    static mut NRPN_MSB: [u8; 16] = [0; 16];
    static mut NRPN_LSB: [u8; 16] = [0; 16];
    
    unsafe {
        match controller {
            0x62 => NRPN_LSB[channel as usize] = value,
            0x63 => NRPN_MSB[channel as usize] = value,
            0x64 => RPN_LSB[channel as usize] = value,
            0x65 => RPN_MSB[channel as usize] = value,
            _ => return Ok(()),
        }
    }
    Ok(())
}

fn process_program_change(_program: u8, _channel: u8, _timestamp: f64) -> Result<(), MidiError> {
    Ok(())
}

fn process_song_select(data: &[u8], _timestamp: f64) -> Result<(), MidiError> {
    if data.len() < 2 {
        return Err(MidiError::InvalidData("Song select message too short".into()));
    }
    let _song = data[1];
    Ok(())
}

fn process_tune_request(_timestamp: f64) -> Result<(), MidiError> {
    Ok(())
}

fn process_start(timestamp: f64, stats: &mut RustMidiStats) -> Result<(), MidiError> {
    stats.clock_count = 0;
    stats.last_clock_time = timestamp;
    Ok(())
}

fn process_continue(_timestamp: f64) -> Result<(), MidiError> {
    Ok(())
}

fn process_stop(_timestamp: f64, _stats: &mut RustMidiStats) -> Result<(), MidiError> {
    Ok(())
}

fn process_active_sensing(_timestamp: f64) -> Result<(), MidiError> {
    Ok(())
}

fn process_system_reset(_timestamp: f64) -> Result<(), MidiError> {
    Ok(())
} 