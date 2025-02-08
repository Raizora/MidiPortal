use crate::{MidiError, RustMidiStats};

// MPE SysEx message types
const MCM_UNIVERSAL_SYSEX: u8 = 0x7E;
const MCM_ALL_CHANNELS: u8 = 0x7F;
const MCM_SUB_ID: u8 = 0x06;
const MCM_SET_ZONE: u8 = 0x02;
const MCM_SET_BEND_RANGE: u8 = 0x03;

#[derive(Debug, Clone, Copy, PartialEq)]
enum MpeInitState {
    NotInitialized,
    AllNotesOff,
    ControllersReset,
    ZoneConfigured,
    BendRangeSet,
    Ready,
}

pub struct MpeInitTracker {
    state: MpeInitState,
    last_message_time: f64,
    timeout: f64,  // Timeout for initialization sequence
}

impl MpeInitTracker {
    pub fn new() -> Self {
        Self {
            state: MpeInitState::NotInitialized,
            last_message_time: 0.0,
            timeout: 1.0,  // 1 second timeout
        }
    }

    pub fn process_message(&mut self, message_type: u8, timestamp: f64) -> Result<(), MidiError> {
        // Check for timeout
        if timestamp - self.last_message_time > self.timeout 
            && self.state != MpeInitState::NotInitialized 
            && self.state != MpeInitState::Ready {
            self.state = MpeInitState::NotInitialized;
            return Err(MidiError::TimingError("MPE initialization timeout".into()));
        }

        self.last_message_time = timestamp;

        match (self.state, message_type) {
            (MpeInitState::NotInitialized, 0x7B) => {  // All Notes Off
                self.state = MpeInitState::AllNotesOff;
                Ok(())
            },
            (MpeInitState::AllNotesOff, 0x79) => {     // Reset All Controllers
                self.state = MpeInitState::ControllersReset;
                Ok(())
            },
            (MpeInitState::ControllersReset, MCM_SET_ZONE) => {
                self.state = MpeInitState::ZoneConfigured;
                Ok(())
            },
            (MpeInitState::ZoneConfigured, MCM_SET_BEND_RANGE) => {
                self.state = MpeInitState::Ready;
                Ok(())
            },
            _ => Err(MidiError::ProcessingError(
                "Unexpected MPE initialization message".into()
            )),
        }
    }

    pub fn is_initialized(&self) -> bool {
        self.state == MpeInitState::Ready
    }
}

pub fn process_sysex(data: &[u8], timestamp: f64, stats: &mut RustMidiStats) -> Result<(), MidiError> {
    // Validate basic SysEx format
    if data.len() < 3 || data[0] != 0xF0 || data[data.len() - 1] != 0xF7 {
        return Err(MidiError::InvalidData("Invalid SysEx message format".into()));
    }

    match data[1] {
        MCM_UNIVERSAL_SYSEX => process_mcm_message(&data[2..], timestamp, stats),
        _ => Ok(()) // Ignore other manufacturer SysEx messages
    }
}

fn process_mcm_message(data: &[u8], timestamp: f64, stats: &mut RustMidiStats) -> Result<(), MidiError> {
    if data.len() < 3 {
        return Err(MidiError::InvalidData("MCM message too short".into()));
    }

    if data[0] != MCM_ALL_CHANNELS || data[1] != MCM_SUB_ID {
        return Ok(()); // Not an MPE message
    }

    match data[2] {
        MCM_SET_ZONE => {
            if data.len() < 4 {
                return Err(MidiError::InvalidData("Invalid zone configuration message".into()));
            }
            let zone_config = data[3];
            let is_lower_zone = (zone_config & 0x10) == 0;
            let member_channels = zone_config & 0x0F;

            stats.mpe_config.configure_zone(
                is_lower_zone,
                member_channels,
                48.0 // Default to 48 semitones pitch bend range
            );

            // Log the MPE configuration
            log::info!("MPE Zone Configured: {} zone with {} member channels",
                      if is_lower_zone { "Lower" } else { "Upper" },
                      member_channels);
        }

        MCM_SET_BEND_RANGE => {
            if data.len() < 5 {
                return Err(MidiError::InvalidData("Invalid pitch bend range message".into()));
            }
            let pitch_msb = data[3];
            let pitch_lsb = data[4];
            let bend_range = ((pitch_msb as u16) << 7 | pitch_lsb as u16) as f64;

            // Update the bend range for the active zone
            // TODO: Implement bend range update in MpeConfiguration
            log::info!("MPE Pitch Bend Range set to {} semitones", bend_range);
        }

        _ => {
            log::debug!("Unhandled MCM message type: {:02X}", data[2]);
        }
    }

    Ok(())
} 