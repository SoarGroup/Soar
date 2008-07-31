/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003
 *     Brian Gerkey
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <libplayercore/playercore.h>
#include "audio_sample.h"

#include <stdlib.h>
#include <iostream>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//	Class functions
////////////////////////////////////////////////////////////////////////////////

// Constructor
AudioSample::AudioSample (void)
{
	// Set the sample type to none
	type = SAMPLE_TYPE_NONE;
	// Blank wave data
	numChannels = 0;
	sampleRate = 0;
	byteRate = 0;
	blockAlign = 0;
	bitsPerSample = 0;
	numFrames = 0;
	// Blank storage variables
	position = 0;
	waveFile = NULL;
	filePath = NULL;
	headerSize = 0;
	dataLength = 0;
	data = NULL;
}

// Constructor from Player type
AudioSample::AudioSample (const player_audio_wav_t *source)
{
	// Set the sample type to none
	type = SAMPLE_TYPE_NONE;
	// Blank wave data
	numChannels = 0;
	sampleRate = 0;
	byteRate = 0;
	blockAlign = 0;
	bitsPerSample = 0;
	numFrames = 0;
	// Blank storage variables
	position = 0;
	waveFile = NULL;
	filePath = NULL;
	headerSize = 0;
	dataLength = 0;
	data = NULL;

	if (!FromPlayer (source))
		PLAYER_ERROR ("unable to create audio sample from Player data");
}

// Constructor from raw data
// source: the data to copy from
// length: number of _bytes_ in the data (not frames)
// channels: number of channels in the data
// sr: sample rate
// bps: bits per sample (8, 16, etc)
AudioSample::AudioSample (const uint8_t *source, uint32_t length, uint16_t channels, uint32_t sr, uint16_t bps)
{
	// Set the sample type to memory
	type = SAMPLE_TYPE_MEM;
	// Set wave info
	numChannels = channels;
	sampleRate = sr;
	bitsPerSample = bps;
	// Calculate the other format info
	blockAlign = numChannels * (bitsPerSample / 8);
	byteRate = sampleRate * blockAlign;

	// Blank other storage variables
	position = 0;
	waveFile = NULL;
	filePath = NULL;
	headerSize = 0;
	dataLength = 0;
	data = NULL;

	// Allocate memory for the data
	if ((data = new uint8_t[length]) == NULL)
	{
		PLAYER_ERROR ("Failed to allocate memory for wave data");
		return;
	}
	// Copy the wave data across
	memcpy (data, source, length);
	dataLength = length;
	numFrames = dataLength / blockAlign;
}

// Destructor
AudioSample::~AudioSample (void)
{
	if (waveFile != NULL)
	{
		if (type != SAMPLE_TYPE_FILE)
			PLAYER_WARN ("file descriptor not NULL for non-file sample");
		fclose (waveFile);
	}
	if (filePath != NULL)
	{
		if (type != SAMPLE_TYPE_FILE)
			PLAYER_WARN ("file path not NULL for non-file sample");
		free (filePath);
	}
	if (data != NULL)
	{
		if (type != SAMPLE_TYPE_MEM)
			PLAYER_WARN ("data not NULL for non-mem sample");
		delete[] data;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Data management functions
////////////////////////////////////////////////////////////////////////////////

// Set the position of reading in the data
void AudioSample::SetDataPosition (uint32_t newPosition)
{
	// If the new position is beyond the end of the data, set it to the end
	if ((newPosition * blockAlign) > dataLength)
		position = dataLength;
	else
		// Otherwise just set it to the new position
		position = newPosition * blockAlign;
}

uint32_t AudioSample::GetDataPosition (void) const
{
	return position / blockAlign;
}

uint32_t AudioSample::GetDataLength (void) const
{
	if (type == SAMPLE_TYPE_FILE || type == SAMPLE_TYPE_MEM)
		return dataLength / blockAlign;
	else	// SAMPLE_TYPE_NONE or some other value that type really shouldn't be
		return 0;
}

// Get a block of wave data
// frameCount: The number of _frames_ to get (not bytes!)
// buffer: The buffer to store the frames in (must allocate enough)
// Returns: the number of frames actually stored in buffer
int AudioSample::GetData (int frameCount, uint8_t *buffer)
{
	int bytesCopied = 0;

	if (buffer == NULL)
	{
		// Can't copy to a NULL buffer
		PLAYER_WARN ("Can't copy data into a null buffer");
		return -1;
	}

	// Number of bytes to copy is number of frames to copy * frame size
	int bytesCount = frameCount * blockAlign;

	if (type == SAMPLE_TYPE_NONE)
	{
		PLAYER_WARN ("Attempt to get data from an empty sample");
		return -1;
	}
	else if (type == SAMPLE_TYPE_FILE)
	{
		// Seek to the position to get data from in the file (offset by header size)
		// (Have to do this in case SetDataPosition was used since the last read)
		if (fseek (waveFile, headerSize + position, SEEK_SET) != 0)
		{
			PLAYER_ERROR1 ("Error seeking to current position in wave file: %s", strerror (errno));
			return -1;
		}
		// Number of bytes to copy shouldn't take us beyond the end of the data
		int bytesToCopy = (position + bytesCount) > dataLength ? (dataLength - position) : bytesCount;
		// Read into the buffer provided the number of bytes to copy
		if ((bytesCopied = fread (buffer, 1, bytesToCopy, waveFile)) == 0)
		{
			if (feof (waveFile))
				PLAYER_ERROR ("End of file reading wave data");
			else
				PLAYER_ERROR1 ("Error reading wave data from file: %s", strerror (errno));
			return 0;
		}
		else if (bytesCopied < bytesToCopy)
		{
			PLAYER_ERROR1 ("Error reading wave data from file (didn't get enough bytes): %s\n", strerror (errno));
			// Return what we got, driver will assume end of data and move to next sample in queue
		}
	}
	else if (type == SAMPLE_TYPE_MEM)
	{
		// If there is no data, return 0 (end of sample)
		if (dataLength == 0)
			return 0;
		// Number of bytes to copy shouldn't take us beyond the end of the array
		int bytesToCopy = (position + bytesCount) > dataLength ? (dataLength - position) : bytesCount;
		// Copy from data[position] to data[position + bytesToCopy]
		memcpy (buffer, &data[position], bytesToCopy);
		bytesCopied = bytesToCopy;
	}

	// Update position with the new position (old position + length actually read)
	position += bytesCopied;
	// Return the number of frames actually copied
	return bytesCopied / blockAlign;
}

// Clears the entire sample, making it a SAMPLE_TYPE_NONE
void AudioSample::ClearSample (void)
{
	// For FILE type, need to close the file and remove the file name string
	if (type == SAMPLE_TYPE_FILE)
	{
		if (waveFile != NULL)
			fclose (waveFile);
		if (filePath != NULL)
			free (filePath);
	}
	// For MEM type, need to delete any stored data
	else if (type == SAMPLE_TYPE_MEM)
	{
		if (data != NULL)
			delete[] data;
	}
	// Do nothing for SAMPLE_TYPE_NONE

	// In both cases, set everything to 0
	// Set the sample type to none
	type = SAMPLE_TYPE_NONE;
	// Blank wave data
	numChannels = 0;
	sampleRate = 0;
	byteRate = 0;
	blockAlign = 0;
	bitsPerSample = 0;
	numFrames = 0;
	// Blank storage variables
	position = 0;
	waveFile = NULL;
	filePath = NULL;
	headerSize = 0;
	dataLength = 0;
	data = NULL;
}

// Fills the data with a constant silence value
// time: The length of time to fill
bool AudioSample::FillSilence (uint32_t time)
{
	// First, make sure this sample is empty
	if (type != SAMPLE_TYPE_NONE)
	{
		PLAYER_ERROR ("Tried to set non-empty sample to silence");
		return false;
	}

	// Next, create a data buffer
	dataLength = static_cast<uint32_t> ((static_cast<double> (time) / 1000.0f) * byteRate);
	if ((data = new uint8_t[dataLength]) == NULL)
	{
		PLAYER_ERROR ("Failed to allocate memory for wave data");
		return false;
	}
	// Set the wave data to zero
	memset (data, 0, dataLength);
	numFrames = dataLength / blockAlign;

	// Set type to MEM
	type = SAMPLE_TYPE_MEM;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	Data conversion functions
////////////////////////////////////////////////////////////////////////////////

// Copies the data from a loaded wave file to the player struct for wave data
// If the wave data doesn't match one of the possible formats supported by the
// player format flags, the copy isn't performed and an error is returned
bool AudioSample::ToPlayer (player_audio_wav_t *dest)
{


	if (type == SAMPLE_TYPE_NONE || (type == SAMPLE_TYPE_MEM && data == NULL) || (type == SAMPLE_TYPE_FILE && filePath == NULL))
	{
		PLAYER_WARN ("No sample to convert to player format");
		return false;
	}

	// Set the format flags
	dest->format = PLAYER_AUDIO_FORMAT_RAW;
	if (numChannels == 2)
		dest->format |= PLAYER_AUDIO_STEREO;
	else if (numChannels != 1)
	{
		PLAYER_ERROR ("Cannot convert wave to player struct: wrong number of channels");
		return false;
	}
	switch (sampleRate)
	{
		case 11025:
			dest->format |= PLAYER_AUDIO_FREQ_11k;
			break;
		case 22050:
			dest->format |= PLAYER_AUDIO_FREQ_22k;
			break;
		case 44100:
			dest->format |= PLAYER_AUDIO_FREQ_44k;
			break;
		case 48000:
			dest->format |= PLAYER_AUDIO_FREQ_48k;
			break;
		default:
			PLAYER_ERROR ("Cannot convert wave to player struct: wrong sample rate");
			return false;
	}

	switch (bitsPerSample)
	{
		case 8:
			dest->format |= PLAYER_AUDIO_8BIT;
			break;
		case 16:
			dest->format |= PLAYER_AUDIO_16BIT;
			break;
		case 24:
			dest->format |= PLAYER_AUDIO_24BIT;
			break;
		default:
			PLAYER_ERROR ("Cannot convert wave to player struct: wrong format (bits per sample)");
			return false;
	}

	// Copy as many frames as can fit into PLAYER_MAX_MESSAGE_SIZE bytes
	uint32_t framesToCopy = PLAYER_MAX_MESSAGE_SIZE / blockAlign;
	if (numFrames < framesToCopy)
		framesToCopy = numFrames;
	if (type == SAMPLE_TYPE_FILE)
	{
		// Remember the current data position
		uint32_t currentPos = GetDataPosition ();
		// Move to the start of the wave
		SetDataPosition (0);
		// Grab some data, put it in the player struct
		printf ("copied %d frames\n", GetData (framesToCopy, dest->data));
		// Move back to where we were
		SetDataPosition (currentPos);
	}
	else
	{
		// Just copy. Nice and easy.
		memcpy (&dest->data, data, framesToCopy * blockAlign);
	}
	dest->data_count = framesToCopy * blockAlign;

	return true;
}

bool AudioSample::FromPlayer (const player_audio_wav_t *source)
{
	// Set format information
	if ((source->format & PLAYER_AUDIO_FORMAT_BITS) != PLAYER_AUDIO_FORMAT_RAW)
	{
		// Can't handle non-raw data
		PLAYER_ERROR ("Cannot play non-raw audio data");
		return false;
	}

	// Clean out any existing data
	ClearSample ();

	if (source->format & PLAYER_AUDIO_STEREO)
		numChannels = 2;
	else
		numChannels = 1;

	if ((source->format & PLAYER_AUDIO_FREQ) == PLAYER_AUDIO_FREQ_11k)
		sampleRate = 11025;
	else if ((source->format & PLAYER_AUDIO_FREQ) == PLAYER_AUDIO_FREQ_22k)
		sampleRate = 22050;
	else if ((source->format & PLAYER_AUDIO_FREQ) == PLAYER_AUDIO_FREQ_44k)
		sampleRate = 44100;
	else if ((source->format & PLAYER_AUDIO_FREQ) == PLAYER_AUDIO_FREQ_48k)
		sampleRate = 48000;

	if ((source->format & PLAYER_AUDIO_BITS) == PLAYER_AUDIO_8BIT)
	{
		bitsPerSample = 8;
	}
	else if ((source->format & PLAYER_AUDIO_BITS) == PLAYER_AUDIO_16BIT)
	{
		bitsPerSample = 16;
	}
	else if ((source->format & PLAYER_AUDIO_BITS) == PLAYER_AUDIO_24BIT)
	{
		bitsPerSample = 24;
	}

	// Calculate the other format info
	blockAlign = numChannels * (bitsPerSample / 8);
	byteRate = sampleRate * blockAlign;

	// Allocate memory for the data if necessary
	if ((data = new uint8_t[source->data_count]) == NULL)
	{
		PLAYER_ERROR ("Failed to allocate memory for wave data");
		return false;
	}
	// Copy the wave data across
	memcpy (data, source->data, source->data_count);
	dataLength = source->data_count;
	numFrames = dataLength / blockAlign;

	// Set type to MEM
	type = SAMPLE_TYPE_MEM;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	File management functions
////////////////////////////////////////////////////////////////////////////////

// Load a wave file from disc
// filePath: The path to the file to load
// Returns: true for success, false otherwise
bool AudioSample::LoadFile (const char *fileName)
{
	char tag[5];
	uint16_t tempUShort = 0;
	uint32_t tempUInt = 0, subChunk1Size = 0;

	// First of all, make sure this sample is empty
	ClearSample ();

	// Try to open the wave file
	if ((waveFile = fopen (fileName, "r")) == NULL)
	{
		PLAYER_ERROR1 ("Couldn't open wave file for reading: %s", strerror (errno));
		return false;
	}

	// Wave file should be in the format (header, format chunk, data chunk), where:
	// header = 4+4+4 bytes: "RIFF", size, "WAVE"
	// format = 4+4+2+2+4+4+2+2[+2] bytes:
	//          "fmt ", size, 1, numChannels, sampleRate, byteRate, blockAlign,
	//          bitsPerSample, [extraParamsSize] (not present for PCM)
	// data = 4+4+? bytes: "data", size, data bytes...

	// Read the header - first the RIFF tag
	if (fgets (tag, 5, waveFile) == NULL)
	{
		PLAYER_ERROR ("Error reading tag from wave file");
		return false;
	}
	if (strcmp (tag, "RIFF") != 0)
	{
		PLAYER_ERROR ("Bad WAV format: missing RIFF tag");
		return false;
	}
	// Get the size of the file
	if (fread (&tempUInt, 4, 1, waveFile) != 1)
	{
		if (feof (waveFile))
			PLAYER_ERROR ("End of file reading WAV header");
		else
			PLAYER_ERROR ("Error reading WAV header");
		return false;
	}
	// Check the file size isn't stupid - should at least have size of all
	// chunks (excluding 2 fields already done)
	if (tempUInt < 36)
	{
		PLAYER_ERROR ("WAV file too short: missing chunk information");
		return false;
	}
	// Next tag should say "WAVE"
	if (fgets (tag, 5, waveFile) == NULL)
	{
		PLAYER_ERROR ("Error reading tag from wave file");
		return false;
	}
	if (strcmp (tag, "WAVE") != 0)
	{
		PLAYER_ERROR ("Bad WAV format: missing WAVE tag");
		return false;
	}

	// Next is the format information chunk, starting with a "fmt " tag
	if (fgets (tag, 5, waveFile) == NULL)
	{
		PLAYER_ERROR ("Error reading tag from wave file");
		return false;
	}
	if (strcmp (tag, "fmt ") != 0)
	{
		PLAYER_ERROR ("Bad WAV format: missing fmt  tag");
		return false;
	}
	// Followed by size of this chunk - should be 16, may be 18 if not quite
	// following the format correctly
	if (fread (&subChunk1Size, 4, 1, waveFile) != 1)
	{
		if (feof (waveFile))
			PLAYER_ERROR ("End of file reading WAV format");
		else
			PLAYER_ERROR ("Error reading WAV format");
		return false;
	}
	if (subChunk1Size != 16 && subChunk1Size != 18)
	{
		PLAYER_ERROR ("WAV file too short: missing chunk information");
		return false;
	}
	// Audio format is next - if not 1, can't read this file cause it isn't PCM
	if (fread (&tempUShort, 2, 1, waveFile) != 1)
	{
		if (feof (waveFile))
			PLAYER_ERROR ("End of file reading WAV format");
		else
			PLAYER_ERROR ("Error reading WAV format");
		return false;
	}
	if (tempUShort != 1)
	{
		PLAYER_ERROR ("WAV file not in PCM format");
		return false;
	}
	// Having got this far, we can now start reading data we want to keep
	// Read the number of channels
	if (fread (&numChannels, 2, 1, waveFile) != 1)
	{
		if (feof (waveFile))
			PLAYER_ERROR ("End of file reading WAV num channels");
		else
			PLAYER_ERROR ("Error reading WAV num channels");
		ClearSample ();
		return false;
	}
	// Read the sample rate
	if (fread (&sampleRate, 4, 1, waveFile) != 1)
	{
		if (feof (waveFile))
			PLAYER_ERROR ("End of file reading WAV sample rate");
		else
			PLAYER_ERROR ("Error reading WAV sample rate");
		ClearSample ();
		return false;
	}
	// Read the byte rate
	if (fread (&byteRate, 4, 1, waveFile) != 1)
	{
		if (feof (waveFile))
			PLAYER_ERROR ("End of file reading WAV byte rate");
		else
			PLAYER_ERROR ("Error reading WAV byte rate");
		ClearSample ();
		return false;
	}
	// Read the block align
	if (fread (&blockAlign, 2, 1, waveFile) != 1)
	{
		if (feof (waveFile))
			PLAYER_ERROR ("End of file reading WAV block align");
		else
			PLAYER_ERROR ("Error reading WAV block align");
		ClearSample ();
		return false;
	}
	// Read the bits per sample
	if (fread (&bitsPerSample, 2, 1, waveFile) != 1)
	{
		if (feof (waveFile))
			PLAYER_ERROR ("End of file reading WAV bits per sample");
		else
			PLAYER_ERROR ("Error reading WAV bits per sample");
		ClearSample ();
		return false;
	}
	// If the size of this chunk was 18, get those extra two bytes
	if (subChunk1Size == 18)
	{
		if (fread (&tempUShort, 2, 1, waveFile) != 1)
		{
			if (feof (waveFile))
				PLAYER_ERROR ("End of file reading blank 2 bytes");
			else
				PLAYER_ERROR ("Error reading WAV blank 2 bytes");
			ClearSample ();
			return false;
		}
	}

	// On to the data chunk, again starting with a tag
	if (fgets (tag, 5, waveFile) == NULL)
	{
		PLAYER_ERROR ("Error reading tag from wave file");
		ClearSample ();
		return false;
	}
	if (strcmp (tag, "data") != 0)
	{
		PLAYER_ERROR ("Bad WAV format: missing data tag");
		ClearSample ();
		return false;
	}
	// Size of the wave data
	if (fread (&dataLength, 4, 1, waveFile) != 1)
	{
		if (feof (waveFile))
			PLAYER_ERROR ("End of file reading WAV data size");
		else
			PLAYER_ERROR ("Error reading WAV data size");
		ClearSample ();
		return false;
	}

	// The file pointer is now positioned at the start of the data.
	// Store the header size (for moving around the wave file easily later on).
	headerSize = ftell (waveFile);
	// Also store the file path
	filePath = strdup (fileName);
	// Go to the end of the file and get position in order to get the length
	// of the file, subtract headerSize to get the position
	fseek (waveFile, 0, SEEK_END);
	dataLength = ftell (waveFile) - headerSize;
	// Back to the start of the data again
	fseek (waveFile, headerSize, SEEK_SET);
	// Calculate the number of frames in the data
	numFrames = dataLength / (numChannels * (bitsPerSample / 8));
	// Set type
	type = SAMPLE_TYPE_FILE;

	return true;
}

void AudioSample::CloseFile (void)
{
	// Just clear the sample, that'll close the file and clean up anything else
	ClearSample ();
}

////////////////////////////////////////////////////////////////////////////////
//	Wave format functions
////////////////////////////////////////////////////////////////////////////////

// Checks if the format information of this sample is the same as rhs
// rhs: A pointer to an AudioSample
// Returns: true if same, false if different or rhs is NULL
bool AudioSample::SameFormat (const AudioSample *rhs)
{
	if (!rhs || type == SAMPLE_TYPE_NONE)
		return false;
	if (numChannels == rhs->GetNumChannels () &&
		   sampleRate == rhs->GetSampleRate () &&
		   bitsPerSample == rhs->GetBitsPerSample ())
		return true;
	return false;
}

// Copies the format of rhs
// rhs: A pointer to an AudioSample whose format should be copied
void AudioSample::CopyFormat (const AudioSample *rhs)
{
	if (!rhs || type == SAMPLE_TYPE_NONE)
		return;
	numChannels = rhs->GetNumChannels ();
	sampleRate = rhs->GetSampleRate ();
	byteRate = rhs->GetByteRate ();
	blockAlign = rhs->GetBlockAlign ();
	bitsPerSample = rhs->GetBitsPerSample ();
}

////////////////////////////////////////////////////////////////////////////////
//	Other useful functions
////////////////////////////////////////////////////////////////////////////////

void AudioSample::PrintWaveInfo (void)
{
	if (type == SAMPLE_TYPE_FILE)
		cout << "File sample, path: " << filePath << endl;
	else if (type == SAMPLE_TYPE_MEM)
		cout << "Memory sample" << endl;
	else
		cout << "Empty sample" << endl;
	cout << "Num channels:\t" << numChannels << endl;
	cout << "Sample rate:\t" << sampleRate << endl;
	cout << "Byte rate:\t" << byteRate << endl;
	cout << "Block align:\t" << blockAlign << endl;
	cout << "Format:\t\t" << endl;
	switch (bitsPerSample)
	{
		case 8:
			cout << "Unsigned 8 bit" << endl;
			break;
		case 16:
			cout << "Signed 16 bit little-endian" << endl;
			break;
		case 24:
			if ((blockAlign / numChannels) == 3)
				cout << "Signed 24 bit 3-byte little-endian" << endl;
			else
				cout << "Signed 24 bit little-endian" << endl;
			break;
		case 32:
			cout << "Signed 32 bit little-endian" << endl;
			break;
		default:
			cout << "Unplayable format: " << bitsPerSample << " bit" << endl;
	}
	cout << "Num frames:\t" << numFrames << endl;
	cout << "Data length:\t" << dataLength << endl;
	if (type == SAMPLE_TYPE_FILE)
		cout << "Frames start at:\t" << headerSize << endl;
}
