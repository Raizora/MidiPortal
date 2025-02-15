//! MPE support module v0.1.1
//! Part of MidiPortal Rust Engine

use crate::MidiError;

#[derive(Debug, Clone)]
pub struct MpeZone {
    pub master_channel: u8,        // Channel 1 or 16
    pub member_channels: Vec<u8>,  // Which channels belong to this zone
    pub pitch_bend_range: f64,     // Semitones for pitch bend
    pub is_active: bool,           // Whether this zone is currently enabled
}

#[derive(Debug)]
pub struct MpeConfiguration {
    lower_zone: Option<MpeZone>,  // Channels 1-15
    upper_zone: Option<MpeZone>,  // Channels 2-16
}

impl MpeConfiguration {
    pub fn new() -> Self {
        Self {
            lower_zone: None,
            upper_zone: None,
        }
    }

    pub fn configure_zone(&mut self, is_lower: bool, member_count: u8, bend_range: f64) {
        let zone = if is_lower {
            // Lower zone: master = 1, members = 2 to N+1
            let members = (2..=member_count+1).collect();
            MpeZone {
                master_channel: 1,
                member_channels: members,
                pitch_bend_range: bend_range,
                is_active: true,
            }
        } else {
            // Upper zone: master = 16, members = 15 down to 16-N
            let members = ((16-member_count)..=15).collect();
            MpeZone {
                master_channel: 16,
                member_channels: members,
                pitch_bend_range: bend_range,
                is_active: true,
            }
        };

        if is_lower {
            self.lower_zone = Some(zone);
        } else {
            self.upper_zone = Some(zone);
        }
    }

    pub fn is_mpe_channel(&self, channel: u8) -> bool {
        self.lower_zone.as_ref().map_or(false, |zone| 
            zone.is_active && zone.member_channels.contains(&channel))
        || self.upper_zone.as_ref().map_or(false, |zone| 
            zone.is_active && zone.member_channels.contains(&channel))
    }

    pub fn is_master_channel(&self, channel: u8) -> bool {
        (self.lower_zone.as_ref().map_or(false, |zone| 
            zone.is_active && zone.master_channel == channel))
        || (self.upper_zone.as_ref().map_or(false, |zone| 
            zone.is_active && zone.master_channel == channel))
    }

    pub fn get_bend_range(&self, channel: u8) -> f64 {
        if let Some(zone) = self.lower_zone.as_ref() {
            if zone.is_active && (zone.master_channel == channel 
                || zone.member_channels.contains(&channel)) {
                return zone.pitch_bend_range;
            }
        }
        if let Some(zone) = self.upper_zone.as_ref() {
            if zone.is_active && (zone.master_channel == channel 
                || zone.member_channels.contains(&channel)) {
                return zone.pitch_bend_range;
            }
        }
        2.0 // Default to 2 semitones if not in MPE mode
    }

    pub fn set_zone_bend_range(&mut self, is_lower: bool, bend_range: f64) {
        if let Some(zone) = if is_lower {
            &mut self.lower_zone
        } else {
            &mut self.upper_zone
        } {
            zone.pitch_bend_range = bend_range;
        }
    }

    pub fn validate_zone_state(&self) -> Result<(), MidiError> {
        // Check for overlapping zones
        if let (Some(lower), Some(upper)) = (&self.lower_zone, &self.upper_zone) {
            for channel in lower.member_channels.iter() {
                if upper.member_channels.contains(channel) {
                    return Err(MidiError::InvalidData(
                        format!("Channel {} is in both zones", channel)
                    ));
                }
            }
        }
        Ok(())
    }
} 