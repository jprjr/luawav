# luawav

A Lua library for reading and writing WAV files,
via [dr_wav](https://github.com/mackron/dr_libs).

MIT licensed (see file `LICENSE`), except for
`dr_wav.h`, which retains its original licensing (details
in `dr_wav.h`).

# Installation

## Building from source

You can build with luarocks or cmake.

# Table of Conents

* [Synopsis](#synopsis)
* [Implementation Notes](#implementation-notes)
* [Constants](#constants)
* [Callbacks](#callbacks)
  * [onRead](#onread)
  * [onSeek](#onseek)
  * [onWrite](#onwrite)
  * [onChunk](#onchunk)
* [Functions](#functions)
  * [drwav](#drwav)
  * [drwav\_init](#drwav_init)
  * [drwav\_init\_write](#drwav_init_write)
  * [drwav\_read\_pcm\_frames\_f32](#drwav_read_pcm_frames_f32)
  * [drwav\_read\_pcm\_frames\_s32](#drwav_read_pcm_frames_s32)
  * [drwav\_read\_pcm\_frames\_s16](#drwav_read_pcm_frames_s16)
  * [drwav\_open\_and\_read\_pcm\_frames\_f32](#drwav_open_and_read_pcm_frames_f32)
  * [drwav\_open\_and\_read\_pcm\_frames\_s32](#drwav_open_and_read_pcm_frames_s32)
  * [drwav\_open\_and\_read\_pcm\_frames\_s16](#drwav_open_and_read_pcm_frames_s16)
  * [drwav\_write\_pcm\_frames](#drwav_write_pcm_frames)
  * [drwav\_uninit](#drwav_uninit)
  * [drwav\_version](#drwav_version)
  * [drwav\_version\_string](#drwav_version_string)

# Synopsis

```lua
local wav = require'luawav'

-- allocate a drwav instance and initialize it for
-- reading a wav file

local reader = wav.drwav()
local info = reader:init('some-file.wav')

local chunk
repeat
  chunk = reader:read_pcm_frames_s16(2048)
  -- chunk is an array-like table of interleaved samples
until #chunk / info.channels ~= 2048
```


# Implementation Notes

Some function/fields use an unsigned 64-bit integer value.
These are represented via a userdata object, with a metatable
for comparison, addition, converting to a string, etc.

# Constants

All constants and enums from `dr_wav` are available, notable
constants/enums are:

* `DR_WAVE_FORMAT_PCM`
* `DR_WAVE_FORMAT_ADPCM`
* `DR_WAVE_FORMAT_IEEE_FLOAT`
* `DR_WAVE_FORMAT_ALAW`
* `DR_WAVE_FORMAT_MULAW`
* `DR_WAVE_FORMAT_DVI_ADPCM`
* `DR_WAVE_FORMAT_EXTENSIBLE`
* `DRWAV_SEQUENTIAL`
* `drwav_container_riff`
* `drwav_container_w64`
* `drwav_container_rf64`

# Callbacks

Some functions take callbacks, here are the signatures and expected return values:

## onRead

**signature:** `string data = onRead(userData, integer bytesToRead)`

Should return a string with length `bytesToRead`. Anything less, and the stream
is considered to be ended.

## onSeek

**signature:** `boolean success = onSeek(userData, string whence, integer offset)`

`whence` is a string value, either `set`, or `cur`. If `set`,
then `offset` is an absolute position, if `cur`, then it's
relative to the current position.

Should return something truthy on success.

## onWrite

**signature:** `number bytes = onWrite(userData, string data)`

Called to write out data, should return the number of bytes written.

## onChunk

**signature**: `number bytes = onChunk(chunkUserData, table fileOps, table chunkHeader, table format)`

Called when a new chunk is encountered.

`fileOps` is a table for performing reads and seeks. `chunkHeader` has information about
the current chunk header, `format` containers information about the WAV file as a whole.

Should return the number of bytes read + number of bytes seeked.

The `fileOps` table will have the following keys:

| Key | Description |
|-----|-------------|
| onRead | The onRead callback |
| onSeek | The onSeek callback |
| userData | The onRead/onSeek userdata |

The `chunkHeader` table will have the following keys:

| Key | Description |
|-----|-------------|
| sizeInBytes | The size of the chunk, in bytes (uint64 value) |
| paddingSize | The amount of padding |
| fourcc | The FOURCC of the chunk (if the container is riff or rf64) |
| guid | the GUID of the chunk (if the container is w64) |

The `format` table will have the following keys:

| Key | Description |
|-----|-------------|
| channels | number of audio channels |
| sampleRate | sample rate in Hz |
| bitsPerSample | number of bits in an audio sample |
| extendedSize | The size of the WAV extended data |
| formatTag | The format tag |
| avgBytesPerSec | Averages bytes per second |
| blockAlign | Block alignment |
| validBitsPerSample | The number of valid bits per sample |
| channelMask | The channel mask |
| subFormat | The sub-format, as specified by the WAV file |

# Functions

## drwav

**syntax:** userdata state = wav.drwav()

Allocated a new `drwav` object, for reading or writing wav files.

The object has a metatable set, allowing for object-oriented
usage.

## drwav_init

**syntax:** `table info = wav.drwav_init(userdata state, string filename | table params)`

Initializes a drwav object for reading. The second parameter can
be either a string (representing a filename), or a table
of parameters for callback-based reading.

Returns a table with the opened WAV file's format information,
all values are integers, except for `subFormat`.

Here are the keys for the returned table:

| Key | Description |
|-----|-------------|
| channels | number of audio channels |
| sampleRate | sample rate in Hz |
| bitsPerSample | number of bits in an audio sample |
| extendedSize | The size of the WAV extended data |
| formatTag | The format tag |
| avgBytesPerSec | Averages bytes per second |
| blockAlign | Block alignment |
| validBitsPerSample | The number of valid bits per sample |
| channelMask | The channel mask |
| subFormat | The sub-format, as specified by the WAV file |

Here are the keys for the `params` table (to use custom read callbacks):

| Key | Description |
|-----|-------------|
| onRead | onRead callback |
| onSeek | onSeek callback |
| userData | Data to pass to onRead and onSeek callbacks |
| onChunk | onChunk callback |
| chunkUserData | Data to pass to onChunk callbacks |
| flags | additional flags to pass |

The `flags` parameter only applies if you specify an `onChunk` callback, it controls
whether the file supports seeking or not.

## drwav_init_write

**syntax:** `boolean success = wav.drwav_init_write(userdata state, string filename | table params, table format )`

Initializes a drwav object for writing. The second parameter
can be a string (representing a filename), or a table of parameters
for callback-based writing. The third parameter specifies formatting
information.

Returns a boolean indicating success or failure.

If you're using callbacks, the table should have the following keys:

| Key | Description |
|-----|-------------|
| onWrite | callback when data needs to be written |
| onSeek  | callback when file position needs to be seeked |
| userdata | userdata for onWrite and onSeek |
| totalSamples | integer representing total audio samples, if known |
| totalFrames | integer representing total audio frames, if known |

Setting `totalSamples` or `totalFrames` will put the output into a sequential-only
writing mode (it won't need `onSeek`, because it won't need to seek).

The format table should have the following keys:

| Key | Description |
|-----|-------------|
| container | One of the `drwav_container_` enums. |
| format | One of the `DR_WAVE_FORMAT_*` constants. |
| channels | The number of audio channels |
| sampleRate | The sample rate in Hz |
| bitsPerSample | The number of bits in a sample |

## drwav_read_pcm_frames_f32

**syntax:** `table samples = wav.drwav_read_pcm_frames_f32(userdata state, number framesToRead)`

Reads the requested number of audio frames, and returns a table of float values between `-1.0` and
`1.0`. Table is an array-like table, single dimension, samples are interleaved.

## drwav_read_pcm_frames_s32

**syntax:** `table samples = wav.drwav_read_pcm_frames_s32(userdata state, number framesToRead)`

Reads the requested number of audio frames, and returns a table of integer values
in the signed, 32-bit range.
Table is an array-like table, single dimension, samples are interleaved.

## drwav_read_pcm_frames_s16

**syntax:** `table samples = wav.drwav_read_pcm_frames_s16(userdata state, number framesToRead)`

Reads the requested number of audio frames, and returns a table of integer values
in the signed, 16-bit range.
Table is an array-like table, single dimension, samples are interleaved.

## drwav_open_and_read_pcm_frames_f32

**syntax:** `table meta_and_samples = wav.dr_wav_open_and_read_pcm_frames_f32(string filename | table params)`

Opens a file and reads all the audio samples as floating-point values in one shot, returns a table
on success.

Can be given either a string representing a filename, or a table of parameters
with `onRead`, `onSeek`, and `userData` callbacks.

The returned table has the following keys:

| Key | Description |
|-----|-------------|
| channels | number of channels |
| sampleRate | sample rate in Hz |
| frameCount | total number of audio frames |
| samples | an array-like table of samples, interleaved, integers |

## drwav_open_and_read_pcm_frames_s32

**syntax:** `table meta_and_samples = wav.dr_wav_open_and_read_pcm_frames_s32(string filename | table params)`

Opens a file and reads all the audio samples as 32-bit signed integers in one shot, returns a table
on success.

Can be given either a string representing a filename, or a table of parameters
with `onRead`, `onSeek`, and `userData` callbacks.

The returned table has the following keys:

| Key | Description |
|-----|-------------|
| channels | number of channels |
| sampleRate | sample rate in Hz |
| frameCount | total number of audio frames |
| samples | an array-like table of samples, interleaved, integers |

## drwav_open_and_read_pcm_frames_s16

**syntax:** `table meta_and_samples = wav.dr_wav_open_and_read_pcm_frames_s16(string filename | table params)`

Opens a file and reads all the audio samples as 16-bit signed integers, in one shot, returns a table
on success.

Can be given either a string representing a filename, or a table of parameters
with `onRead`, `onSeek`, and `userData` callbacks.

The returned table has the following keys:

| Key | Description |
|-----|-------------|
| channels | number of channels |
| sampleRate | sample rate in Hz |
| frameCount | total number of audio frames |
| samples | an array-like table of samples, interleaved, integers |


## drwav_write_pcm_frames

**syntax:** `boolean success = wav.drwav_write_pcm_frames(userdata state, table samples)`

Writes the given table of audio samples, returns success or not.

Table should be array-like, interleaved samples.

## drwav_uninit

**syntax:** `wav.drwav_uninit(userdata state)`

This closes out a drwav object. It's set as the `__gc` value on the metatable, so
it will be called whenever the object is garbage-collected.

## drwav_version

**syntax:** `table info = wav.drwav_version()`

Returns a table with the `major`, `minor`, and `revision` values for the underlying
dr_wav library.

## drwav_version_string

Returns the dr_wav version as a string.
