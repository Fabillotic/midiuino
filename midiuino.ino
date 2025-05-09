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
  int velocity;
};

#define MIDI_MESSAGE_PITCH_BEND 7
struct MIDIMessagePitchBend {
  int channel;
  int value;
};

struct MIDIMessage {
  int type; // Message type present in data (MIDI_MESSAGE_###)
  union {
    struct MIDIMessageNoteOff note_off;
    struct MIDIMessageNoteOn note_on;
    struct MIDIMessageNoteAftertouch note_aftertouch;
    struct MIDIMessageControlChange control_change;
    struct MIDIMessageProgramChange program_change;
    struct MIDIMessageChannelAftertouch channel_aftertouch;
    struct MIDIMessagePitchBend pitch_bend;
  } data;
};

class MIDI {
  public:
  MIDI();
  void begin(Stream* stream);
  void begin(HardwareSerial& serial);
  void begin(HardwareSerial& serial, int rxPin, int txPin);
  int recv(MIDIMessage *msg);

  private:
  Stream* _stream;

  int _run_status; // Current running status or 0 if none
  int _data1; // Buffered first data byte or -1 if none
  int _data2; // Buffered second data byte or -1 if none
};

MIDI::MIDI() {
  _stream = NULL;
  _run_status = 0;
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

/* Receive MIDI message
 * Returns 1 on success, otherwise 0.
 */
int MIDI::recv(MIDIMessage *msg) {
  if(!_stream) return 0;

  int byte = _stream->read(); // Read new byte
  if(byte < 0) return 0;

  // System Real Time messages
  if(byte >= 0xF8 && byte <= 0xFE) return 0;
  if(byte == 0xFF) return 0; // System Reset

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

  if((status & 0x80) == 0) return 0; // No current status, discard byte

  // Set _data1 or _data2 if a new data byte was read
  if(data >= 0 && _data1 < 0) _data1 = data;
  else if(data >= 0 && _data2 < 0) _data2 = data;

  // Set or unset currently active running status
  if(status >= 0x80 && status <= 0xEF) _run_status = status;
  else _run_status = 0;

  if(status == 0xF0) return 0; // Start of System Exclusive (SysEx) message
  if(status == 0xF7) return 0; // EOX (end of SysEx)

  if((status & 0xF0) == 0x80) { // Note off
    if(_data1 < 0 || _data2 < 0) return 0;

    msg->type = MIDI_MESSAGE_NOTE_OFF;
    msg->data.note_off.channel = (status & 0xF) + 1;
    msg->data.note_off.note = _data1;
    msg->data.note_off.velocity = _data2;
  }
  else if((status & 0xF0) == 0x90) { // Note on
    if(_data1 < 0 || _data2 < 0) return 0;

    msg->type = MIDI_MESSAGE_NOTE_ON;
    msg->data.note_on.channel = (status & 0xF) + 1;
    msg->data.note_on.note = _data1;
    msg->data.note_on.velocity = _data2;
  }
  else if((status & 0xF0) == 0xA0) { // Note Aftertouch
    if(_data1 < 0 || _data2 < 0) return 0;

    msg->type = MIDI_MESSAGE_NOTE_AFTERTOUCH;
    msg->data.note_aftertouch.channel = (status & 0xF) + 1;
    msg->data.note_aftertouch.note = _data1;
    msg->data.note_aftertouch.velocity = _data2;
  }
  else if((status & 0xF0) == 0xB0) { // Control Change
    if(_data1 < 0 || _data2 < 0) return 0;

    msg->type = MIDI_MESSAGE_CONTROL_CHANGE;
    msg->data.control_change.channel = (status & 0xF) + 1;
    msg->data.control_change.control = _data1;
    msg->data.control_change.value = _data2;
  }
  else if((status & 0xF0) == 0xC0) { // Program Change
    if(_data1 < 0) return 0;

    msg->type = MIDI_MESSAGE_PROGRAM_CHANGE;
    msg->data.program_change.channel = (status & 0xF) + 1;
    msg->data.program_change.program = _data1;
  }
  else if((status & 0xF0) == 0xD0) { // Channel Aftertouch
    if(_data1 < 0) return 0;

    msg->type = MIDI_MESSAGE_CHANNEL_AFTERTOUCH;
    msg->data.channel_aftertouch.channel = (status & 0xF) + 1;
    msg->data.channel_aftertouch.velocity = _data1;
  }
  else if((status & 0xF0) == 0xE0) { // Pitch Bend
    if(_data1 < 0 || _data2 < 0) return 0;

    msg->type = MIDI_MESSAGE_PITCH_BEND;
    msg->data.pitch_bend.channel = (status & 0xF) + 1;
    msg->data.pitch_bend.value = (_data2 << 7) | _data1;
  }
  else {
    return 0;
  }

  return 1;
}