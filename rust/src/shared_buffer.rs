use std::sync::atomic::{AtomicUsize, Ordering};
use std::time::SystemTime;

/// Represents a MIDI event with timestamp and device information
#[derive(Debug, Clone)]
pub struct MidiEvent {
    /// Raw MIDI data bytes
    pub data: Vec<u8>,
    /// Timestamp in microseconds since epoch
    pub timestamp: u64,
    /// Name of the device that generated this event
    pub device_name: String,
}

/// A lock-free ring buffer for sharing MIDI data between C++ and Rust
pub struct SharedMidiBuffer {
    /// Pointer to the shared memory region
    buffer: *mut u8,
    /// Total capacity of the buffer in bytes
    capacity: usize,
    /// Current write position (atomic for thread safety)
    write_pos: AtomicUsize,
    /// Current read position (atomic for thread safety)
    read_pos: AtomicUsize,
    /// Whether this instance owns the buffer (should free memory on drop)
    owns_buffer: bool,
}

// Safety: We need to manually implement Send and Sync since we're using raw pointers
// This is safe because we use atomic operations for all shared access
unsafe impl Send for SharedMidiBuffer {}
unsafe impl Sync for SharedMidiBuffer {}

impl SharedMidiBuffer {
    /// Creates a new shared buffer with the specified capacity
    pub fn new(capacity: usize) -> Self {
        // Allocate memory for the buffer
        let buffer = unsafe {
            libc::malloc(capacity) as *mut u8
        };
        
        Self {
            buffer,
            capacity,
            write_pos: AtomicUsize::new(0),
            read_pos: AtomicUsize::new(0),
            owns_buffer: true,
        }
    }
    
    /// Creates a shared buffer from an existing memory address
    /// 
    /// # Safety
    /// 
    /// The caller must ensure that:
    /// - The pointer points to a valid memory region of at least `capacity` bytes
    /// - The memory remains valid for the lifetime of this object
    /// - No other code will free this memory while this object exists
    pub unsafe fn from_raw(buffer: *mut u8, capacity: usize) -> Self {
        Self {
            buffer,
            capacity,
            write_pos: AtomicUsize::new(0),
            read_pos: AtomicUsize::new(0),
            owns_buffer: false,
        }
    }
    
    /// Gets the raw pointer to the buffer
    pub fn as_ptr(&self) -> *const u8 {
        self.buffer
    }
    
    /// Gets the mutable raw pointer to the buffer
    pub fn as_mut_ptr(&self) -> *mut u8 {
        self.buffer
    }
    
    /// Writes a MIDI event to the buffer
    /// 
    /// Returns true if the write was successful, false if the buffer is full
    pub fn write(&self, event: &MidiEvent) -> bool {
        // Calculate the total size needed for this event
        let data_len = event.data.len();
        let device_name_len = event.device_name.len();
        let total_size = 8 + 4 + data_len + 4 + device_name_len;
        
        // Check if there's enough space in the buffer
        let write_pos = self.write_pos.load(Ordering::Relaxed);
        let read_pos = self.read_pos.load(Ordering::Relaxed);
        
        let available_space = if write_pos >= read_pos {
            self.capacity - (write_pos - read_pos)
        } else {
            read_pos - write_pos
        };
        
        if total_size + 4 > available_space {
            return false; // Not enough space
        }
        
        // Write the event to the buffer
        unsafe {
            let mut pos = write_pos;
            
            // Write total size (for easy skipping when reading)
            *(self.buffer.add(pos) as *mut u32) = total_size as u32;
            pos += 4;
            
            // Write timestamp
            *(self.buffer.add(pos) as *mut u64) = event.timestamp;
            pos += 8;
            
            // Write data length and data
            *(self.buffer.add(pos) as *mut u32) = data_len as u32;
            pos += 4;
            std::ptr::copy_nonoverlapping(
                event.data.as_ptr(),
                self.buffer.add(pos),
                data_len
            );
            pos += data_len;
            
            // Write device name length and device name
            *(self.buffer.add(pos) as *mut u32) = device_name_len as u32;
            pos += 4;
            std::ptr::copy_nonoverlapping(
                event.device_name.as_ptr(),
                self.buffer.add(pos),
                device_name_len
            );
            pos += device_name_len;
            
            // Update write position atomically
            self.write_pos.store(pos % self.capacity, Ordering::Release);
        }
        
        true
    }
    
    /// Reads a MIDI event from the buffer
    /// 
    /// Returns Some(MidiEvent) if an event was read, None if the buffer is empty
    pub fn read(&self) -> Option<MidiEvent> {
        let read_pos = self.read_pos.load(Ordering::Relaxed);
        let write_pos = self.write_pos.load(Ordering::Acquire);
        
        if read_pos == write_pos {
            return None; // Buffer is empty
        }
        
        unsafe {
            let mut pos = read_pos;
            
            // Read total size
            let total_size = *(self.buffer.add(pos) as *const u32) as usize;
            pos += 4;
            
            // Read timestamp
            let timestamp = *(self.buffer.add(pos) as *const u64);
            pos += 8;
            
            // Read data
            let data_len = *(self.buffer.add(pos) as *const u32) as usize;
            pos += 4;
            let mut data = vec![0u8; data_len];
            std::ptr::copy_nonoverlapping(
                self.buffer.add(pos),
                data.as_mut_ptr(),
                data_len
            );
            pos += data_len;
            
            // Read device name
            let device_name_len = *(self.buffer.add(pos) as *const u32) as usize;
            pos += 4;
            let mut device_name_bytes = vec![0u8; device_name_len];
            std::ptr::copy_nonoverlapping(
                self.buffer.add(pos),
                device_name_bytes.as_mut_ptr(),
                device_name_len
            );
            pos += device_name_len;
            
            // Update read position atomically
            self.read_pos.store(pos % self.capacity, Ordering::Release);
            
            // Convert device name bytes to string
            let device_name = String::from_utf8_lossy(&device_name_bytes).to_string();
            
            Some(MidiEvent {
                data,
                timestamp,
                device_name,
            })
        }
    }
    
    /// Gets the current timestamp in microseconds
    pub fn current_timestamp() -> u64 {
        SystemTime::now()
            .duration_since(SystemTime::UNIX_EPOCH)
            .unwrap_or_default()
            .as_micros() as u64
    }
}

impl Drop for SharedMidiBuffer {
    fn drop(&mut self) {
        if self.owns_buffer && !self.buffer.is_null() {
            unsafe {
                libc::free(self.buffer as *mut libc::c_void);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_write_read() {
        let buffer = SharedMidiBuffer::new(1024);
        
        let event = MidiEvent {
            data: vec![0x90, 0x40, 0x7F],
            timestamp: 12345678,
            device_name: "Test Device".to_string(),
        };
        
        assert!(buffer.write(&event));
        
        let read_event = buffer.read().unwrap();
        assert_eq!(read_event.data, event.data);
        assert_eq!(read_event.timestamp, event.timestamp);
        assert_eq!(read_event.device_name, event.device_name);
        
        // Buffer should be empty now
        assert!(buffer.read().is_none());
    }
    
    #[test]
    fn test_multiple_events() {
        let buffer = SharedMidiBuffer::new(1024);
        
        for i in 0..10 {
            let event = MidiEvent {
                data: vec![0x90, i, 0x7F],
                timestamp: i as u64 * 1000,
                device_name: format!("Device {}", i),
            };
            
            assert!(buffer.write(&event));
        }
        
        for i in 0..10 {
            let event = buffer.read().unwrap();
            assert_eq!(event.data, vec![0x90, i, 0x7F]);
            assert_eq!(event.timestamp, i as u64 * 1000);
            assert_eq!(event.device_name, format!("Device {}", i));
        }
        
        // Buffer should be empty now
        assert!(buffer.read().is_none());
    }
} 