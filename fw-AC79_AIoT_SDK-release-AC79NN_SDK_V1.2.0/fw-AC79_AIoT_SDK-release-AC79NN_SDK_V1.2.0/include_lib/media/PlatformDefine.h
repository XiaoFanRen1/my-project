#ifndef PLATFORM_DEFINE_H
#define PLATFORM_DEFINE_H

#if 0//def WIN32
#define AT_MIDI(x)
#define AT_MIDI_CODE
#define AT_MIDI_CONST
#define AT_MIDI_SPARSE_CODE
#define AT_MIDI_SPARSE_CONST
#else
#define AT_MIDI(x)            __attribute__((section(#x)))
#define AT_MIDI_CODE		  AT_MIDI(.midi.text.cache.L2.code)
#define AT_MIDI_CONST		  AT_MIDI(.midi.text.cache.L2.const)
#define AT_MIDI_SPARSE_CODE	  AT_MIDI(.midi.text)
#define AT_MIDI_SPARSE_CONST  AT_MIDI(.midi.text.const)
#endif

#endif
