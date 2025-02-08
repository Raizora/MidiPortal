class SettingsComponent : public juce::Component {
public:
    struct MidiFilterSettings {
        bool logNotes = true;
        bool logCC = true;
        bool logClock = true;
        bool logSysEx = true;
        // etc...
    };

    struct DeviceFilterSettings {
        std::map<juce::String, bool> deviceEnabled;  // device name -> enabled
        std::map<juce::String, bool> deviceLogging;  // device name -> logging enabled
    };
}; 