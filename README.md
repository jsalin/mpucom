# tracer
Serial port to MPU401 loop

(C) 2020 Jussi Salin under GNU GPL v2

For archiving purposes, I wanted to run games in DosBox while having the MIDI music come out of a real ISA wavetable sound card in an old PC. While there exists good programs for transferring MIDI data over normal serial ports or LAN, I could not find a good combination that would together from Windows 10 to either XP or 98. On the sending end I found that LoopMIDI and Hairless MIDI are good for getting the MIDI messages from DosBox to a real serial port, so I decided to write a simple DOS program for the receiving end myself. It forwards the MIDI messages from a real serial port to a normal MPU401 midi controller that is used on a variety of old wavetable sound cards.

To compile the program, simply open the C file in Borland Turbo C++ 3.0. Precompiled version and a sample video exist also in this repository.

By default, COM1 at 19200bps and MPU at 0x330 are used. Use same speed at the sending computer. A simple 3-wire null modem cable is enough - no need for handshaking. USB serial converters should also work with this. Speed can be increased if the cable is good enough.