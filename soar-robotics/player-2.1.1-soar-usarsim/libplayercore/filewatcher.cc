/*
 * FileWatcher.cc
 *
 *  Created on: 10/06/2008
 *      Author: tcollett
 */

#include <libplayercore/filewatcher.h>
#include <libplayercore/message.h>
#include <libplayercore/error.h>
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <error.h>
#include <math.h>
#include <string.h>

FileWatcher::FileWatcher()
{
	WatchedFilesArraySize = INITIAL_WATCHED_FILES_ARRAY_SIZE;
	WatchedFilesArrayCount = 0;
	WatchedFiles = reinterpret_cast<struct fd_driver_pair *> (calloc(WatchedFilesArraySize,sizeof(WatchedFiles[0])));
	assert(WatchedFiles);
	pthread_mutex_init(&this->lock,NULL);

}

FileWatcher::~FileWatcher()
{
	free(WatchedFiles);
}

void FileWatcher::Lock()
{
  pthread_mutex_lock(&lock);
}

void FileWatcher::Unlock()
{
  pthread_mutex_unlock(&lock);
}


int FileWatcher::Wait(double Timeout)
{
	Lock();
	if (WatchedFilesArrayCount == 0)
	{
		Unlock();
		return 0;
	}

	// intialise our FD sets for the select call
	fd_set ReadFds,WriteFds,ExceptFds;
	FD_ZERO(&ReadFds);
	FD_ZERO(&WriteFds);
	FD_ZERO(&ExceptFds);

	int maxfd = 0;

	for (unsigned int ii = 0; ii < WatchedFilesArrayCount; ++ii)
	{
		if (WatchedFiles[ii].fd >= 0)
		{
			if (WatchedFiles[ii].fd > maxfd)
				maxfd = WatchedFiles[ii].fd;
			if (WatchedFiles[ii].Read)
				FD_SET(WatchedFiles[ii].fd,&ReadFds);
			if (WatchedFiles[ii].Write)
				FD_SET(WatchedFiles[ii].fd,&WriteFds);
			if (WatchedFiles[ii].Except)
				FD_SET(WatchedFiles[ii].fd,&ExceptFds);
		}
	}

	struct timeval t;
	t.tv_sec = static_cast<int> (floor(Timeout));
	t.tv_usec = static_cast<int> ((Timeout - static_cast<int> (floor(Timeout))) * 1e6);
	Unlock();

	int ret = select (maxfd+1,&ReadFds,&WriteFds,&ExceptFds,&t);

	if (ret < 0)
	{
		PLAYER_ERROR2("Select called failed in File Watcher: %d %s",errno,strerror(errno));
		return ret;
	}

	int queueless_count = 0;

	Lock();
	for (unsigned int ii = 0; ii < WatchedFilesArrayCount && static_cast<int> (ii) < maxfd; ++ii)
	{
		int fd = WatchedFiles[ii].fd;
		QueuePointer &q = WatchedFiles[ii].queue;
		if (fd > 0 && fd <= maxfd)
		{
			if ((WatchedFiles[ii].Read && FD_ISSET(fd,&ReadFds)) ||
					(WatchedFiles[ii].Write && FD_ISSET(fd,&WriteFds)) ||
					(WatchedFiles[ii].Except && FD_ISSET(fd,&ExceptFds)))
			{
				if (q != NULL)
				{
					q->DataAvailable();
				}
				else
					queueless_count++;

			}
		}
	}
	Unlock();

	return queueless_count;
}

int FileWatcher::AddFileWatch(int fd, bool WatchRead, bool WatchWrite, bool WatchExcept)
{
	QueuePointer q;
	return AddFileWatch(fd, q,WatchRead,WatchWrite,WatchExcept);
}


int FileWatcher::AddFileWatch(int fd, QueuePointer & queue, bool WatchRead, bool WatchWrite, bool WatchExcept)
{
	Lock();
	fprintf(stderr,"Added file watch %d \n",fd);
	// find the first available file descriptor
	struct fd_driver_pair *next_entry = NULL;
	if (WatchedFilesArrayCount < WatchedFilesArraySize)
	{
		next_entry = &WatchedFiles[WatchedFilesArrayCount];
		WatchedFilesArrayCount++;
	}
	else
	{
		// first see if there is an empty spot in the list
		for (unsigned int ii = 0; ii < WatchedFilesArrayCount; ++ii)
		{
			if (WatchedFiles[ii].fd < 0)
			{
				next_entry = &WatchedFiles[ii];
				break;
			}
		}
		if (next_entry == NULL)
		{
			// otherwise we allocate some more room for the array
			WatchedFilesArraySize*=2;
			WatchedFiles = reinterpret_cast<struct fd_driver_pair *> (realloc(WatchedFiles,sizeof(WatchedFiles[0])*WatchedFilesArraySize));
			next_entry = &WatchedFiles[WatchedFilesArrayCount];

		}
	}

	next_entry->fd = fd;
	next_entry->queue = queue;
	next_entry->Read = WatchRead;
	next_entry->Write = WatchWrite;
	next_entry->Except = WatchExcept;
	Unlock();
	return 0;
}

int FileWatcher::RemoveFileWatch(int fd, bool WatchRead, bool WatchWrite, bool WatchExcept)
{
	QueuePointer q;
	return RemoveFileWatch(fd, q,WatchRead,WatchWrite,WatchExcept);
}


int FileWatcher::RemoveFileWatch(int fd, QueuePointer &queue, bool WatchRead, bool WatchWrite, bool WatchExcept)
{
	Lock();
	// this finds the first matching entry and removes it. It only removes one entry so call remove for every add
	for (unsigned int ii = 0; ii < WatchedFilesArrayCount; ++ii)
	{
		if (WatchedFiles[ii].fd == fd &&
				WatchedFiles[ii].queue == queue &&
				WatchedFiles[ii].Read == WatchRead &&
				WatchedFiles[ii].Write == WatchWrite &&
				WatchedFiles[ii].Except == WatchExcept)
		{
			WatchedFiles[ii].fd = -1;
			Unlock();
			return 0;
		}
	}
	Unlock();
	return -1;
}

