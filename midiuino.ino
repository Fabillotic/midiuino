void setup() {
  Serial.begin(9600);
  Serial2.begin(31250, SERIAL_8N1, 22, 23);
}

void loop() {
  int x = Serial2.read();
  if(x > 0) {
    if(x != 0xFE && x != 0xF8) {
      Serial.println(x, HEX);
    }
  }
}

#define MIDI_MESSAGE_NONE 0 // Special type reserved as invalid message

#define MIDI_MESSAGE_NOTE_OFF 1
struct MIDIMessageNoteOff {
  int channel;
  int note;
  int velocity;
};

#define MIDI_MESSAGE_NOTE_ON 2
struct MIDIMessageNoteOn {
  int channel;
  int note;
  int velocity;
};

#define MIDI_MESSAGE_NOTE_AFTERTOUCH 3
struct MIDIMessageNoteAftertouch {
  int channel;
  int note;
  int velocity;
};

#define MIDI_MESSAGE_CONTROL_CHANGE 4
struct MIDIMessageControlChange {
  int channel;
  int control;
  int value;
};

#define MIDI_MESSAGE_PROGRAM_CHANGE 5
struct MIDIMessageProgramChange {
  int channel;
  int program;
};

#define MIDI_MESSAGE_CHANNEL_AFTERTOUCH 6
struct MIDIMessageChannelAftertouch {
  int channel;
  int note;
  int velocity;
};

#define MIDI_MESSAGE_PITCH_BEND 7
struct MIDIMessagePitchBend {
  int channel;
  int value;
};

class MIDI {
  public:
  MIDI();
  void begin(Stream* stream);
  void begin(HardwareSerial& serial);
  void begin(HardwareSerial& serial, int rxPin, int txPin);
  void recv();

  private:
  Stream* _stream;

  int _run_status; // Current running status or 0 if none
  int _data1; // Buffered first data byte or -1 if none
  int _data2; // Buffered second data byte or -1 if none

  int _msg_type; // Message type present in _msg (MIDI_MESSAGE_###)
  union {
    struct MIDIMessageNoteOff note_off;
    struct MIDIMessageNoteOn note_on;
    struct MIDIMessageNoteAftertouch note_aftertouch;
    struct MIDIMessageControlChange control_change;
    struct MIDIMessageProgramChange program_change;
    struct MIDIMessageChannelAftertouch channel_aftertouch;
    struct MIDIMessagePitchBend pitch_bend;
  } _msg;
};

MIDI::MIDI() {
  _stream = NULL;
  _run_status = 0;
  _msg_type = MIDI_MESSAGE_NONE;
}

void MIDI::begin(Stream* stream) {
  _stream = stream;
}

void MIDI::begin(HardwareSerial& serial) {
  serial.begin(31250);
  _stream = &serial;
}

void MIDI::begin(HardwareSerial& serial, int rxPin, int txPin) {
  serial.begin(31250, rxPin, txPin);
  _stream = &serial;
}

void MIDI::recv() {
  if(!_stream) return;

  int byte = _stream->read(); // Read new byte
  if(byte < 0) return;

  // System Real Time messages
  if(byte >= 0xF8 && byte <= 0xFE) return;
  if(byte == 0xFF) return; // System Reset

  int status = _run_status;
  int data = -1;
  if((byte & 0x80) > 0) { // Read a new status byte
    status = byte;
    _data1 = -1;
    _data2 = -1;

    // Check for next data byte
    byte = _stream->peek();
    if(byte >= 0 && byte <= 0x80) {
      _stream->read();
      data = byte;
    }
  }

  if((status & 0x80) == 0) return; // No current status, discard byte

  // Set _data1 or _data2 if a new data byte was read
  if(data >= 0 && _data1 < 0) _data1 = data;
  else if(data >= 0 && _data2 < 0) _data2 = data;

  // Set or unset currently active running status
  if(status >= 0x80 && status <= 0xEF) _run_status = status;
  else _run_status = 0;

  if(status == 0xF0) return; // Start of System Exclusive (SysEx) message
  if(status == 0xF7) return; // EOX (end of SysEx)

  if((status & 0xF0) == 0x80) { // Note off
    if(_data1 < 0 || _data2 < 0) return;

    _msg_type = MIDI_MESSAGE_NOTE_OFF;
  }
}