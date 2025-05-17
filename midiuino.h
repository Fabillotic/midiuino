#ifndef MIDIUINO_H
#define MIDIUINO_H

#define MIDI_BAUD 31250

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

class MIDIReceiver {
  public:
  MIDIReceiver();
  void begin(Stream &stream);
  int recv(MIDIMessage *msg);

  private:
  Stream* _stream;

  int _status; // Most recently read status or 0 if none
  int _data1; // Buffered first data byte or -1 if none
  int _data2; // Buffered second data byte or -1 if none
};

#endif