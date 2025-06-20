/* midiuino - MIDI library for Arduino
 *
 * MIT License
 *
 * Copyright (C) 2025 Fabillotic
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

typedef void(*midi_callback_note_off_t)(int, int, int);        // channel, note, velocity
typedef void(*midi_callback_note_on_t)(int, int, int);         // channel, note, velocity
typedef void(*midi_callback_note_aftertouch_t)(int, int, int); // channel, note, velocity
typedef void(*midi_callback_control_change_t)(int, int, int);  // channel, control, value
typedef void(*midi_callback_program_change_t)(int, int);       // channel, program
typedef void(*midi_callback_channel_aftertouch_t)(int, int);   // channel, velocity
typedef void(*midi_callback_pitch_bend_t)(int, int);           // channel, value

struct MIDICallbacks {
  midi_callback_note_off_t note_off;
  midi_callback_note_on_t note_on;
  midi_callback_note_aftertouch_t note_aftertouch;
  midi_callback_control_change_t control_change;
  midi_callback_program_change_t program_change;
  midi_callback_channel_aftertouch_t channel_aftertouch;
  midi_callback_pitch_bend_t pitch_bend;
};

class MIDIReceiver {
  public:
  MIDIReceiver();
  void begin(Stream &stream);
  int recv(MIDIMessage *msg);
  void recv();
  void register_note_off(midi_callback_note_off_t callback);
  void register_note_on(midi_callback_note_on_t callback);
  void register_note_aftertouch(midi_callback_note_aftertouch_t callback);
  void register_control_change(midi_callback_control_change_t callback);
  void register_program_change(midi_callback_program_change_t callback);
  void register_channel_aftertouch(midi_callback_channel_aftertouch_t callback);
  void register_pitch_bend(midi_callback_pitch_bend_t callback);

  private:
  Stream* _stream;
  MIDICallbacks _callbacks;
  int _status; // Last read status or 0 if not set
  int _data_buf; // Single byte buffer for last read data byte or -1 if not set
};

#endif
